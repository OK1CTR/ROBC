/*!
 * \addtogroup Morse Morse
 * \brief Morse code transmitter working with the AX radio in ASK mode
 * @{
 */
 
/*!
 * \file    morse.c
 * \brief   Morse code transmitter working with the AX radio in ASK mode, source
 * \author  OK1CTR
 * \version 2.0
 * \date    28.08.2018
 */


#include <stdint.h>
#include <ctype.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "ax5043.h"
#include "morse.h"


#if !defined(ROBC_VER_2) && !defined(ROBC_VER_3)
	#error Define the ROBC board version!
#endif

/*! @name HW version dependent settings
 *  @{
 */
#ifdef ROBC_VER_2
  //! Morse tone generator on
	#define beep_on() GPIOB->BSRR = (uint32_t) (0x00000001L << 14)
	//! Morse tone generator off
	#define beep_off() GPIOB->BRR = (uint32_t) (0x00000001L << 14)
	//! EXTI line from RSCK
	#define MORSE_EXTI 13
	//! AFIOCR register index for INT from RSCK
	#define MORSE_AFIO_REG 3
	//! AFIOCR register offset for INT from RSCK
	#define MORSE_AFIO_OFS 4
	//! AFIOCR register word contents for INT from RSCK
	#define MORSE_AFIO_WRD 1
#endif

#ifdef ROBC_VER_3
	//! Morse tone generator on
	#define beep_on() GPIOB->BSRR = (uint32_t) (0x00000001L << 11)
	//! Morse tone generator off
	#define beep_off() GPIOB->BRR = (uint32_t) (0x00000001L << 11)
	//! EXTI line from RSCK
	#define MORSE_EXTI 10
	//! AFIOCR register index for INT from RSCK
	#define MORSE_AFIO_REG 2
	//! AFIOCR register offset for INT from RSCK
	#define MORSE_AFIO_OFS 8
	//! AFIOCR register word contents for INT from RSCK
	#define MORSE_AFIO_WRD 1
#endif
/*! @} */


//! Letters, marks from MSB, 2 LSB bits = length - 1
const uint8_t morse_c[] = {
//! A     B      C     D     E     F 
   0x41, 0x83, 0xA3, 0x82, 0x00, 0x23,
   
//! G     H      I     J     K     L
   0xC2, 0x03, 0x01, 0x73, 0xA2, 0x83,
   
//! M     N      O     P     Q     R
   0xC1, 0x81, 0xE2, 0x63, 0xD3, 0x42,
   
//! s     T      U     V     W     X 
   0x02, 0x80, 0x22, 0x13, 0x62, 0x93,
   
//! Y     Z
   0XB3, 0XC3
};

//! Numbers, marks from MSB, const. length 5 marks
const uint8_t morse_n[] = {
//! 0     1     2      3     4
   0xF8, 0x78, 0x38, 0x18, 0x08,
   
//! 5     6     7      8     9
   0x00, 0x80, 0xC0, 0xE0, 0xF0
};

//! Special characters, character table
const uint8_t morse_zn[] = {
    '.',  ',',  '?',  '-',  '/',  '+',  '@',  '_'
};

//! Special characters, marks from MSB, LSB is length: 0=5, 1=6 marks
const uint8_t morse_zc[] = {
   0x55, 0xCD, 0x31, 0x85, 0x90, 0x50, 0x15, 0x88
};


//! Morse mark or space transmission flag register
volatile uint8_t m_flag = 0;
//! Morse transmitter status register
volatile uint8_t m_status = 0;
//! Morse PA status register
volatile uint8_t m_pa = 0;
//! Transmit buffer
uint8_t morse_bf[MORSE_BFLEN];
//! Transmit buffer reading pointer
uint8_t *morse_rd = morse_bf;
//! Transmit buffer writing pointer
uint8_t *morse_wr = morse_bf;


/*! \brief External Interrupt Handler - AX5043 RCLK
 */
void EXTI15_10_IRQHandler(void)
{
	static uint8_t beep = 0, count = 0;
#ifdef _MORSE_INT_TEST_
	static uint8_t k = 0;
#endif

	if ((EXTI->PR & (1 << MORSE_EXTI)) == (1 << MORSE_EXTI)) {
#ifdef _MORSE_INT_TEST_
	if (k & 1) beep_on(); else beep_off();
	k++;		
#else		
		// morse - find mark or space length
		if (m_flag & FLAG_ENTER) {
			m_flag ^= (FLAG_ENTER | FLAG_RUN);
			if (m_flag & FLAG_SPACE) {
				beep = 0;
				count = (m_flag & FLAG_LONG) ? DASH_SP : ((m_flag & FLAG_WORD) ? WORD_SP : DOT);
			} else {
				beep = 1;
				count = (m_flag & FLAG_LONG) ? DASH : DOT;
			}
		}

  // morse - marks and space timing
	if (m_flag & FLAG_RUN) {
		if (count) { count--;	} else { m_flag = 0; beep = 0; }
	}
  
  // morse - output
  if (beep) beep_on(); else beep_off();
#endif
		EXTI->PR |= 1 << MORSE_EXTI;
	}

	return;
}


/* Morse keyer initialisation and rate setting */
void morse_init(uint8_t wpm)
{
	uint64_t a;
	
	m_flag = 0;	m_status = 0; m_pa = 0;
	morse_rd = morse_bf; morse_wr = morse_bf;
	if (wpm) {
		RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
		AFIO->EXTICR[MORSE_AFIO_REG] &= ~(0x0F << MORSE_AFIO_OFS);
		AFIO->EXTICR[MORSE_AFIO_REG] |= ((MORSE_AFIO_WRD) & 0x0F) << MORSE_AFIO_OFS;  // enable EXTI from RCLK
		EXTI->IMR |= 1 << MORSE_EXTI;
		EXTI->RTSR |= 1 << MORSE_EXTI;
		NVIC_EnableIRQ(EXTI15_10_IRQn);	

		// txrate = (WPM * 2^24) / (2.4 * fxtal)
		if (wpm < 5) wpm = 5; if (wpm > 120) wpm = 120;
		a = (((uint64_t) wpm) * 6990507L) / FRQ_XTAL;
		ax_rw_3(1, 0x167, a);
	} else {
		AFIO->EXTICR[MORSE_AFIO_REG] &= ~(0x0F << MORSE_AFIO_OFS);  // disable EXTI from RCLK
	}
  
  return;
}


/* Place new message to transmit buffer */
uint8_t morse_send(uint8_t *s)
{
	uint8_t *c = s;
	uint8_t ret = 0;

	ax_rw_2(1, 0x27, 0x01); // PWRAMP - PA on
	m_pa = 1;
	GPIOB->CRH &= CONFMASK(3); GPIOB->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 3);
  // copy the message info buffer with an overrun protection
	while (*c != '\0' && (morse_wr + 1) != morse_rd) {
		*morse_wr = *c;
		// cyclic incrementation of the write buffer pointer
		if (morse_wr < (morse_bf + MORSE_BFLEN - 1)) morse_wr++;
			else morse_wr = morse_bf;
		// message pointer incrementation
		c++;
		// character counter
		ret++;
  } 

  return(ret);
}


/* Main loop of keyer function */
void morse_loop(void)
{
	uint8_t chr;
	static uint8_t n, lng = 0, code = 0;

	// PA control
	if (m_pa == 1 && m_status == STATUS_FREE && morse_rd == morse_wr) {
		ax_rw_2(1, 0x27, 0x00); // PWRAMP - PA off
		m_pa = 0;
		GPIOB->CRH &= CONFMASK(3); GPIOB->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 3);
	}

	// if the transmitter is free and some characters are in transmit buffer
	if (m_status == STATUS_FREE && morse_rd != morse_wr) {
		// change status
		m_status = STATUS_CHARIN;
		n = 0x80;
		// take the next character from transmit buffer
		chr = toupper(*morse_rd);
		// increment the buffer pointer
		if (morse_rd < (morse_bf + MORSE_BFLEN - 1)) morse_rd++;
			else morse_rd = morse_bf;
      
		// number coding
		if (chr >= '0' && chr <= '9') {
			code = morse_n[chr - '0'];
			lng = 5;
		} else {
			// alphabet coding
			if (chr >= 'A' && chr <= 'Z') {
				code = morse_c[chr - 'A'];
				lng = 1 + (code & 0x03);
			} else {
				for (lng = 0, code = 0xFF; lng < 8; lng++) {
					if (chr == morse_zn[lng]) {
						code = 0x00; break;
					}
				}
				if (code != 0xFF) {
					code = morse_zc[lng];
					lng = 5 + (code & 0x01);
				} else {
					// space or other character - word space
					lng = 0x00;
					code = 0xFF; 
				}
			}
		}	
  }
  
	// mark and space transmission in accordance to the code
	if (m_status != STATUS_FREE && m_flag == FLAG_FREE) {

		if (m_status & STATUS_SPACE) {
			// space transmission
			if (lng >= 1) m_flag = FLAG_ENTER + FLAG_DOT_SP;
				else m_flag = FLAG_ENTER + FLAG_DASH_SP;
			m_status &= ~STATUS_SPACE;
		} else {
			// mark transmission
			if (lng > 0) {
				if (code & n) m_flag = FLAG_ENTER + FLAG_DASH;
					else m_flag = FLAG_ENTER + FLAG_DOT;
				// space
				m_status |= STATUS_SPACE;
				// increment
				lng--; n >>= 1;
			} else {
				m_status &= ~STATUS_CHARIN;
				// word space
				if (code == 0xFF) m_flag = FLAG_ENTER + FLAG_SPACE + FLAG_WORD;
			}
		}
	}

  return;
}

/*! @} */

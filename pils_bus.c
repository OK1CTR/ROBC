/*!
 * \addtogroup PilsBus Bus
 * \brief PilsenCUBE satellite communication on-board bus driver
 * @{
 */
 
/*!
 * \file    pils_bus.c
 * \brief   PilsenCUBE satellite communication on-board bus driver source
 * \author  OK1CTR
 * \version 1.0
 * \date    17.07.2018
 */


#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "pils_bus.h"


//! Convert value a < 16 to hexadecimal character
#define hexchar(a) (((a) < 10) ? '0' + (a) : 'A' + (a) - 10)
//! High nibble
#define nib_hi(a) ((a) >> 4)
//! Low nibble
#define nib_lo(a) ((a) & 0x0F)
//! Convert hex character to numeric value
#define charhex(a) ((a <= '9') ? (a) - '0' : (a) - 'A' + 10)


//! State request of the USART1 transmitter
volatile uint8_t us1_tx_str = US_TXSR_IDLE;
//! State request of the USART2 transmitter
volatile uint8_t us2_tx_str = US_TXSR_IDLE;
//! Command for device on USART1
volatile uint8_t us1_cmd;
//! Command for device on USART2
volatile uint8_t us2_cmd;
//! Number of bytes to send via USART1
volatile uint8_t us1_n;
//! Number of bytes to send via USART2
volatile uint8_t us2_n;
//! Pointer to data to send via USART1
volatile uint8_t *us1_p;
//! Pointer to data to send via USART2
volatile uint8_t *us2_p;
//! State of the USART1 receiver
volatile uint8_t us1_rx_st = US_RX_STOP;
//! State of the USART2 receiver
volatile uint8_t us2_rx_st = US_RX_STOP;
//! Received command from device on USART1
volatile uint8_t us1_rx_cmd = 0;
//! Received command from device on USART2
volatile uint8_t us2_rx_cmd = 0;
//! Number of bytes received via USART1
volatile uint8_t us1_nrx = 0;
//! Number of bytes receivec via USART2
volatile uint8_t us2_nrx = 0;
//! Max bytes which can be received via USART1
volatile uint8_t us1_mrx = 0;
//! Max bytes which can be received via USART2
volatile uint8_t us2_mrx = 0;
//! Pointer to receive buffer for USART1
volatile uint8_t *us1_prx = NULL;
//! Pointer to receive buffer for USART2
volatile uint8_t *us2_prx = NULL;
//! Backup pointer to receive buffer for USART1
volatile uint8_t *us1_prxb = NULL;
//! Backup pointer to receive buffer for USART2
volatile uint8_t *us2_prxb = NULL;

//! PilsenCUBE communication bus receive buffer, channel 1
uint8_t pilbus1_rxbf[PILS_RXBF1_SIZE];
//! PilsenCUBE communication bus receive buffer, channel 2
uint8_t pilbus2_rxbf[PILS_RXBF2_SIZE];


/* Initialize both channel USART&RS485 based PilsenCUBE bus transceivers. */
void pilbus_init(void)
{
	uint8_t i;
	
	// (Re)Init GPIO lines dedicated to SPI1 peripherial
	// PIN USART1 - TX, RX, DE
	GPIOA->CRH &= CONFMASK(1); GPIOA->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_PP, 1);
	GPIOA->CRH &= CONFMASK(2); GPIOA->CRH |= GPIOCONF(GPIO_M_INPUT, GPIO_I_FLOAT, 2);	
	GPIOA->CRH &= CONFMASK(3); GPIOA->CRH |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 3);
	// PIN USART2 - TX, RX, DE
	GPIOA->CRL &= CONFMASK(2); GPIOA->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_PP, 2);
	GPIOA->CRL &= CONFMASK(3); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_FLOAT, 3);	
	GPIOA->CRL &= CONFMASK(1); GPIOA->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 1);

	// Reset USART1 and USART2 peripherials
	RCC->APB2RSTR = RCC_APB2RSTR_USART1RST;
	RCC->APB1RSTR = RCC_APB1RSTR_USART2RST;
	for (i = 0; i < 255; i++) __nop();
	RCC->APB2RSTR = 0; RCC->APB1RSTR = 0;
	
	// Init USART1
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	USART1->BRR = (SystemCoreClock / PILS_BUS1_RATE);
	NVIC_EnableIRQ(USART1_IRQn);
	usart1_de_off();
	// Init USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
	USART2->BRR = (SystemCoreClock / PILS_BUS2_RATE);
	NVIC_EnableIRQ(USART2_IRQn);
	usart2_de_off();
	
	// Receive buffer init on channel 1
	us1_mrx = PILS_RXBF1_SIZE;
	us1_prx = pilbus1_rxbf;
	us1_prxb = pilbus1_rxbf;
	us1_nrx = 0;
	us1_rx_st = US_RX_IDLE;
	// Receive buffer init on channel 2
	us2_mrx = PILS_RXBF2_SIZE;
	us2_prx = pilbus2_rxbf;
	us2_prxb = pilbus2_rxbf;
	us2_nrx = 0;
	us2_rx_st = US_RX_IDLE;

	return;
}


/* Send message on given  USART channel */
void pilbus_send(uint8_t ch, uint8_t adr, uint8_t cmd, uint8_t n, uint8_t *msg) 
{
	// Limit the channel number
	if (ch == 0 || ch > 2) return;
	// Send the OBU address
	if (ch == 1) {
		usart1_de_on();
		us1_cmd = cmd;
		us1_n = n;
		us1_p = msg;
		us1_tx_str = US_TXSR_COMMAND;
		USART1->DR = adr;
		USART1->CR1 |= USART_CR1_TXEIE | USART_CR1_TCIE;
	} else {
		usart2_de_on();
		us2_cmd = cmd;
		us2_n = n;
		us2_p = msg;
		us2_tx_str = US_TXSR_COMMAND;
		USART2->DR = adr;
		USART2->CR1 |= USART_CR1_TXEIE | USART_CR1_TCIE;
	}
	return;
}


/* Receiver quick buffer and state machine reinitialization */
void pilbus_rxreinit(uint8_t ch)
{
	// Limit the channel number
	if (ch == 0 || ch > 2) return;
	// Send the OBU address
	if (ch == 1) {
		us1_prx = us1_prxb;
		us1_nrx = 0;
		us1_rx_st = US_RX_IDLE;
	} else {
		us2_prx = us2_prxb;
		us2_nrx = 0;
		us2_rx_st = US_RX_IDLE;
	}
	return;
}


/*! \brief USART1 Event interrupt handler */
void USART1_IRQHandler(void)
{
	volatile uint32_t sr = USART1->SR;
	static uint8_t a, sum, rx, rsum, tsum;
	uint8_t c, d;

	// Receiving register not empty
	if (sr & USART_SR_RXNE) {
		switch (us1_rx_st) {
			case US_RX_IDLE:
				if (USART1->DR == PILS_OBC_ADR1) {
					us1_rx_st = US_RX_ADDRESSED;
				}
				break;
			case US_RX_ADDRESSED:
				us1_rx_st = US_RX_COMMAND;
				us1_rx_cmd = USART1->DR;
				rsum = us1_rx_cmd;
				break;
			case US_RX_COMMAND:
				if ((d = USART1->DR) != PILS_MSG_TERM) {
					// received character range check
					if (!isxdigit(d)) { us1_rx_st = US_RX_ERROR; break; }
					// check free space in buffer
					if (us1_nrx + 1 >= us1_mrx) { us1_rx_st = US_RX_ERROR; break; }
					// save rx and sum if >0 received characters
					if (us1_nrx) { *us1_prx = rx; us1_prx++; rsum += tsum; }
					// make new rx and sum
					rx = charhex(d) << 4;	us1_rx_st = US_RX_DATAH; tsum = d;
				} else {
					// received PILS_MSG_TERM
					rsum += rx;
					if (rsum) us1_rx_st = US_RX_ERROR; else us1_rx_st = US_RX_OK;
					us1_nrx--;
				}
				break;
			case US_RX_DATAH:
				if ((d = USART1->DR) != PILS_MSG_TERM) {
					// received character range check
					if (!isxdigit(d)) { us1_rx_st = US_RX_ERROR; break; }
					rx |= charhex(d);	us1_rx_st = US_RX_COMMAND;
					tsum += d; us1_nrx++;
				} else {
					// must not receive PILS_MSG_TERM here
					us1_rx_st = US_RX_ERROR; break;
				}
				break;
			case US_RX_ERROR:  // in case of error
			case US_RX_OK:   // or in case of ok
			case US_RX_STOP:   // or receiver stopped
				d = USART1->DR;  // do nothing
				break;
		}
	}

	// TX buffer empty
	if (sr & USART_SR_TXE) {
		// Zero payload length protection
		if (us1_tx_str == US_TXSR_DATAH && us1_n == 0) us1_tx_str = US_TXSR_SUMH;

		switch (us1_tx_str) {
			case US_TXSR_COMMAND:
				USART1->DR = us1_cmd;
				us1_tx_str = US_TXSR_DATAH;
				sum = us1_cmd;
				break;
			case US_TXSR_DATAH:
				a = *us1_p; us1_p++; us1_n--;
				c = hexchar(nib_hi(a));	USART1->DR = c;	sum += c;
				us1_tx_str = US_TXSR_DATAL;
				break;
			case US_TXSR_DATAL:
				c = hexchar(nib_lo(a)); USART1->DR = c; sum += c;
				if (us1_n) us1_tx_str = US_TXSR_DATAH; else us1_tx_str = US_TXSR_SUMH;
				break;
			case US_TXSR_SUMH:
				sum = ~sum; sum++;
				USART1->DR = hexchar(nib_hi(sum));
				us1_tx_str = US_TXSR_SUML;
				break;
			case US_TXSR_SUML:
				USART1->DR = hexchar(nib_lo(sum));
				us1_tx_str = US_TXSR_TERM;
				break;
			case US_TXSR_TERM:
				USART1->DR = PILS_MSG_TERM;
				us1_tx_str = US_TXSR_CLOSE;
				USART1->CR1 &= ~USART_CR1_TXEIE;
				break;
		}
	}

	/* Transmission complette */
	if (sr & USART_SR_TC && us1_tx_str == US_TXSR_CLOSE) {
		USART1->CR1 &= ~USART_CR1_TCIE;
		USART1->SR &= ~USART_SR_TC;
		usart1_de_off();
		us1_tx_str = US_TXSR_IDLE;
		return;
	}

	return;
}


/*! \brief USART2 Event interrupt handler */
void USART2_IRQHandler(void)
{
	volatile uint32_t sr = USART2->SR;
	static uint8_t a, sum, rx, rsum, tsum;
	uint8_t c, d;


	// Receiving register not empty
	if (sr & USART_SR_RXNE) {
		switch (us2_rx_st) {
			case US_RX_IDLE:
				if (USART2->DR == PILS_OBC_ADR2) {
					us2_rx_st = US_RX_ADDRESSED;
				}
				break;
			case US_RX_ADDRESSED:
				us2_rx_st = US_RX_COMMAND;
				us2_rx_cmd = USART2->DR;
				rsum = us2_rx_cmd;
				break;
			case US_RX_COMMAND:
				if ((d = USART2->DR) != PILS_MSG_TERM) {
					// received character range check
					if (!isxdigit(d)) { us2_rx_st = US_RX_ERROR; break; }
					// check free space in buffer
					if (us2_nrx + 1 >= us2_mrx) { us2_rx_st = US_RX_ERROR; break; }
					// save rx and sum if >0 received characters
					if (us2_nrx) { *us2_prx = rx; us2_prx++; rsum += tsum; }
					// make new rx and sum
					rx = charhex(d) << 4;	us2_rx_st = US_RX_DATAH; tsum = d;
				} else {
					// received PILS_MSG_TERM
					rsum += rx;
					if (rsum) us2_rx_st = US_RX_ERROR; else us2_rx_st = US_RX_OK;
					us2_nrx--;
				}
				break;
			case US_RX_DATAH:
				if ((d = USART2->DR) != PILS_MSG_TERM) {
					// received character range check
					if (!isxdigit(d)) { us2_rx_st = US_RX_ERROR; break; }
					rx |= charhex(d);	us2_rx_st = US_RX_COMMAND;
					tsum += d; us2_nrx++;
				} else {
					// must not receive PILS_MSG_TERM here
					us2_rx_st = US_RX_ERROR; break;
				}
				break;
			case US_RX_ERROR:  // in case of error
			case US_RX_OK:   // or in case of ok
			case US_RX_STOP:   // or receiver stopped
				d = USART2->DR;  // do nothing
				break;
		}
	}
	
	// TX buffer empty
	if (sr & USART_SR_TXE) {
		// Zero payload length protection
		if (us2_tx_str == US_TXSR_DATAH && us2_n == 0) us2_tx_str = US_TXSR_SUMH;
		switch (us2_tx_str) {
			case US_TXSR_COMMAND:
				USART2->DR = us2_cmd;
				us2_tx_str = US_TXSR_DATAH;
				sum = us2_cmd;
				break;
			case US_TXSR_DATAH:
				a = *us2_p; us2_p++; us2_n--;
				c = hexchar(nib_hi(a));	USART2->DR = c;	sum += c;
				us2_tx_str = US_TXSR_DATAL;
				break;
			case US_TXSR_DATAL:
				c = hexchar(nib_lo(a)); USART2->DR = c; sum += c;
				if (us2_n) us2_tx_str = US_TXSR_DATAH; else us2_tx_str = US_TXSR_SUMH;
				break;
			case US_TXSR_SUMH:
				sum = ~sum; sum++;
				USART2->DR = hexchar(nib_hi(sum));
				us2_tx_str = US_TXSR_SUML;
				break;
			case US_TXSR_SUML:
				USART2->DR = hexchar(nib_lo(sum));
				us2_tx_str = US_TXSR_TERM;
				break;
			case US_TXSR_TERM:
				USART2->DR = PILS_MSG_TERM;
				us2_tx_str = US_TXSR_CLOSE;
				USART2->CR1 &= ~USART_CR1_TXEIE;
				break;
		}
	}

	// Transmission complette
	if (sr & USART_SR_TC && us2_tx_str == US_TXSR_CLOSE) {
		USART2->CR1 &= ~USART_CR1_TCIE;
		USART2->SR &= ~USART_SR_TC;
		usart2_de_off();
		us2_tx_str = US_TXSR_IDLE;
		return;
	}

	return;
}

/*! @} */

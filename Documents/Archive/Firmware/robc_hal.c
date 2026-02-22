/*!
 * \addtogroup Hal HAL
 * \brief PilsenCUBE COM-OBC Hardware Abstraction Level module
 * @{
 */

/*!
 * \file    robc_hal.c
 * \brief   PilsenCUBE COM-OBC Hardware Abstraction Level module source
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "i2c.h"
#include "robc_hal.h"


#if !defined(ROBC_VER_2) && !defined(ROBC_VER_3)
	#error Define the ROBC board version!
#endif

// PA protection control lines
#define paen_H() GPIOA->BSRR = (uint32_t) (0x00000001L << 8)
#define paen_L() GPIOA->BRR  = (uint32_t) (0x00000001L << 8)
#define plow_H() GPIOA->BSRR = (uint32_t) (0x00000001L << 12)
#define plow_L() GPIOA->BRR  = (uint32_t) (0x00000001L << 12)

// Shift ADC_SMPR2
#define SMP0				 0
#define SMP4				12
#define SMP5				15
#define SMP6				18
#define SMP7				21
#define SMP8				24
#define SMP9				27

// Sample time for ADC_SMPR2
#define SMT0				 6
#define SMT4				 6
#define SMT5				 6
#define SMT6				 6
#define SMT7				 6
#define SMT8				 6
#define SMT9				 6 


//! One second counter
volatile uint32_t seconds = 0;
//! One second update flag
volatile uint8_t sec_upd = 0;
//! Timeout software timer 1
volatile uint8_t utmr1 = 0;
//! Timeout software timer 2
volatile uint8_t utmr2 = 0;
//! Timeout software timer 3
volatile uint8_t utmr3 = 0;

//! PA current protection setting
uint8_t pa_low = 0;


/*! \brief Timer 3 IRQ handler
 * \detail One second timing
 *
void TIM3_IRQHandler(void)
{
	if (TIM3->SR & TIM_SR_UIF) {
		// clear the UIF flag
		TIM3->SR &= ~TIM_SR_UIF;
		// ...
	}
	return;
}
*/


/*! \brief Systick IRQ handler
 * \detail Used for software timeout timers and for system time
 */
void SysTick_Handler(void)
{
	static volatile uint16_t ms2s = 0;
	
	// one second timer
	if (++ms2s >= 1000) {
		ms2s = 0;
		seconds++;
		sec_upd = 1;
	}
	// timeout processing
	if (utmr1 != 0) utmr1--;
	if (utmr2 != 0) utmr2--;
	if (utmr3 != 0) utmr3--;
	return;
}


// Startup STM32 IO ports configuration
void ioports_init(void)
{
	// System clock configuration
	RCC->CR |= RCC_CR_HSEON;
	RCC->CFGR |= RCC_CFGR_SW_HSE;
	// JTAG interface disable and SPI remapping
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG; AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
	// GPIO clocks
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
	// PIN HW Watchdog - reset = PB2
	GPIOB->CRL &= CONFMASK(2); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 2);
	
#ifdef ROBC_VER_2
	/*
	// Debug output lines in place of FRAM memory on SPI1 = PB3, PB4, PB5
	// Collision with JTAG ! - pins won't set as GPIO. Only dbg_3 working.
	GPIOB->CRL &= CONFMASK(3); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 3);
	GPIOB->CRL &= CONFMASK(4); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 4);
	GPIOB->CRL &= CONFMASK(5); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_OUT_PP, 5);
	*/
#endif

#ifdef ROBC_VER_3
	// PIN PilsenCUBE connector - SERV, NEAR = PA0, PA4
	GPIOA->CRL &= CONFMASK(0); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_PULL, 0);  // Pullup on
	GPIOA->CRL &= CONFMASK(4); GPIOA->CRL |= GPIOCONF(GPIO_M_INPUT, GPIO_I_FLOAT, 4);
#endif

	return;
}


// Default timer configuration
void timer_init(void)
{
	// TIMER2 - short pauses for HW, 1 us, non-retriggerable
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 23; TIM2->CR1 |= TIM_CR1_OPM | TIM_CR1_DIR;

	// Systick, 1 ms timer for software timeout timers and for system time
	SysTick_Config(24000);

	/* TIMER3 - system timer, 1 ms, 16bit, retriggerable, still running, 1 s int
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->PSC = 23999; TIM3->ARR = 1000;
	TIM3->DIER = TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM3_IRQn);
	TIM3->CR1 |= TIM_CR1_CEN;
	*/

	// Timer input frequency fioclk = 24 MHz.
	return;
}


// Sleep specified time in us, based on TIMER2
void sleep_us(uint16_t time)
{
	TIM2->CNT = time - 1; TIM2->CR1 |= TIM_CR1_CEN;
	while (TIM2->CR1 & TIM_CR1_CEN)
		;
	TIM2->CR1 &= ~TIM_CR1_CEN; TIM2->CNT = 0;
	return;
}


// Watchdog trigger
void wdt_trig(void)
{
	static uint8_t level = 0;

	if (level & 0x01) {
		GPIOB->BRR |= 4;
	} else {
		GPIOB->BSRR |= 4;
	}
	level++;
	return;
}


// PA current protection control
void pa_prot_ctrl(uint8_t pa_en)
{
	if (pa_low) plow_L(); else plow_H();
#if defined (PA_DISABLE) && PA_DISABLE == 1
	paen_L();
#else
	if (pa_en) paen_H(); else paen_L();
#endif
	return;
}


// Display symbol on LED bar
void led_disp(uint8_t symbol)
{
	i2c1_write_buf(0x20, ~symbol, 0);
	i2c1_wait(); sleep_us(1000);
	return;
}


/*! @} */

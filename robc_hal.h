/*!
 * \addtogroup Hal HAL
 * \brief PilsenCUBE COM-OBC Hardware Abstraction Level module
 * @{
 */

/*!
 * \file    robc_hal.h
 * \brief   PilsenCUBE COM-OBC Hardware Abstraction Level module, header
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#ifndef _ROBC_HAL_H_
#define _ROBC_HAL_H_


// Debug lines in place of memory SPI, PB3, PB4, PB5
#define dbg_1H() GPIOB->BSRR = (uint32_t) (0x00000001L << 3)
#define dbg_2H() GPIOB->BSRR = (uint32_t) (0x00000001L << 4)
#define dbg_3H() GPIOB->BSRR = (uint32_t) (0x00000001L << 5)
#define dbg_1L() GPIOB->BRR  = (uint32_t) (0x00000001L << 3)
#define dbg_2L() GPIOB->BRR  = (uint32_t) (0x00000001L << 4)
#define dbg_3L() GPIOB->BRR  = (uint32_t) (0x00000001L << 5)

// Poll the radio IRQ pin
#define rirq_active() (GPIOB->PIN & (1 << 9))


// One second counter
extern volatile uint32_t seconds;
// One second update flag
extern volatile uint8_t sec_upd;
//! Timeout software timer 1
extern volatile uint8_t utmr1;
//! Timeout software timer 2
extern volatile uint8_t utmr2;
//! Timeout software timer 3
extern volatile uint8_t utmr3;

//! PA current protection setting
extern uint8_t pa_low;


// Startup STM32 IO ports configuration
extern void ioports_init(void);
// Default timer configuration
extern void timer_init(void);
// Sleep specified time in us (TIM-4)
extern void sleep_us(uint16_t time);
// Watchdog trigger
extern void wdt_trig(void);
// PA current protection control
extern void pa_prot_ctrl(uint8_t pa_en);
// Display symbol on LED bar
extern void led_disp(uint8_t symbol);


#endif

/*! @} */

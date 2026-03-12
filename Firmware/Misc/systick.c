/**
 * @file       systick.c
 * @author     OK1CTR
 * @date       Mar 2026
 * @brief      MCU core SzsTick based timing
 *
 * @addtogroup grSystick
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <systick.h>
#include <stm32f1xx_ll_rcc.h>
#include <stm32f1xx_ll_utils.h>
#include <stm32f1xx_ll_cortex.h>
#include <main.h>

/* Private variables ---------------------------------------------------------*/

/* SysTick tick counter */
static volatile ticks_t systick_counter;

/* Functions -----------------------------------------------------------------*/

/* SysTick initialization */
void tick_init()
{
    LL_Init1msTick(SystemCoreClock);
    LL_SYSTICK_EnableIT();
    //LL_SYSTICK_EnableCounter();
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    systick_counter = 0;
}


/* Return actual state of the SysTick tick counter */
ticks_t ticks_now()
{
    return systick_counter;
}

/* ISR -----------------------------------------------------------------------*/

/* SysTick IRQ handler */
void SysTick_Handler()
{
    systick_counter++;
}

/* ---------------------------------------------------------------------------*/

/** @} */

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
#include <main.h>
#include <stm32f1xx_ll_rcc.h>
#include <stm32f1xx_ll_utils.h>
#include <stm32f1xx_ll_cortex.h>

/* Private variables ---------------------------------------------------------*/

/* SysTick tick counter */
static volatile ticks_t systick_counter;

/* Functions -----------------------------------------------------------------*/

/* SysTick initialization */
void tick_init()
{
    critical_enter();
    systick_counter = 0;
    critical_exit();
    LL_Init1msTick(SystemCoreClock);
    LL_SYSTICK_EnableIT();
    //LL_SYSTICK_EnableCounter();
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
}


/* Return actual state of the SysTick tick counter */
ticks_t ticks_now()
{
    return systick_counter;
}


/* Set 64bit unsigned tick timer to given number of ticks */
void tick_timer_set(ticks_t *tmr, ticks_t ticks)
{
    critical_enter();
    *tmr = systick_counter + ticks;
    critical_exit();
}


/* Check 64bit unsigned tick timer to expire */
bool tick_timer_expired(ticks_t *tmr)
{
    bool ret;
    critical_enter();
    ret = systick_counter - *tmr < 0x7FFFFFFFFFFFFFFF;
    critical_exit();
    return ret;
}

/* ISR -----------------------------------------------------------------------*/

/* SysTick IRQ handler */
void SysTick_Handler()
{
    critical_enter();
    systick_counter++;
    critical_exit();
}

/* ---------------------------------------------------------------------------*/

/** @} */

/**
 * @file       critical.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Disable interrupts recursively for critical code sections
 *
 * @addtogroup grCritical
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <stm32F1xx.h>

/* Private variables ---------------------------------------------------------*/

static uint32_t critical_section_counter = 0;

/* Functions -----------------------------------------------------------------*/

/* Begin of critical section */
void critical_enter(void)
{
    __disable_irq();
    critical_section_counter++;
}


/* End of critical section */
void critical_exit(void)
{
    critical_section_counter--;
    if (critical_section_counter <= 0)
    {
        __enable_irq();
    }
}

/* ---------------------------------------------------------------------------*/

/** @} */

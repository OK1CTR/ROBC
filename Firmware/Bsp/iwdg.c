/**
 * @file       iwdg.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Independent watchdog module
 *
 * @addtogroup grIwdg
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <iwdg.h>
#include <main.h>

/* Functions -----------------------------------------------------------------*/

/* IWDG initialization */
void iwdg_init()
{
#if IWDG_ENABLED == 1
    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_4);
    LL_IWDG_SetReloadCounter(IWDG, 4095);
    while (LL_IWDG_IsReady(IWDG) != 1)
    {
    }

    LL_IWDG_ReloadCounter(IWDG);
#endif
}


/* Reset the IWDG timer */
void iwdg_hit()
{
#if IWDG_ENABLED == 1
    LL_IWDG_ReloadCounter(IWDG);
#endif
}

/* ---------------------------------------------------------------------------*/

/** @} */

/**
 * @file       watchdog.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Watchdog module
 *
 * @addtogroup grWatchdog
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <main.h>
#include <watchdog.h>

/* Functions -----------------------------------------------------------------*/

/* Watchdog initialization */
void watchdog_init()
{
    // independent watchdog
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

    // hardware and communication watchdog
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_GPIO_ResetOutputPin(WDI_GPIO_Port, WDI_Pin);
    GPIO_InitStruct.Pin = WDI_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(WDI_GPIO_Port, &GPIO_InitStruct);

    // toggle pin to first open the hardware watchdog
    LL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);
    LL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);
}


/* Reset the watchdog */
void watchdog_hit()
{
    // independent watchdog hit
#if IWDG_ENABLED == 1
    LL_IWDG_ReloadCounter(IWDG);
#endif

    // hardware and communication watchdog
    LL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);
    LL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);
}

/* ---------------------------------------------------------------------------*/

/** @} */

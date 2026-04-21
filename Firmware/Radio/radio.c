/**
 * @file       radio.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Radio control module
 *
 * @addtogroup grRadio
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <radio.h>
#include <main.h>

/* Functions -----------------------------------------------------------------*/

/* Initialize radio module */
void radio_init()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // PAEN
    LL_GPIO_ResetOutputPin(PAEN_GPIO_Port, PAEN_Pin);  // PAEN = 0 (PA OFF)
    GPIO_InitStruct.Pin = PAEN_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(PAEN_GPIO_Port, &GPIO_InitStruct);

    // LOW
    LL_GPIO_SetOutputPin(LOW_GPIO_Port, LOW_Pin);  // LOW = 1 (Power high)
    GPIO_InitStruct.Pin = LOW_Pin;
    LL_GPIO_Init(LOW_GPIO_Port, &GPIO_InitStruct);
}

/* ---------------------------------------------------------------------------*/

/** @} */

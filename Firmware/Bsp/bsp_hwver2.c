/**
 * @file       bsp_hwver2.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      RadioOBC Board Specific Definitions - HW version 2 (first prototype)
 *
 * @addtogroup grBsp
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <bsp_hwver2.h>
#include <main.h>

/* Functions -----------------------------------------------------------------*/

/* Generic GPIO initialization */
void gpio_init()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // RDDA pin (default analog)
    GPIO_InitStruct.Pin = RDDA_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(RDDA_GPIO_Port, &GPIO_InitStruct);

    // RDCL pin (default analog)
    GPIO_InitStruct.Pin = RDCL_Pin;
    LL_GPIO_Init(RDCL_GPIO_Port, &GPIO_InitStruct);
}

/* ---------------------------------------------------------------------------*/

/** @} */

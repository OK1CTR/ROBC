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
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_GPIO_ResetOutputPin(GPIOA, PAEN_Pin | LOW_Pin);
    LL_GPIO_ResetOutputPin(GPIOB, RSCK_Pin | RSEL_Pin);

    GPIO_InitStruct.Pin = PAEN_Pin | LOW_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = RSCK_Pin | RSEL_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = RMISO_Pin | RDA_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTB, LL_GPIO_AF_EXTI_LINE11);
    LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTB, LL_GPIO_AF_EXTI_LINE13);
    LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTB, LL_GPIO_AF_EXTI_LINE9);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_11;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_13;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_9;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    LL_GPIO_SetPinMode(RMOSI_GPIO_Port, RMOSI_Pin, LL_GPIO_MODE_FLOATING);
    LL_GPIO_SetPinMode(RDCL_GPIO_Port, RDCL_Pin, LL_GPIO_MODE_FLOATING);
    LL_GPIO_SetPinMode(RIRQ_GPIO_Port, RIRQ_Pin, LL_GPIO_MODE_FLOATING);
}

/* ---------------------------------------------------------------------------*/

/** @} */

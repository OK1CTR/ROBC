/**
 * @file       i2c.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Auxiliary I2C bus module
 *
 * @addtogroup grI2C
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <i2c.h>
#include <main.h>

/* Functions -----------------------------------------------------------------*/

/* I2C1 initialization */
void i2c_init()
{
    LL_I2C_InitTypeDef I2C_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = SCL_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    LL_GPIO_Init(SCL_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = SDA_Pin;
    LL_GPIO_Init(SDA_GPIO_Port, &GPIO_InitStruct);

    I2C1_CLOCK_EN();

    NVIC_SetPriority(I2C1_IRQ_N, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C1_IRQ_N);
    NVIC_SetPriority(I2C1_ERROR_IRQ_N, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C1_ERROR_IRQ_N);

    LL_I2C_DisableOwnAddress2(I2C1_I2C);
    LL_I2C_DisableGeneralCall(I2C1_I2C);
    LL_I2C_EnableClockStretching(I2C1_I2C);
    I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
    I2C_InitStruct.ClockSpeed = I2C1_CLOCK_SPEED;
    I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
    I2C_InitStruct.OwnAddress1 = 0;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
    I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(I2C1_I2C, &I2C_InitStruct);
    LL_I2C_SetOwnAddress2(I2C1_I2C, 0);
}


/* Send byte to given I2C device */
void i2c_write(uint8_t dev_addr, uint8_t data)
{
    // generate Start
    LL_I2C_GenerateStartCondition(I2C1_I2C);
    while(!LL_I2C_IsActiveFlag_SB(I2C1_I2C));

    // send slave address (write mode)
    LL_I2C_TransmitData8(I2C1_I2C, dev_addr);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1_I2C));
    LL_I2C_ClearFlag_ADDR(I2C1_I2C);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1_I2C));

    // send data
    LL_I2C_TransmitData8(I2C1_I2C, data);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1_I2C));

    // generate stop
    LL_I2C_GenerateStopCondition(I2C1_I2C);
}

/* ---------------------------------------------------------------------------*/

/** @} */


// Example: Write one byte to a register
/*
void I2C_Write(I2C_TypeDef *I2Cx, uint8_t devAddr, uint8_t regAddr, uint8_t data) {
}
*/

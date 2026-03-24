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

//! I2C-1 data buffer
uint8_t *i2c1_buffer;
//! I2C-1 error flags including timeout
uint8_t i2c1_error = 0;


//! Adress of the slave device to communicate with
static uint8_t i2c1_address;
//! Register in the slave device to access
static uint8_t i2c1_register;
//! Number of bytes to read or write
static uint8_t i2c1_bytenum = 0;
//! I2C data buffer index
static uint8_t i2c1_index = 0;
//! I2C repeated start request
static uint8_t i2c1_bytenum2 = 0;

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


/*!\brief Write buffer content to the I2C slave
 * \note The function is non blocking, use i2c1_wait() to block the program execution.
 * \note 2018-04-30 LIS2DS12TR: Sleep 3 us or more before every I2C operation necessary!
 * \param addr I2C slave address
 * \param reg I2C slave internal register address
 * \param n Number of bytes to be written
 */
void i2c1_write_buf(uint8_t addr, uint8_t reg, uint8_t n)
{
    i2c1_address = addr << 1;
    i2c1_register = reg;
    i2c1_bytenum = n;
    i2c1_index = 0;
    i2c1_bytenum2 = 0;

    I2C1->CR2 |= I2C_CR2_ITBUFEN;
    I2C1->CR2 |= I2C_CR2_ITEVTEN;
    I2C1->CR2 |= I2C_CR2_ITERREN;
    I2C1->CR1 |= I2C_CR1_START;
    return;
}


/*!\brief Read content from the I2C slave
 * \note The function is non blocking, use i2c1_wait() to block the program execution.
 * \note 2018-04-30 LIS2DS12TR: Sleep 3 us or more before every I2C operation necessary!
 * \param addr I2C slave address
 * \param reg I2C slave internal register address
 * \param n Number of bytes to be read
 */
void i2c1_read_buf(uint8_t addr, uint8_t reg, uint8_t n)
{
    i2c1_address = addr << 1;
    i2c1_register = reg;
    i2c1_bytenum = 0;
    i2c1_index = 0;
    i2c1_bytenum2 = n;
    I2C1->CR2 |= I2C_CR2_ITBUFEN;
    I2C1->CR2 |= I2C_CR2_ITEVTEN;
    I2C1->CR2 |= I2C_CR2_ITERREN;
    I2C1->CR1 |= I2C_CR1_ACK;
    I2C1->CR1 |= I2C_CR1_START;
    return;
}


/*!\brief I2C1 Event interrupt handler */
void I2C1_EV_IRQHandler(void)
{
    uint32_t SR1Register;
    uint32_t SR2Register;
    uint32_t temp;

    SR1Register = I2C1->SR1; SR2Register = I2C1->SR2;

    if (SR1Register & I2C_SR1_SB) {
        I2C1->DR = i2c1_address;
        SR1Register = 0; SR2Register = 0;
  }

  if (SR2Register & I2C_SR2_MSL) { // If I2C1 is Master (MSL flag = 1)
        if (SR1Register & I2C_SR1_ADDR) {  // Address was sent
            if (SR2Register & I2C_SR2_TRA) { // Master Transmiter
                I2C1->DR = i2c1_register;
                if (i2c1_bytenum == 0) {
                    I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
                }
            } else { // Master Receiver
                i2c1_index = 0;
                if (i2c1_bytenum == 1) {
                    I2C1->CR1 &= ~I2C_CR1_ACK;
                    I2C1->CR1 |= I2C_CR1_STOP;
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }

        if ((SR1Register & (I2C_SR1_TXE | I2C_SR1_BTF)) == I2C_SR1_TXE) {
            temp = i2c1_bytenum;
            if (temp) {
                I2C1->DR = i2c1_buffer[i2c1_index++];
                temp--;
                i2c1_bytenum = temp;
                if (temp == 0) {
                    I2C1->CR2 &= ~I2C_CR2_ITBUFEN;
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }
        if ((SR1Register & (I2C_SR1_TXE | I2C_SR1_BTF)) == (I2C_SR1_TXE | I2C_SR1_BTF)) {
            if (i2c1_bytenum2) {
                i2c1_bytenum = i2c1_bytenum2;
                i2c1_bytenum2 = 0;
                i2c1_address++;
                I2C1->CR2 |= I2C_CR2_ITBUFEN;
                I2C1->CR1 |= I2C_CR1_START;
                I2C1->DR = 0; // !!!
            } else {
                I2C1->CR1 |= I2C_CR1_STOP;
                I2C1->CR2 &= ~I2C_CR2_ITEVTEN;
                I2C1->CR2 &= ~I2C_CR2_ITERREN;
            }
            SR1Register = 0;
            SR2Register = 0;
        }
        if ((SR1Register & I2C_SR1_RXNE) == I2C_SR1_RXNE) {
            temp = i2c1_index;
            i2c1_buffer[temp] = I2C1->DR;
            temp++;
            i2c1_index = temp;
            temp = i2c1_bytenum;
            temp--;
            i2c1_bytenum = temp;
            if (temp == 1) {
                I2C1->CR1 &= ~I2C_CR1_ACK;
                I2C1->CR1 |= I2C_CR1_STOP;
            }
            SR1Register = 0;
            SR2Register = 0;
        }
    } // If I2C1 is Master
    return;
}


/*!\brief I2C1 Error interrupt handler */
void I2C1_ER_IRQHandler(void)
{
    uint32_t SR1Register;

    SR1Register = I2C1->SR1;

    if (SR1Register & I2C_SR1_AF) {
        I2C1->SR1 &= ~I2C_SR1_AF;
        i2c1_error |= I2C_ERR_NOACK;
    }

    return;
}

/* ---------------------------------------------------------------------------*/

/** @} */

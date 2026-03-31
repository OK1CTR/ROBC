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
#include <string.h>

/* Private typedefs ----------------------------------------------------------*/

/* Interface state */
typedef enum
{
    state_reset = 0,
    state_init
} state_e;

/*! I2C private variables */
typedef struct
{
    uint8_t address;    ///< Address of the slave device to communicate with
    uint8_t reg_adr;    ///< Register in the slave device to access
    uint8_t bytenum;    ///< Number of bytes to read or write
    uint8_t index;      ///< Data buffer index
    uint8_t bytenum2;   ///< Repeated start request
    i2c_error_t error;  ///< Error flags including timeout
    uint8_t *buffer;    ///< Transmit and receive data buffer pointer
} i2c_context_t;

/* Private variables ---------------------------------------------------------*/

/*! I2C private variables */
static i2c_context_t i2c1con;

/*! Interface state */
static state_e state;

/* Functions -----------------------------------------------------------------*/

/* I2C1 initialization */
void i2c_init()
{
    LL_I2C_InitTypeDef I2C_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // ports
    GPIO_InitStruct.Pin = SCL_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    LL_GPIO_Init(SCL_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = SDA_Pin;
    LL_GPIO_Init(SDA_GPIO_Port, &GPIO_InitStruct);

    // I2C1
    I2C1_CLOCK_EN();

    // interrupts
    NVIC_SetPriority(I2C1_IRQ_N, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C1_IRQ_N);
    NVIC_SetPriority(I2C1_ERROR_IRQ_N, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C1_ERROR_IRQ_N);

    // I2C
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

    // private variables
    i2c1con.address = 0;
    i2c1con.reg_adr = 0;
    i2c1con.bytenum = 0;
    i2c1con.index = 0;
    i2c1con.bytenum2 = 0;
    i2c1con.error.noack1 = 0;
    i2c1con.error.timeout1 = 0;
    i2c1con.buffer = NULL;

    state = state_init;
}


/* Assign transmit and receive data buffer pointer for I2C operations */
void set_buffer(uint8_t *buf_ptr)
{
    i2c1con.buffer = buf_ptr;
}


/* Write buffer content to the I2C slave */
void i2c1_write_buf(uint8_t addr, uint8_t reg, uint8_t n)
{
    if (state != state_init)
    {
        return;
    }

    i2c1con.address = addr << 1;
    i2c1con.reg_adr = reg;
    i2c1con.bytenum = n;
    i2c1con.index = 0;
    i2c1con.bytenum2 = 0;
    LL_I2C_EnableIT_TX(I2C1_I2C);
    LL_I2C_EnableIT_ERR(I2C1_I2C);
    LL_I2C_GenerateStartCondition(I2C1_I2C);
}


/* Read content from the I2C slave into the buffer */
void i2c1_read_buf(uint8_t addr, uint8_t reg, uint8_t n)
{
    if (state != state_init)
    {
        return;
    }

    i2c1con.address = addr << 1;
    i2c1con.reg_adr = reg;
    i2c1con.bytenum = 0;
    i2c1con.index = 0;
    i2c1con.bytenum2 = n;
    LL_I2C_EnableIT_TX(I2C1_I2C);
    LL_I2C_EnableIT_ERR(I2C1_I2C);
    LL_I2C_AcknowledgeNextData(I2C1_I2C, LL_I2C_ACK);
    LL_I2C_GenerateStartCondition(I2C1_I2C);
}


/* I2C bus communication in progress test */
bool is_i2c1_busy()
{
    return LL_I2C_IsActiveFlag_BUSY(I2C1_I2C);
}


/* Return actual I2C error state */
i2c_error_t get_i2c_error()
{
    return i2c1con.error;
}

/* ISR -----------------------------------------------------------------------*/

/* I2C1 Event interrupt handler */
void I2C1_IRQ_HANDLER()
{
    uint32_t SR1Register = I2C1_I2C->SR1;
    uint32_t SR2Register = I2C1_I2C->SR2;
    uint32_t temp;

    if (SR1Register & I2C_SR1_SB)
    {
        LL_I2C_TransmitData8(I2C1_I2C, i2c1con.address);
        SR1Register = 0;
        SR2Register = 0;
    }

    if (SR2Register & I2C_SR2_MSL)  // If I2C1 is Master (MSL flag = 1)
    {
        if (SR1Register & I2C_SR1_ADDR)  // address was sent
        {
            if (SR2Register & I2C_SR2_TRA)
            {
                // master transmitter
                LL_I2C_TransmitData8(I2C1_I2C, i2c1con.reg_adr);
                if (i2c1con.bytenum == 0)
                {
                    LL_I2C_DisableIT_BUF(I2C1_I2C);
                }
            }
            else
            {
                // master receiver
                i2c1con.index = 0;
                if (i2c1con.bytenum == 1)
                {
                    LL_I2C_AcknowledgeNextData(I2C1_I2C, LL_I2C_NACK);
                    LL_I2C_GenerateStopCondition(I2C1_I2C);
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }

        if ((SR1Register & (I2C_SR1_TXE | I2C_SR1_BTF)) == I2C_SR1_TXE)
        {
            temp = i2c1con.bytenum;
            if (temp)
            {
                LL_I2C_TransmitData8(I2C1_I2C, i2c1con.buffer[i2c1con.index++]);
                temp--;
                i2c1con.bytenum = temp;
                if (temp == 0)
                {
                    LL_I2C_DisableIT_BUF(I2C1_I2C);
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }

        if ((SR1Register & (I2C_SR1_TXE | I2C_SR1_BTF)) == (I2C_SR1_TXE | I2C_SR1_BTF)) {
            if (i2c1con.bytenum2)
            {
                i2c1con.bytenum = i2c1con.bytenum2;
                i2c1con.bytenum2 = 0;
                i2c1con.address++;
                LL_I2C_EnableIT_BUF(I2C1_I2C);
                LL_I2C_GenerateStartCondition(I2C1_I2C);
                LL_I2C_TransmitData8(I2C1_I2C, 0);  // !!!
            }
            else
            {
                LL_I2C_GenerateStopCondition(I2C1_I2C);
                I2C1->CR2 &= ~I2C_CR2_ITEVTEN;
                LL_I2C_DisableIT_ERR(I2C1_I2C);
            }
            SR1Register = 0;
            SR2Register = 0;
        }

        if ((SR1Register & I2C_SR1_RXNE) == I2C_SR1_RXNE)
        {
            temp = i2c1con.index;
            i2c1con.buffer[temp] = LL_I2C_ReceiveData8(I2C1_I2C);
            temp++;
            i2c1con.index = temp;
            temp = i2c1con.bytenum;
            temp--;
            i2c1con.bytenum = temp;
            if (temp == 1)
            {
                LL_I2C_AcknowledgeNextData(I2C1_I2C, LL_I2C_NACK);
                LL_I2C_GenerateStopCondition(I2C1_I2C);
            }
            SR1Register = 0;
            SR2Register = 0;
        }
    }  // if I2C1 is master
    return;
}


/* I2C1 Error interrupt handler */
void I2C1_ERROR_IRQ_HANDLER(void)
{
    uint32_t SR1Register = I2C1_I2C->SR1;

    if (SR1Register & I2C_SR1_AF)
    {
        LL_I2C_ClearFlag_AF(I2C1_I2C);
        i2c1con.error.noack1 = 1;
    }

    return;
}

/* ---------------------------------------------------------------------------*/

/** @} */

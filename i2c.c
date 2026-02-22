/*!
 * \addtogroup I2CDriver I2C
 * \brief I2C1 interconnection bus low-level driver
 * @{
 */
 
/*!
 * \file    i2c.c
 * \brief   I2C1 interconnection bus low-level driver source
 * \author  OK1CTR
 * \version 1.0
 * \date    17.07.2018
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "i2c.h"


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


/*!\brief Default I2C1 configuration
 * \note Configure I/O ports for SDA and SCL before.
 * \note No timeouts and fault handling.
 */
void i2c1_init(void)
{
	uint8_t n;

	// PIN I2C1 - SCL, SDA = PB6, PB7
	GPIOB->CRL &= CONFMASK(6); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_OD, 6);
	GPIOB->CRL &= CONFMASK(7); GPIOB->CRL |= GPIOCONF(GPIO_M_OUT02, GPIO_AFIO_OD, 7);
	// reset I2C1 peripherial
	RCC->APB1RSTR = RCC_APB1RSTR_I2C1RST;
	for (n = 0; n < 255; n++) __nop();
	RCC->APB1RSTR = 0;
	// initialisation
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	I2C1->CR2 &= 0x3F; I2C1->CR2 |= 4;
	I2C1->TRISE = 5; I2C1->CCR = 240; I2C1->CR1 = I2C_CR1_ACK | I2C_CR1_PE;	// 100 kHz
	NVIC_EnableIRQ(I2C1_EV_IRQn);
	NVIC_EnableIRQ(I2C1_ER_IRQn);
	return;
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

/*! @} */

/**
 * @file       i2c.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Auxiliary I2C bus module
 *
 * @addtogroup grI2C
 * @{
 */

#ifndef _I2C_H_
#define _I2C_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Functions -----------------------------------------------------------------*/

/**
 * @brief I2C1 initialization
 */
extern void i2c_init();

//! No ACK from the I2C slave
#define I2C_ERR_NOACK 0x01
//! I2C communication timeout
#define I2C_ERR_TMOUT 0x02


//! I2C-1 bus communication in progress test
#define i2c1_is_busy() (I2C1->SR2 & I2C_SR2_BUSY)
//! I2C-1 bus wait to communication finished or error raised
#define i2c1_wait() while (I2C1->SR2 & I2C_SR2_BUSY) { if (i2c1_error != 0 || utmr1 == 0) break; }


//! I2C-1 data buffer
extern uint8_t *i2c1_buffer;
//! I2C-1 error flags including timeout
extern uint8_t i2c1_error;

/*!\brief Write buffer content to the I2C slave
 * \note The function is non blocking, use i2c1_wait() to block the program execution.
 * \note 2018-04-30 LIS2DS12TR: Sleep 3 us or more before every I2C operation necessary!
 * \param addr I2C slave address
 * \param reg I2C slave internal register address
 * \param n Number of bytes to be written
 */
extern void i2c1_write_buf(uint8_t addr, uint8_t reg, uint8_t n);

/*!\brief Read content from the I2C slave
 * \note The function is non blocking, use i2c1_wait() to block the program execution.
 * \note 2018-04-30 LIS2DS12TR: Sleep 3 us or more before every I2C operation necessary!
 * \param addr I2C slave address
 * \param reg I2C slave internal register address
 * \param n Number of bytes to be read
 */
extern void i2c1_read_buf(uint8_t addr, uint8_t reg, uint8_t n);



/**
 * @brief Send byte to given I2C device
 * @param dev_addr Slave device address
 * @param data Byte to write
 */
//extern void i2c_write(uint8_t dev_addr, uint8_t data);

/* ---------------------------------------------------------------------------*/

#endif  /* _I2C_H_ */

/** @} */

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

/**
 * @brief Send byte to given I2C device
 * @param dev_addr Slave device address
 * @param data Byte to write
 */
extern void i2c_write(uint8_t dev_addr, uint8_t data);

/* ---------------------------------------------------------------------------*/

#endif  /* _I2C_H_ */

/** @} */

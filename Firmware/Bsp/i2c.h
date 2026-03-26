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
#include <stdbool.h>

/* Typedefs ------------------------------------------------------------------*/

/*! I2C error flags */
typedef struct
{
    uint8_t noack1:1;    ///< No ACK from the I2C1 slave
    uint8_t timeout1:1;  ///< I2C1 communication timeout
    uint8_t res:6;
} i2c_error_t;

/* Functions -----------------------------------------------------------------*/

/**
 * @brief I2C1 initialization
 */
extern void i2c_init();

/**
 * @brief Assign transmit and receive data buffer pointer for I2C operations
 * @param buf_ptr Pointer to data buffer
 */
extern void set_buffer(uint8_t *buf_ptr);

/*!\brief Write buffer content to the I2C slave
 * \note The function is non blocking, use i2c1_wait() to block the program execution.
 * \param addr I2C slave address
 * \param reg I2C slave internal register address
 * \param n Number of bytes to be written
 */
extern void i2c1_write_buf(uint8_t addr, uint8_t reg, uint8_t n);

/*!\brief Read content from the I2C slave into the buffer
 * \note The function is non blocking, use i2c1_wait() to block the program execution.
 * \param addr I2C slave address
 * \param reg I2C slave internal register address
 * \param n Number of bytes to be read
 */
extern void i2c1_read_buf(uint8_t addr, uint8_t reg, uint8_t n);

/**
 * @brief Return actual I2C error state
 * @return Actual error state
 */
extern i2c_error_t get_i2c_error();

/* ---------------------------------------------------------------------------*/

#endif  /* _I2C_H_ */

/** @} */

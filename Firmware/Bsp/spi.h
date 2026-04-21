/**
 * @file       spi.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      SPI communication module
 *
 * @addtogroup grSPI
 * @{
 */

#ifndef _SPI_H_
#define _SPI_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Initialize SPI1
 */
extern void spi1_init();

/**
 * @brief SPI1 transfer of 8 bits
 * @param data_tx Data byte to be transmitted
 * @return Data byte received
 */
extern uint8_t spi1_trx8(uint8_t data_tx);

/* ---------------------------------------------------------------------------*/

#endif  /* _SPI_H_ */

/** @} */

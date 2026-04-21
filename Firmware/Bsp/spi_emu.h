/**
 * @file       spi_emu.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      Emulated SPI communication module
 *
 * @addtogroup grSPIemu
 * @{
 */

#ifndef _SPI_EMU_H_
#define _SPI_EMU_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Initialize emulated SPI
 */
extern void spi_emu_init();

/**
 * @brief Emulated SPI transfer of 8 bits
 * @param data_tx Data byte to be transmitted
 * @return Data byte received
 */
extern uint8_t spi_emu_trx8(uint8_t data_tx);

/* ---------------------------------------------------------------------------*/

#endif  /* _SPI_EMU_H_ */

/** @} */

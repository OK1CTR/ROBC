/**
 * @file       serial.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      USART serial ring buffer communication module for ports
 *
 * @addtogroup grSerial
 * @{
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <main.h>

/* Exported C functions ------------------------------------------------------*/

#ifdef CONSOLE_SERIAL_1

/**
 * @brief Write a message into the output buffer
 * @param file Standard parameter, not used here
 * @param *ptr Pointer to message to write
 * @param len Message length, only 1 here
 * @return Number of characters written, only 1 here
 */
extern int _write(int file, char *ptr, int len);

/**
 * @brief Wait to all characters are sent
 */
extern void _flush();

#endif  /* CONSOLE_SERIAL_1 */

/* Functions ----------------------------------------------------------------*/

/**
 * @brief Initialize the serial interface and data buffer
 */
extern void serial_init();

/**
 * @brief Serial interface regular job
 * @return True if communication in progress
 */
extern bool serial_job();

/**
 * @brief Transmit next character from buffer via serial interface
 */
extern void serial_transmit();

/**
 * @brief Insert one character into the transmit data buffer
 */
extern int serial_insert_tx(uint8_t c);

/**
 * @brief Wait for empty TX buffer
 */
extern void serial_wait_tx_empty();

/**
 * @brief Are any characters in the RX buffer
 */
extern bool serial_is_rx_not_empty();

/**
 * @brief Pull a character from the RX buffer
 */
extern char serial_pull_rx();

/* ---------------------------------------------------------------------------*/

#endif /* _SERIAL_H_ */

/** @} */

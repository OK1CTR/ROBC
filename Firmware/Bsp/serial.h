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

#include <stdint.h>
#include <stdbool.h>
#include <main.h>

/* Typedefs ------------------------------------------------------------------*/

/* Serial port selection */
typedef enum
{
    serial1 = 1,
    serial2 = 2
} serial_e;

/* Exported C functions ------------------------------------------------------*/

#ifdef CONSOLE_SERIAL_1

/**
 * @brief Write a message into the output buffer
 * @param file Standard parameter, not used here
 * @param *data Pointer to message to write
 * @param len Message length, only 1 here
 * @return Number of characters written, only 1 here
 */
extern int _write(int file, char *data, int len);

/**
 * @brief Wait to all characters are sent
 */
extern void _flush();

#endif  /* CONSOLE_SERIAL_1 */

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Initialize the serial interface and data buffer
 */
extern void serial_init();

/**
 * @brief Serial interface regular job
 * @return False
 */
extern bool serial_job();

/**
 * @brief Insert one character into the transmit data buffer
 * @param serial Serial port selection
 * @param c Character to send
 */
extern int serial_insert_tx(serial_e ser, uint8_t c);

/**
 * @brief Wait for empty TX buffer
 * @param serial Serial port selection
 */
extern void serial_wait_tx_empty(serial_e ser);

/**
 * @brief Are any characters in the RX buffer
 * @param serial Serial port selection
 */
extern bool serial_is_rx_not_empty(serial_e ser);

/**
 * @brief Pull a character from the RX buffer
 * @param serial Serial port selection
 */
extern char serial_pull_rx(serial_e ser);

/* ---------------------------------------------------------------------------*/

#endif /* _SERIAL_H_ */

/** @} */

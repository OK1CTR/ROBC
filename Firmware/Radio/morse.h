/**
 * @file       morse.h
 * @author     OK1CTR
 * @date       Apr 2026
 * @brief      Morse code transmitter module working with the AX radio in ASK mode
 *
 * @addtogroup grMorse
 * @{
 */

#ifndef _MORSE_H_
#define _MORSE_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Morse keyer initialization according to WPM
 * @param wpm Keying speed in wpm
 */
extern void morse_init(uint32_t wpm);

/**
 * @brief Morse keyer deinitialization
 */
extern void morse_deinit();

/**
 * @brief Morse keyer regular job
 * @details Call it from \b idle function
 */
extern void morse_job(void);

/**
 * @brief Store new telegram message to transmit buffer and start sending
 * @param telegram New telegram text
 * @return Number of characters placed into buffer
 */
extern uint32_t morse_send(char *telegram);

/**
 * @brief Return true if Morse transmission is still in progress
 * @return True if transmit
 */
extern bool morse_is_transmit();

/**
 * @brief Return true if telegram FIFO is empty
 * @return True if FIFO is free
 */
extern bool morse_is_fifo_empty();

/* ---------------------------------------------------------------------------*/

#endif  /* _MORSE_H_ */

/** @} */

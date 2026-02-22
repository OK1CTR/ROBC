/*!
 * \addtogroup Morse Morse
 * \brief Morse code transmitter working with the AX radio in ASK mode 
 * @{
 */
 
/*!
 * \file    morse.h
 * \brief   Morse code transmitter working with the AX radio in ASK mode, header
 * \author  OK1CTR
 * \version 2.0
 * \date    28.08.2018
 */

#ifndef _MORSE_H_
#define _MORSE_H_

/*! @name Timing intervals in counter ticks
 *  @{
 */
//! Dot
#define DOT     1
//! Dash
#define DASH    (3 * DOT)
//! Space between characters
#define DASH_SP (5 * DOT)
//! Space between words
#define WORD_SP (2 * DASH_SP + DOT)
/*! @} */

/*! @name State machine flags (internal)
 *  @{
 */
//! Mark transmission progress
#define FLAG_RUN      0x20
//! Space between characters or dash
#define FLAG_LONG     0x01
//! Space between words
#define FLAG_WORD     0x04
//! Space transmission progress
#define FLAG_SPACE    0x02
/*! @} */

/*! @name State machine flags (querry)
 *  @{
 */
//! Free to transmit
#define FLAG_FREE     0x00
/*! @} */

/*! @name State machine flags (request)
 *  @{
 */
//! New request enter
#define FLAG_ENTER    0x10
//! Dot
#define FLAG_DOT      0x00
//! Dash
#define FLAG_DASH     0x01
//! Short space
#define FLAG_DOT_SP   0x02
//! Long space
#define FLAG_DASH_SP  0x03
//! Word space
#define FLAG_WORD_SP  0x06
/*! @} */

/*! @name Keyer status
 *  @{
 */
//! Character transmission in progress
#define STATUS_CHARIN    0x01
//! Space requested
#define STATUS_SPACE     0x02
//! Free to transmit
#define STATUS_FREE      0x00
/*! @} */


//! Is keyer free to transmit?
#define morse_key_free() (m_flag != FLAG_FREE)

//! Is transmit buffer empty?
#define morse_buf_free() (morse_wr == morse_rd)

//! Are or will be next characters transmitted?
#define morse_transmit() (m_status != STATUS_FREE || m_flag != FLAG_FREE)

//! Is PA (transmitter) switched on?
#define morse_pa_on() (m_pa)


//! Morse mark or space transmission flag register
extern volatile uint8_t m_flag;
//! Morse transmitter status register
extern volatile uint8_t m_status;
//! Transmit buffer reading pointer
extern uint8_t *morse_rd;
//! Transmit buffer writing pointer
extern uint8_t *morse_wr;
//! Morse PA status register
extern volatile uint8_t m_pa;



/*! \brief Morse keyer initialisation according to WPM
 *  \param wpm Keying speed in wpm
 */
extern void morse_init(uint8_t wpm);

/*! \brief Place new message to transmit buffer
 *  \param s New message text
 *  \return Number of characters placed into buffer
 */
extern uint8_t morse_send(uint8_t *s);

/*! \brief Main loop of keyer function
 *  \details Call it from \b idle function.
 */
extern void morse_loop(void);

#endif

/*! @} */

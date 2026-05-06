/**
 * @file       ax25.h
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      AX.25 and HDLC protocol module
 *
 * @addtogroup grAx25
 * @{
 */

#ifndef _AX25_H_
#define _AX25_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>


//! AX.25 Flag pattern (0x7E)
#define HDLC_FLAG       0x7E
//! Flag parameter for AX.25 packet preamble (0x18)
#define AX25_FLAG_PAR   0x10
//! Flag count in AX.25 packet preamble 1 .. 255
#define AX25_PRE_LEN      32
//! Flag count in packet tail 1 .. 255. Don't use less, or the packet will fail when PA_OFF is sent after.
#define AX25_TAIL_LEN      3
//! AX.25 telemetry source callsign
#define AX25_CALL_SRC   "OK1CTR"
//! AX.25 telemetry destination callsign
#define AX25_CALL_DEST  "OK1CTR"
//! AX.25 telemetry source SSID 0 .. 15, default 0
#define AX25_SSID_SRC      0
//! AX.25 telemetry destination SSID 0 .. 15, default 1
#define AX25_SSID_DEST     1
//! AX.25 UI frame controll field
#define AX25_CMD        0x03
//! AX.25 packet type identifier
#define AX25_PID        0xF0


/*
Attention to preamble length when using a scrambler! 8 is not enough! Use 10+.
The problem might be caused by a too tight AFC. With AFC range 1 kHz is preamble length 8 possible.
*/
//! Flag count in HDLC packet preamble for safe operation with a scrambler
#define HDLC_SCRP_LEN     32
//! Flag count in HDLC packet preamble 1 .. 255, long type
#define HDLC_PREAMB_1     32
//! Flag count in HDLC packet preamble 1 .. 255, short type
#define HDLC_PREAMB_2      2

//! The position in packet serie - first packet
#define SER_POS_1          1
//! The position in packet serie - last packet
#define SER_POS_N          2

/* Functions -----------------------------------------------------------------*/

/**
 * @brief Initialize the AX.25 and HDLC transmitter
 */
extern void ax25_init(void);

/**
 * @brief CRC-CCITT calculation for AX.25 Frame Check Sequence (FCS)
 * @detail Init the memory with 0xFFFF. Before sending invert all bits. Send LSB first.
 * @param c Input data octet
 * @param f Memory state
 * @return New memory state
 */
extern uint16_t ax25_crc_calc(uint8_t c, uint16_t f);

/**
 * @brief Send the TXCTL command into the AX5043 FIFO
 * @param param The parameter of the TXCTL command
 * @note Add FIFO timeout !!!
 */
extern void ax25_send_txctl(uint8_t param);

/*! \brief Send HDLC/AX.25/FEC universal flag chunk into the AX5043 FIFO
 *  \param patern Flag pattern character, HDLC or AX.25 default is 0x7E
 *  \param length Number of repeated flags, 1 .. 255
 *  \param parameter The parameter holding the RAW/UNENC bit values to specify data processing
 *  \note Waiting for 5 free places in FIFO.
 */
extern void hdlc_send_flag(uint8_t pattern, uint8_t length, uint8_t parameter);

/*! \brief Send AX.25 data chunk into the AX5043 FIFO
 *  \param data Pointer to data buffer
 *  \param length Number of data octers to send
 *  \param packetstat Command parameter used mainly for packet start or stop flag, or zero.
 *  \note FIFO sync?
 */
extern void ax25_send_inf(uint8_t *data, uint8_t length, uint8_t pacstart);

/*! \brief Send AX.25 data chunk with CRC into the AX5043 FIFO
 *  \param crc The CCITT CRC
 *  \note FIFO sync?
 */
extern void ax25_send_crc(uint16_t crc);

/*! \brief Send AX.25 telemetry message
 *  \param msg The pointer to the payload buffer
 *  \param length Buffer length or 0 for null terminated message
 */
extern void ax25_send_msg(uint8_t *msg, uint8_t length);

/*! \brief Send HDLC ground contact message
 *  \param msg The pointer to the payload buffer
 *  \param length Buffer length or 0 for null terminated message
 *  \param position The position in packet serie, first, middle, last
 */
extern void hdlc_send_msg(uint8_t *msg, uint8_t length, uint8_t position);

/* ---------------------------------------------------------------------------*/

#endif  /* _AX25_H_ */

/** @} */

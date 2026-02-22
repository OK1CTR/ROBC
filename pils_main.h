/*!
 * \addtogroup Main Main
 * \brief PilsenCUBE COM-OBC main module
 * @{
 */

/*!
 * \file    pils_main.h
 * \brief   PilsenCUBE COM-OBC main module header
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#ifndef _PILS_MAIN_H_
#define _PILS_MAIN_H_


/*! @name Special definitions for debug purposes.
 *  @{
 */
//! Use AX5043 transceiver DAC output as receiver debug analog signal.
#define _DEBUG_ANALOG_
//! Use AX5043 transceiver digital outputs DATA and DCLK as receiver debug signals.
#define _DEBUG_DIGITAL_
/*! @} */


/*! @name Test procedure selection.
 *  @{
 */
//! Functionality of the PilsenCUBE communication bus test
#define TEST_BUS         0x0001
//#define TEST_PROCEDURE TEST_BUS

//! Functionality of the FRAM storage test
#define TEST_FRAM        0x0002
//#define TEST_PROCEDURE TEST_FRAM

//! I2C ADC measurement test
#define TEST_I2C_ADC     0x0003
//#define TEST_PROCEDURE TEST_I2C_ADC

//! Standard ADC measurement test
#define TEST_STD_ADC     0x0004
//#define TEST_PROCEDURE TEST_STM_ADC

//! Radio analog FM transmission test
#define TEST_RAD_FM      0x0005
//#define TEST_PROCEDURE TEST_RAD_FM

//! Radio FM reception test
#define TEST_RAD_FM_RX   0x0006
//#define TEST_PROCEDURE TEST_RAD_FM_RX

//! Radio ASK MORSE transmission test
#define TEST_RAD_MORSE   0x0007
//#define TEST_PROCEDURE TEST_RAD_MORSE

//! Radio AFSK/GMSK AX.25 Packet Radio transmission test
#define TEST_RAD_PACKET  0x0008
//#define TEST_PROCEDURE TEST_RAD_PACKET

//! Radio GMSK HDLC transmission test
#define TEST_RAD_HDLC_TX 0x0009
//**********************#define TEST_PROCEDURE TEST_RAD_HDLC_TX

//! Radio GMSK HDLC reception test
#define TEST_RAD_HDLC_RX 0x000A
//**********************#define TEST_PROCEDURE TEST_RAD_HDLC_RX

//! Radio GMSK HDLC TRX test
#define TEST_RAD_HDLC_TRX 0x000B
//**********************#define TEST_PROCEDURE TEST_RAD_HDLC_TRX

//! Radio GMSK HDLC transmission test
#define TEST_RAD_FEC_TX 0x000C
//#define TEST_PROCEDURE TEST_RAD_FEC_TX

//! Radio GMSK HDLC reception test
#define TEST_RAD_FEC_RX 0x000D
//#define TEST_PROCEDURE TEST_RAD_FEC_RX

//! Test the communication with another unit, OBC is bus gateway
#define TEST_BUS_GATEWAY 0x000E
//#define TEST_PROCEDURE TEST_BUS_GATEWAY

//! Test the backup domain and the RTC
#define TEST_BACKUP_DOM 0x000F
//#define TEST_PROCEDURE TEST_BACKUP_DOM

//! Test procedure for AMP, PilsenCUBE bus - I2C bridge
#define TEST_AMP_I2C_MS 0x0010

//!  If defined, selected test procedure is run instead standard COM-OBC operation
#define TEST_PROCEDURE TEST_AMP_I2C_MS

/*! @} */


//! I2C-1 data buffer size in Bytes
#define I2C1_BF_SIZE 16


//! I2C-1 data buffer
extern uint8_t i2c_bf1[I2C1_BF_SIZE];


#if defined(TEST_PROCEDURE)
/*! \brief Optional test procedure
 */
extern void test_procedure(void);
#endif

/*! \brief Idle function
 */
extern void idle(void);

#endif

/*! @} */

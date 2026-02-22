/*!
 * \addtogroup GlConfig Global Config
 * \brief PilsenCUBE COM-OBC global configuration module
 * \note Default APB2 clock frequency is 24 MHz.
 * @{
 */

/*!
 * \file    pils_config.h
 * \brief   PilsenCUBE COM-OBC global configuration module
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#ifndef _CONFIG_H_
#define _CONFIG_H_


//! Important selection of the HW board version
#define ROBC_VER_2
//#define ROBC_VER_3


/*! @name PilsenCUBE communication bus settings
 *  @{
 */
//! Baud rate on ch.1 PilsenCUBE satellite bus
#define PILS_BUS1_RATE 250000
//! Baud rate on ch.2 PilsenCUBE satellite bus
#define PILS_BUS2_RATE 250000
//! Receive data buffer size in Bytes, channel 1
#define PILS_RXBF1_SIZE 128
//! Receive data buffer size in Bytes, channel 2
#define PILS_RXBF2_SIZE 128

//! The OBC PilsenCUBE address on channel 1
#define PILS_OBC_ADR1 0xCC
//! The OBC PilsenCUBE address on channel 2
#define PILS_OBC_ADR2 0xCC
/*! @} */


/*! @name PilsenCUBE FRAM data storage SPI settings
 *  @{
 */
//! The Baud Rate register for SPI1 working as FRAM storage interface (0x0 - 0x7). The APB2 clock is divided by: 0 :2 | 1 :4 | 2 :8 | 3 :16 | 4 :32 | 5 :64 | 6 :128 | 7 :256
#define FRAM_SPI_BAUD_RATE 0x4
	#if FRAM_SPI_BAUD_RATE < 0 || FRAM_SPI_BAUD_RATE > 7
		#error pils_config.h: Wrong value of FRAM_SPI_BAUD_RATE.
	#endif
/*! @} */


/*! @name PilsenCUBE Radio SPI settings
 *  @{
 */
//! The Baud Rate register for SPI2 working as Radio interface (0x0 - 0x7). The APB1 clock is divided by: 0 :2 | 1 :4 | 2 :8 | 3 :16 | 4 :32 | 5 :64 | 6 :128 | 7 :256
#define RADIO_SPI_BAUD_RATE 0x4
	#if RADIO_SPI_BAUD_RATE < 0 || RADIO_SPI_BAUD_RATE > 7
		#error pils_config.h: Wrong value of RADIO_SPI_BAUD_RATE.
	#endif
/*! @} */


/*! @name PilsenCUBE radio settings
 *  @{
 */
//! Reference xtal frequency in Hz
#define FRQ_XTAL 16368000L
//! Default working frequency in Hz
#define FREQUENCY 435000000L
//! Relative frequency error x 1E6. Undef to switch error compensation off.
#define RF_FRQ_ERR -973563L
// Frequency correction enabled if defined
#undef FRQ_ERR
//! SPI signal timming delay in us
#define SPI_DELAY 10
//! PA disabled if 1
#define PA_DISABLE 1
/*! @} */


/*! @name PilsenCUBE morse settings
 *  @{
 */
//! Default morse keying speed in WPM
#define MORSE_RATE 60
//! Morse transmit buffer length
#define MORSE_BFLEN 50
/*! @} */

#endif
/*! @} */

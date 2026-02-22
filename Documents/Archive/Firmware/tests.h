/*!
 * \addtogroup TestConfig Test Config
 * \brief Common constants for tests
 * @{
 */

/*!
 * \file    tests.h
 * \brief   Common constants for tests
 * \author  OK1CTR
 * \version 1.0
 * \date    09.2019
 */
 
 
#ifndef _TESTS_H_
#define _TESTS_H_


//! Destination address used for test messages with no reply expected
#define TST_ADR  0x99
//! Special command character for test messages with machine readable content
#define TST_CHR  '?'
//! Special command character for test messages with machine readable content, received data which will continue in next packet
#define TST_RXD  '<'
//! Special command base character for test messages with machine readable content, received data final packet
#define TST_RXDF '>'


//! Message type identifier - fatal error
#define MTID_FATAL_ERROR 0xFF
//! I2C ADC results
#define MTID_ADC_RES1    0x01
//! STM32 internal ADC results for version 3 hardware
#define MTID_ADC_RES2    0x02
//! STM32 internal ADC results for version 2 hardware
#define MTID_ADC_RES3    0x03

#endif
/*! @} */

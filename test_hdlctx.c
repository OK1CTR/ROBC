/*!
 * \addtogroup  TestHDLCTx Test-HDLC-TX
 * \brief PilsenCUBE HDLC data packet transmission test
 * @{
 */

/*!
 * \file    test_hdlctx.c
 * \brief   PilsenCUBE HDLC data packet transmission test
 * \author  OK1CTR
 * \version 1.0
 * \date    12.2019
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"
#include "i2c.h"
#include "ax5043.h"
#include "ax25.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_HDLC_TX


//! Transmission period in seconds (Consider the message length!)
#define TEST_PER 5
//! Radio transmission mode
#define RF_BAUD_MODE G3RUH_9600
//! HDLC test message constant
#define HDLC_PKT_CONST "%08X-%02X-%02X...PilsenCUBE-SATELITE-UWB-PILSEN-0123456789ABCDEF"
//! HDLC test message maximal length (message lenhth + null character)
#define AX25_M00_LEN 65
//! Length of the serial packet
#define MSG_SER_N 16


//! Message buffer for PilsenCUBE BUS transmission
extern uint8_t msg_bf_tx[32];


//! Test mesage buffer
uint8_t msg00[AX25_M00_LEN];


/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i;  // main loop counter in seconds
	uint16_t msg_cnt = 0;  // message counter
	uint8_t msg_ser_n, a;

	i2c1_init(); // !!!!
	
	// test message initialization
	//memset(msg00, '.', AX25_M00_LEN);
	//memcpy(msg00, HDLC_PKT_CONST)
	
	// radio and protocol initialization
	ax_load_par(); // default radio parameter set activation
	ax_init();
	ax_pwrmode(AX_PWRMODE_TXS);
	ax_frequency(0, FREQUENCY, 1);
	ax_mode_g3ruh(RF_BAUD_MODE, AX_CRC_CRC32, AX_ENC_SCRAMBLER);
	pa_low = 1;  // PA output power limit
	ax25_init();
	pa_prot_ctrl(1);  // PA protecion 1=ON, 0=OFF
	ax_crc_init();  // initialize the HW CRC generator
	ax_fifo_init();
	ax_pwrmode(AX_PWRMODE_TX);

	while (1) {

		// watchdog reset
		wdt_trig();

		// one second timing
		sec_upd = 0;
		while (sec_upd == 0)
			idle();

		if (i >= TEST_PER) {
			msg_ser_n = 0;
			for (msg_ser_n = 0; msg_ser_n < MSG_SER_N; msg_ser_n++) {
				sprintf((char *) msg00, HDLC_PKT_CONST, msg_cnt, msg_ser_n, MSG_SER_N);
				if (msg_ser_n == 0) a = SER_POS_1; else a = 0;
				if (msg_ser_n == MSG_SER_N  - 1) a |= SER_POS_N;
				hdlc_send_msg(msg00, 64, a);
			}
			msg_cnt++;
			led_disp(msg_cnt & 0xFF);
			i = 0;
		}

		// send a status using the PilsenCUBE BUS
		msg_bf_tx[0] = i & 0xFF;
		msg_bf_tx[1] = ax_status >> 8;
		msg_bf_tx[2] = ax_status & 0xFF;
		pilbus_send(1, 0x99, 'b', 10, msg_bf_tx);

		// increment the main loop counter
		i++;
	}		
}


#endif


/*! @} */

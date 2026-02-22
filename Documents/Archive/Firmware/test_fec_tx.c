/*!
 * \addtogroup TestHDLCFECTx Test-HDLC-FEC-TX
 * \brief PilsenCUBE HDLC-FEC data packet transmission test
 * @{
 */

/*!
 * \file    test_fectx.c
 * \brief   PilsenCUBE HDLC-FEC data packet transmission test
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


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_FEC_TX


//! Transmission period in seconds (Consider the message length!)
#define TEST_PER 5
//! HDLC test message constant
#define HDLC_PKT_CONST "PilsenCUBE-SATELITE-UWB-PILSEN-0123456789ABCDEF"
//! HDLC test message maximal length
#define AX25_M00_LEN 64


//! Message buffer for PilsenCUBE BUS transmission
extern uint8_t msg_bf_tx[20];


//! Test mesage buffer
uint8_t msg00[AX25_M00_LEN];


/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i;  // main loop counter in seconds
	uint16_t msg_cnt = 0;  // message counter

	// test message initialization
	memset(msg00, '.', AX25_M00_LEN);

	// radio and protocol initialization
	ax_init();
	i2c1_init(); // !!!!
	ax_rw_N(0, 0, msg_bf_tx + 3, 5);  // AX5043 buffer read test
	ax_pwrmode(AX_PWRMODE_TXS);
	ax_frequency(0, FREQUENCY, 1);

	ax_mode_g3ruh(G3RUH_9600, AX_CRC_CRC32);

	ax_rw_2(1, 0x11, 0x00); // encoding must be OFF

	// FEC
	ax_rw_2(1, 0x18, 0x13);

	pa_low = 1;  // PA output power limit
	ax_pwrmode(AX_PWRMODE_TX);
	ax25_init();
	ax_fifo_init();
	pa_prot_ctrl(1);  // PA protecion 1=ON, 0=OFF

	//! constant preamble pattern transmission for measurement
#ifdef _RADIO_PATTERN_TEST_
	ax25_send_txctl(AX_TXCTL_PAON);  // PA on
	while (1) {
		ax25_send_flag(AX25_PREAMBLE, HDLC_PREAMB_1);
	}
#endif

	while (1) {

		// watchdog reset
		wdt_trig();

		// one second timing
		sec_upd = 0;
		while (sec_upd == 0)
			idle();

		if (i >= TEST_PER) {
			sprintf((char *) (msg00 + 51), "%08X", msg_cnt);
			sprintf((char *) msg00, "%08X-%s", msg_cnt, HDLC_PKT_CONST);
			hdlc_send_msg(msg00, 0);
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

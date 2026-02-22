/*!
 * \addtogroup TestPacket Test-Packet
 * \brief AX.25 Packet Radio beacon transmission test
 * @{
 */

/*!
 * \file    test_packet.c
 * \brief   AX.25 Packet Radio beacon transmission test
 * \author  OK1CTR
 * \version 1.0
 * \date    09.2019
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


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_PACKET


//! If defined, AFSK modulation is used, if not, GMSK/G3RUH instead
#define PACKET_AFSK
//! Transmission period in seconds (Consider the message length!)
#define TEST_PER 5
//! AX.25 test message buffer length
#define AX25_M00_LEN     64
//! AX.25 test message constant (50 chars + 14 spaces)
#define AX25_M00_STR "The PilsenCUBE satellite COM/OBC test @ @ @ @ \x7F\xFE\x7F\xFE              "


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
	strcpy((char *) msg00, AX25_M00_STR);

	// radio and protocol initialization
	ax_load_par(); // default radio parameter set activation
	ax_init();
	i2c1_init(); // !!!!
	ax_rw_N(0, 0, msg_bf_tx + 3, 5);  // AX5043 buffer read test
	ax_pwrmode(AX_PWRMODE_TXS);
	ax_frequency(0, FREQUENCY, 1);

#ifdef PACKET_AFSK
	ax_mode_afsk(AX_CRC_OFF);
	ax_rw_2(1, 0x11, 0x03); // encoding, NRZI, scrambler is OFF
	#else
	ax_mode_g3ruh(G3RUH_9600, AX_CRC_OFF, AX_ENC_SCRAMBLER);
	//ax_mode_g3ruh(G3RUH_19200, AX_CRC_OFF, AX_ENC_SCRAMBLER);
#endif

	pa_low = 1;  // PA output power limit
	ax_pwrmode(AX_PWRMODE_TX);
	ax25_init();
	ax_fifo_init();
	pa_prot_ctrl(1);  // PA protecion 1=ON, 0=OFF

	//! constant preamble pattern transmission for measurement
#ifdef _RADIO_PATTERN_TEST_
	ax25_send_txctl(AX_TXCTL_PAON);  // PA on
	while (1) {
		hdlc_send_flag(HDLC_FLAG, AX25_PRE_LEN, AX25_FLAG_PAR);
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
//			ax25_send_flag(10);
//			ax25_send_msg((unsigned char*) "HOVNO PRDEL SRACKA TO JE NASE ZNACKA", 0);
//			ax25_send_msg((unsigned char*) "PPPPPPPPPP PRDELLL PRDELLL PRDELL !!!!!", 0);
//			ax25_send_msg((unsigned char*) "v", 0);
			sprintf((char *) (msg00 + 51), "%08X", msg_cnt);
			ax25_send_msg(msg00, 0);
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

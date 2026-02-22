/*!
 * \addtogroup TestHDLCTRx Test-HDLC-TRX
 * \brief PilsenCUBE HDLC data packet switched reception + transmission test
 * @{
 */

/*!
 * \file    test_hdlctrx.c
 * \brief   PilsenCUBE HDLC data packet switched reception + transmission test
 * \author  OK1CTR
 * \version 1.0
 * \date    03.2020
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32f10x.h" // Still needed?
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"
#include "i2c.h"
#include "ax5043.h"
#include "ax25.h"
#include "tests.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_HDLC_TRX


//! Radio transmission mode
#define RF_BAUD_MODE G3RUH_9600
//! Message buffer length
#define MSG_BF_LEN 32

//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[32];
//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf[MSG_BF_LEN];


/* Non returning test loop. */
void test_procedure(void)
{
	uint8_t a, j, x = 0;

	i2c1_init(); // !!!!

	// Radio receive init
	ax_load_par(); // default radio parameter set activation
	ax_init();
	ax_pwrmode(AX_PWRMODE_RXS);
	ax_frequency(0, FREQUENCY, 1);
	ax_mode_g3ruh(RF_BAUD_MODE, AX_CRC_CRC32, AX_ENC_SCRAMBLER);
	pa_low = 1;  // PA output power limit
	ax25_init();
	pa_prot_ctrl(1);  // PA protecion 1=ON, 0=OFF
	ax_crc_init();  // initialize the HW CRC generator
	ax_fifo_init();
	ax_pwrmode(AX_PWRMODE_RX);

	while (1) {

		// ratchdog reset
		wdt_trig();

		// resend received data from PilsenCUBE bus ch.1 to Radio
		if (us1_rx_st == US_RX_OK) {
			if (us1_rx_cmd == 'T') {
				// init Radio TX mode
				ax_pwrmode(0);
				ax_rw_2(1, 0x07, 0x00); // RIRQ OUTPUT DISABLE
				ax_pwrmode(AX_PWRMODE_TX);

				// wait for xtal
				while (ax_rw_2(0, 0x1D, 0x00) == 0) {
					sleep_us(500);
				}
				// wait for FIFO ready
				while ((ax_rw_2(0, 0x03, 0x00) & 0x08) == 0) {
					sleep_us(500);
				}

				// send received message via the radio, use us1_rx_cmd, us1_nrx, pilbus1_rxbf
				hdlc_send_msg(pilbus1_rxbf, us1_nrx, SER_POS_1 | SER_POS_N);
				//hdlc_send_msg(pilbus1_rxbf, us1_nrx, SER_POS_N);
				// wait for finish the transmission
				while ((ax_rw_2(0, 0x1C, 0x00) & 0x0F) != 0) {
					sleep_us(5000);
				}

				// reinit Radio RX mode
				ax_pwrmode(0);
				// reconfigure RIRQ
				ax_rw_2(1, 0x07, 0x01);  // FIFO not empty
				EXTI->PR |= 1 << 9; // clear pending ints
				ax_trx_st = AX_TRX_WAIT;
				ax_pwrmode(AX_PWRMODE_RX);
				x = 0;
			}
			pilbus_rxreinit(1);
		}

		// reporting error on PilsenCUBE bus ch.1 to ch.1
		if (us1_rx_st == US_RX_ERROR) {
			for (j = 0; j < 3; j++)
				msg_bf_tx[j] = '!';
			msg_bf_tx[j] = ':';
			msg_bf_tx[j + 1] = '1';
			pilbus_send(1, 0x99, 'E', 5, msg_bf_tx);
			sleep_us(5000);
			pilbus_rxreinit(1);
		}

		// resend received message from Radio to PilsenCUBE bus
		if (ax_trx_st == AX_TRX_DATA) {
			while((a = ax_rw_2(0, 0x2B, 0x00)) > 0) {
				if (a > 32) a = 32;
				ax_rw_N(0, 0x29, msg_bf_tx, a);
				while (us1_tx_str != US_TXSR_IDLE)
					;
				pilbus_send(1, 0x99, 'R', a, msg_bf_tx);
				ax_trx_st = AX_TRX_WAIT;  // reset the transceiver state machine
			}
			x++;
		}
		led_disp(x);
	}
}


#endif


/*! @} */

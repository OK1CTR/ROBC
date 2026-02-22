/*!
 * \addtogroup TestHDLCRx Test-HDLC-RX
 * \brief PilsenCUBE HDLC data packet reception test
 * @{
 */

/*!
 * \file    test_hdlcrx.c
 * \brief   PilsenCUBE HDLC data packet reception test
 * \author  OK1CTR
 * \version 1.0
 * \date    09.2019
 */


#include <stdint.h>

#include "stm32f10x.h" // still needed??
#include "stm32f10x_gpio.h" // still needed??
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "ax5043.h"
#include "i2c.h"
#include "pils_bus.h"
#include "tests.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_HDLC_RX


//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[32];


/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i;
	uint8_t x = 0, a, b;

	i2c1_init(); // !!!!

	// radio and protocol initialization
	ax_load_par(); // default radio parameter set activation
	ax_init();
	ax_pwrmode(AX_PWRMODE_RXS);
	ax_frequency(0, FREQUENCY - 0L, 1);
	ax_mode_g3ruh(G3RUH_9600, AX_CRC_CRC32, AX_ENC_SCRAMBLER);
	pa_low = 1;
	pa_prot_ctrl(1);
	ax_crc_init();  // initialize the HW CRC generator
	ax_fifo_init();
	ax_pwrmode(AX_PWRMODE_RX);
	
	while (1) {

		// watchdog reset
		wdt_trig();

		// check the radio IRQ output
		if (/*GPIOB->IDR & 0x0200*/ ax_trx_st == AX_TRX_DATA) {
			/* radioevent debug message
			msg_bf_tx[6] = ax_rw_2(0, 0x1C, 0x00); // radio state
			msg_bf_tx[0] = ax_rw_2(0, 0x0C, 0x00); // int rq 1
			msg_bf_tx[1] = ax_rw_2(0, 0x0D, 0x00); // int rq 0
			msg_bf_tx[2] = ax_rw_2(0, 0x0E, 0x00); // radio event rq 1
			msg_bf_tx[3] = ax_rw_2(0, 0x0F, 0x00); // radio event rq 0
			msg_bf_tx[4] = ax_rw_2(0, 0x2B, 0x00); // fifo count L
			msg_bf_tx[5] = ax_rw_3(0, 0x118, 0x00); // current parameter set
			pilbus_send(1, 'X', 'Y', 7, msg_bf_tx);
			x++;
			*/
			// read the radio event request
			//ax_rw_2(0, 0x0E, 0x00);
			//ax_rw_2(0, 0x0F, 0x00);

			// incomming data packet processing
			while ((a = ax_rw_2(0, 0x2B, 0x00)) > 0) { // fifo count L
				if (a > 32) {
					a = 32;
					b = TST_RXD;
				} else b = TST_RXDF; // fianl part of incomming packet
				ax_rw_N(0, 0x29, msg_bf_tx, a);
				while (us1_tx_str != US_TXSR_IDLE)
					;
				pilbus_send(1, TST_ADR, b, a, msg_bf_tx);
				ax_trx_st = AX_TRX_WAIT; // reset the transceiver state machine
			}
			x++;
		}
		
		// increment the main loop counter
		i++;
		led_disp(x);
	}		
}


#endif


/*! @} */

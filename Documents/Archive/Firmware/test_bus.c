/*!
 * \addtogroup TestBus Test-Bus
 * \brief PilsenCUBE Communication Bus test
 * @{
 */

/*!
 * \file    test_bus.c
 * \brief   PilsenCUBE Communication Bus Test
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#include <stdint.h>

#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_BUS

//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[20];


/* Non returning test loop */
void test_procedure(void)
{
	uint8_t data[] = "HELLO!!!";
	uint32_t i;
	uint8_t j;


	// Initial message send
	pilbus_send(1, 0x99, 'b', 8, data);
	
	while (1) {

		// Watchdog reset
		wdt_trig();

		// One second timing
		sec_upd = 0;
		while (sec_upd == 0)
			;

		// Reply bytes + 1 with channel identification on PilsenCUBE bus ch. 1
		if (us1_rx_st == US_RX_OK) {
			for (j = 0; j < us1_nrx; j++)
				msg_bf_tx[j] = pilbus1_rxbf[j] + 1;
			msg_bf_tx[j] = ':';
			msg_bf_tx[j + 1] = '1';
			pilbus_send(1, 0x99, 'b', us1_nrx + 2, msg_bf_tx);
			sleep_us(5000);
			pilbus_rxreinit(1);
		}
		// Reply bytes + 1 with channel identification on PilsenCUBE bus ch. 2
		if (us2_rx_st == US_RX_OK) {
			for (j = 0; j < us2_nrx; j++)
				msg_bf_tx[j] = pilbus2_rxbf[j] + 1;
			msg_bf_tx[j] = ':';
			msg_bf_tx[j + 1] = '1';
			pilbus_send(1, 0x99, 'b', us2_nrx + 2, msg_bf_tx);
			sleep_us(5000);
			pilbus_rxreinit(2);
		}
		// Error reporting on PilsenCUBE bus ch. 1
		if (us1_rx_st == US_RX_ERROR) {
			for (j = 0; j < 3; j++)
				msg_bf_tx[j] = '!';
			msg_bf_tx[j] = ':';
			msg_bf_tx[j + 1] = '1';
			pilbus_send(1, 0x99, 'E', 5, msg_bf_tx);
			sleep_us(5000);
			pilbus_rxreinit(1);
		}
		// Error reporting on PilsenCUBE bus ch. 2
		if (us2_rx_st == US_RX_ERROR) {
			for (j = 0; j < 3; j++)
				msg_bf_tx[j] = '!';
			msg_bf_tx[j] = ':';
			msg_bf_tx[j + 1] = '2';
			pilbus_send(2, 0x99, 'E', 5, msg_bf_tx);
			sleep_us(5000);
			pilbus_rxreinit(2);
		}

		// Increment the main loop counter
		i++;
	}		
}
#endif

/*! @} */

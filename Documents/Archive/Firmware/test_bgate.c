/*!
 * \addtogroup TestBusGateway Test-Bus-Gateway
 * \brief PilsenCUBE Bus bidirectional gateway test working with XRAY subsystem
 * @{
 */

/*!
 * \file    test_bus.c
 * \brief   PilsenCUBE Bus bidirectional gateway test working with XRAY subsystem
 * \author  OK1CTR
 * \version 1.0
 * \date    02.2020
 */


#include <stdint.h>

#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_BUS_GATEWAY

//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[64];


/* Non returning test loop */
void test_procedure(void)
{
	uint8_t data[] = "HELLO!!!";
	uint32_t i;
	uint8_t j;


	// Initial message send to PilsenCUBE bus ch. 1
	pilbus_send(1, 0x99, 'b', 8, data);
	
	while (1) {

		// watchdog reset
		wdt_trig();

/*
		// one second timing
		sec_upd = 0;
		while (sec_upd == 0)
			;
*/

		// resend received data from PilsenCUBE bus ch.1 to PilsenCUBE bus ch.2
		if (us1_rx_st == US_RX_OK) {
			pilbus_send(2, 0x87, us1_rx_cmd, us1_nrx, pilbus1_rxbf);  // medipix addressed
			sleep_us(1000);
			pilbus_rxreinit(1);
		}

		// reply bytes + 1 with ch.l identification on PilsenCUBE bus ch.2
		if (us2_rx_st == US_RX_OK) {
			pilbus_send(1, 0x99, us2_rx_cmd, us2_nrx, pilbus2_rxbf);  // PC addressed
			sleep_us(1000);
			pilbus_rxreinit(2);
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

		// reporting error on PilsenCUBE bus ch.2 to ch.1
		if (us2_rx_st == US_RX_ERROR) {
			for (j = 0; j < 3; j++)
				msg_bf_tx[j] = '!';
			msg_bf_tx[j] = ':';
			msg_bf_tx[j + 1] = '2';
			pilbus_send(1, 0x99, 'E', 5, msg_bf_tx);
			sleep_us(5000);
			pilbus_rxreinit(2);
		}

		// increment the main loop counter
		i++;
	}		
}


#endif

/*! @} */

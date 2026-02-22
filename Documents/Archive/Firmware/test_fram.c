/*!
 * \addtogroup TestFram Test-FRAM
 * \brief PilsenCUBE FRAM storage test
 * @{
 */

/*!
 * \file    test_fram.c
 * \brief   PilsenCUBE FRAM storage test
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#include <stdint.h>

#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"
#include "pils_fram.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_FRAM

//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[20];


/* Non returning test loop */
void test_procedure(void)
{
	uint8_t data[] = "HELLO!!!";
	uint32_t i;

	// FRAM initialization
	msg_bf_tx[0] = fram_init(0x0F);
	// Block protection test
	msg_bf_tx[1] = fram_block_prot(1, 2);
	msg_bf_tx[2] = fram_block_prot(1, 0);
	// Data write test
	fram_write(1, 0, 8, data);
	
	while (1) {

		// Watchdog reset
		wdt_trig();

		// One second timing
		sec_upd = 0;
		while (sec_upd == 0)
			;

		// Periodic FRAM data block read
		fram_read(1, 0, 8, msg_bf_tx + 3);
		pilbus_send(1, 0x99, 'f', 11, msg_bf_tx);

		// Increment the main loop counter
		i++;
	}		
}
#endif

/*! @} */

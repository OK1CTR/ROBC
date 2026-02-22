/*!
 * \addtogroup TestExtAdc Test-AdcExt
 * \brief External (I2C) ADC measurement test
 * @{
 */

/*!
 * \file    test_i2cadc.c
 * \brief   External (I2C) ADC measurement test
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "i2c.h"
#include "pils_bus.h"
#include "tests.h"
#include "pils_adc.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_I2C_ADC

//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[20];


/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i = 0;
	uint8_t n, j;
	uint16_t result[9];
	
	// I2C1 init (WARNING! I2C1+SPI1 remaped -> collision !!!
	i2c1_init();
	
	// I2C ADC init
	if (i2c_adc_init()) {
		msg_bf_tx[0] = MTID_FATAL_ERROR;
		msg_bf_tx[1] = 1; // Point 1
		pilbus_send(1, TST_ADR, TST_CHR, 2, msg_bf_tx);
		while (1)
			wdt_trig();
	}

	while (1) {

		// watchdog reset
		wdt_trig();

		// one second timing
		sec_upd = 0;
		while (sec_upd == 0)
			;
		
		// start the measurement
		n = i % 9; // select the channel
		if (i2c_adc_meas(result + n, n)) {
			msg_bf_tx[0] = MTID_FATAL_ERROR;
			msg_bf_tx[1] = 2; // Point 2
			pilbus_send(1, TST_ADR, TST_CHR, 2, msg_bf_tx);
			while (1)
				wdt_trig();
		}

		// send the message with results
		if (n == 8) {
			msg_bf_tx[0] = MTID_ADC_RES1;
			for (j = 0; j < 9; j++) {
				msg_bf_tx[(j << 1) + 1] = result[j] >> 8;
				msg_bf_tx[(j << 1) + 2] = result[j] & 0xFF;
			}
			pilbus_send(1, TST_ADR, TST_CHR, 19, msg_bf_tx);
		}

		// increment the main loop counter
		i++;
	}		
}
#endif

/*! @} */

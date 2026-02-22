/*!
 * \addtogroup TestAdc Test-AdcInt
 * \brief Internal ADC measurement test
 * @{
 */

/*!
 * \file    test_adc.c
 * \brief   Internal ADC measurement test
 * \author  OK1CTR
 * \version 1.0
 * \date    09.2019
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"
#include "tests.h"
#include "pils_adc.h"


#if !defined(ROBC_VER_2) && !defined(ROBC_VER_3)
	#error Define the ROBC board version!
#endif


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_STM_ADC

//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[20];


/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i = 0;
	uint8_t n, j;
	uint16_t result[8];

	// ADC init
	stm_adc_init();

	while (1) {

		// watchdog reset
		wdt_trig();

		// one second timing
		sec_upd = 0;
		while (sec_upd == 0)
			;

#ifdef ROBC_VER_2
		// start the measurement
		n = i % 8; // select the channel
		stm_adc_meas(result + n, n);

		// send the message with results
		if (n == 7) {
			msg_bf_tx[0] = MTID_ADC_RES2;
			for (j = 0; j < 8; j++) {
				msg_bf_tx[(j << 1) + 1] = result[j] >> 8;
				msg_bf_tx[(j << 1) + 2] = result[j] & 0xFF;
			}
			pilbus_send(1, TST_ADR, TST_CHR, 17, msg_bf_tx);
		}
#endif

#ifdef ROBC_VER_3		
		// start the measurement
		n = i % 7; // select the channel
		stm_adc_meas(result + n, n);

		// send the message with results
		if (n == 6) {
			msg_bf_tx[0] = MTID_ADC_RES3;
			for (j = 0; j < 7; j++) {
				msg_bf_tx[(j << 1) + 1] = result[j] >> 8;
				msg_bf_tx[(j << 1) + 2] = result[j] & 0xFF;
			}
			pilbus_send(1, TST_ADR, TST_CHR, 15, msg_bf_tx);
		}
#endif

		// increment the main loop counter
		i++;
	}		
}
#endif

/*! @} */

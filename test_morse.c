/*!
 * \addtogroup TestMorse Test-Morse
 * \brief ASK Morse beacon transmission test
 * @{
 */

/*!
 * \file    test_morse.c
 * \brief   ASK Morse beacon transmission test
 * \author  OK1CTR
 * \version 1.0
 * \date    09.2019
 */


#include <stdint.h>

#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "ax5043.h"
#include "morse.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_MORSE

//! Message transmitter periodically as MORSE test
#define TEST_MSG "PILSENCUBE SATELLITE 2019"
//! Transmission period in seconds (Consider the message length!)
#define TEST_PER 30


/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i; // Main loop counter in seconds

	ax_init();
	ax_pwrmode(AX_PWRMODE_TXS);
	ax_frequency(0, FREQUENCY, 1);
	ax_mode_askw();
	morse_init(MORSE_RATE);
	pa_low = 1;  // PA output power limit
	ax_pwrmode(AX_PWRMODE_TX);
	pa_prot_ctrl(1);  // PA protecion 1=ON, 0=OFF
	
	while (1) {

		// Watchdog reset
		wdt_trig();

		// One second timing
		sec_upd = 0;
		while (sec_upd == 0)
			idle(); // Needed for Morse module run!

		if (i >= TEST_PER && morse_buf_free()) {
			morse_send((unsigned char*) TEST_MSG);
			i = 0;
		}

		// Increment the main loop counter
		i++;
	}		
}
#endif

/*! @} */

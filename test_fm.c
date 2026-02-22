/*!
 * \addtogroup TestFMTx Test-FM-TX
 * \brief Analog FM beacon transmission test
 * @{
 */

/*!
 * \file    test_fm.c
 * \brief   Analog FM beacon transmission test
 * \author  OK1CTR
 * \version 1.0
 * \date    09.2019
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "ax5043.h"
#include "morse.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_FM

/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i;

	ax_load_par(); // default radio parameter set activation
	ax_init();
	ax_pwrmode(AX_PWRMODE_TXS);
	ax_frequency(0, FREQUENCY, 1);
	pa_low = 1;
	pa_prot_ctrl(1);
	ax_mode_fm();
	
/* AMP */
	ax_rw_2(1, 0x25, 0x01);
/* *** */
	
	while (1) {

		// Watchdog reset
		wdt_trig();

		// One second timing
		sec_upd = 0;
		while (sec_upd == 0)
			;

		// Increment the main loop counter
		i++;
	}		
}
#endif

/*! @} */

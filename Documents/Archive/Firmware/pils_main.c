/*!
 * \addtogroup Main Main
 * \brief PilsenCUBE COM-OBC main module
 * @{
 */

/*!
 * \file    pils_main.c
 * \brief   PilsenCUBE COM-OBC main module source
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */


#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"
#include "pils_fram.h"
#include "pils_bkp.h"
#include "i2c.h"
#include "ax5043.h"
#include "pils_adc.h"
#include "morse.h"
#include "ax25.h"


//! I2C-1 data buffer
uint8_t i2c_bf1[I2C1_BF_SIZE];
//! Message buffer for PilsenCUBE bus transmission
uint8_t msg_bf_tx[32];


/*! \brief Main function
 */
int main(void)
{

	volatile uint32_t i = 0;

	ioports_init();
	timer_init();
	pilbus_init();
//i2c1_init(); // I2C1 + SPI1 remaped -> collision!!! (switch the clock OFF)
	i2c1_buffer = i2c_bf1;

#if defined(TEST_PROCEDURE)
	test_procedure();
#endif
	
	while (1) {

		// Watchdog reset
		wdt_trig();

		// One second timing
		sec_upd = 0;
		while (sec_upd == 0)
			idle();

		// increment the main loop counter
		i++;
	}
}


/* Idle function */
void idle(void)
{
	morse_loop();
	return;
}

/*! @} */

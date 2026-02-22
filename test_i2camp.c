/*!
 * \addtogroup TestI2CGateway Test-I2C-Gateway
 * \brief PilsenCUBE bus I2C gateway test used for AMP Testing purposes 
 * @{
 */

/*!
 * \file    test_i2camp.c
 * \brief   PilsenCUBE bus I2C gateway test used for AMP Testing purposes
 * \author  OK1CTR
 * \version 1.0
 * \date    04.2020
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "i2c.h"
#include "pils_bus.h"
#include "tests.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_AMP_I2C_MS


//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[20];


/* Non returning test loop. */
void test_procedure(void)
{
	uint8_t n, j;
	
	// I2C1 init (WARNING! I2C1+SPI1 remaped -> collision !!!
	i2c1_init();

	while (1) {

		// watchdog reset
		wdt_trig();

		// resend received data from PilsenCUBE bus ch.1 to Radio
		if (us1_rx_st == US_RX_OK) {
			if (us1_rx_cmd == 'A') {
				// character no. 0 is direction - read (nonzero) or write (zero)
				if (*pilbus1_rxbf == 0) {  // write register
					// number of characters written must be lower or the same as the I2C buffer length
					n = (us1_nrx > I2C1_BF_SIZE + 3) ? I2C1_BF_SIZE : us1_nrx - 3;
					for (j = 0; j < n; j++)
						i2c_bf1[j] = pilbus1_rxbf[j + 3];
					// character no. 1 and 2 are I2C slave address and register address
					i2c1_write_buf(pilbus1_rxbf[1], pilbus1_rxbf[2], n);
					i2c1_wait(); sleep_us(1000);
					// character no. 0 is I2C final result or error code
					*msg_bf_tx = i2c1_error; i2c1_error = 0;
					pilbus_send(1, 0x99, 'A', 1, msg_bf_tx);
				} else {  // read register
					// number of characters read must be lower or the same as the I2C buffer length
					n = (pilbus1_rxbf[3] > I2C1_BF_SIZE) ? I2C1_BF_SIZE : pilbus1_rxbf[3];
					// character no. 1 and 2 are I2C slave address and register address
					i2c1_read_buf(pilbus1_rxbf[1], pilbus1_rxbf[2], n);
					i2c1_wait(); sleep_us(1000);
					// character no. 0 is I2C final result or error code
					*msg_bf_tx = i2c1_error; i2c1_error = 0;
					// then bytes read are sent
					for (j = 0; j < n; j++)
						msg_bf_tx[j + 1] = i2c_bf1[j];
					pilbus_send(1, 0x99, 'A', n + 1, msg_bf_tx);
				}
			}
			pilbus_rxreinit(1);
		}
	}		
}


#endif


/*! @} */

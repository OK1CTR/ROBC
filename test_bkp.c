/*!
 * \addtogroup TestBkp Test-Backup
 * \brief PilsenCUBE Backup Domain test
 * @{
 */

/*!
 * \file    test_bkp.c
 * \brief   PilsenCUBE Backup Domain test
 * \author  OK1CTR
 * \version 1.0
 * \date    02.2020
 */


#include <stdint.h>

#include "stm32f10x.h"
#include "pils_config.h"
#include "pils_main.h"
#include "robc_hal.h"
#include "pils_bus.h"
#include "pils_bkp.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_BACKUP_DOM

//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[20];


/* Non returning test loop */
void test_procedure(void)
{
	uint8_t r, j;
	uint8_t data[4] = "RTC-";
	uint32_t a;

	r = bkp_init();
	// Initial message send with the information about backup domain reset
	data[3] = (r) ? 'R' : 'O';  // send R in case of RESET or O if OK
	pilbus_send(1, 0x99, 't', 4, data);
	sleep_us(5000);
	
	while (1) {

		// watchdog reset
		wdt_trig();
		
		// alarm flag clear and report
		if (rtc_alarm_flag()) {
			rtc_alarm_clear();
			*msg_bf_tx = 'A';
			r = 1;
			pilbus_send(1, 0x99, 't', r, msg_bf_tx);
			sleep_us(5000);
		}

		// reply bytes + 1 with channel identification on PilsenCUBE bus ch. 1
		if (us1_rx_st == US_RX_OK) {
			switch (us1_rx_cmd) {
				case 't': // get RTC time
					a = rtc_get_time();
					*msg_bf_tx = a >> 24;
					*(msg_bf_tx + 1) = (a >> 16) & 0xFF;
					*(msg_bf_tx + 2) = (a >> 8) & 0xFF;
					*(msg_bf_tx + 3) = a & 0xFF;
					r = 4;
					break;

				case 'T': // set RTC and software time
					if (us1_nrx < 4) { *msg_bf_tx = '!'; break;	}
					a = *pilbus1_rxbf; a <<= 8;
					a |= *(pilbus1_rxbf + 1); a <<= 8;
					a |= *(pilbus1_rxbf + 2); a <<= 8;
					a |= *(pilbus1_rxbf + 3);
					rtc_set_time(a);
					sw_rtc_sync(); // sync SW time
					*msg_bf_tx = 'O';
					r = 1;
					break;

				case 's': // get software time
					a = sw_rtc;
					*msg_bf_tx = a >> 24;
					*(msg_bf_tx + 1) = (a >> 16) & 0xFF;
					*(msg_bf_tx + 2) = (a >> 8) & 0xFF;
					*(msg_bf_tx + 3) = a & 0xFF;
					r = 4;
					break;

				case 'A': // set alarm
					if (us1_nrx < 4) { *msg_bf_tx = '!'; break;	}
					a = *pilbus1_rxbf; a <<= 8;
					a |= *(pilbus1_rxbf + 1); a <<= 8;
					a |= *(pilbus1_rxbf + 2); a <<= 8;
					a |= *(pilbus1_rxbf + 3);
					rtc_alarm_clear();
					rtc_set_alarm(a);
					*msg_bf_tx = 'O';
					r = 1;
					break;

				case 'r': // get the backup register
					if (us1_nrx < 1) r = 0; else r = *pilbus1_rxbf;
					a = bkp_get_val(r);
					*(msg_bf_tx) = (a >> 8) & 0xFF;
					*(msg_bf_tx + 1) = a & 0xFF;					
					r = 2;
					break;

				case 'R': // set the backup register
					if (us1_nrx < 3) { *msg_bf_tx = '!'; break;	}
					r = *pilbus1_rxbf;
					a = *(pilbus1_rxbf + 1); a <<= 8;
					a |= *(pilbus1_rxbf + 2);
					bkp_set_val(r, a);
					*msg_bf_tx = 'O';
					r = 1;
					break;

				default:  // wrong cmd
					*msg_bf_tx = '?';
					r = 1;
			}
			
			pilbus_send(1, 0x99, 't', r, msg_bf_tx);
			sleep_us(5000);
			pilbus_rxreinit(1);
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
	}		
}
#endif

/*! @} */

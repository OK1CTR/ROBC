/*!
 * \addtogroup TestFMRx Test-FM-RX
 * \brief Analog FM reception test
 * @{
 */

/*!
 * \file    test_fmrx.c
 * \brief   Analog FM reception test
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


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_FM_RX

/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i;

	ax_init();
	ax_pwrmode(AX_PWRMODE_TXS);
	ax_frequency(0, FREQUENCY, 1);
	pa_low = 1;
	pa_prot_ctrl(1);

	// Performance tuning registers
	/*
	ax_rw_3(1, 0xF00, 0x0F);
	ax_rw_3(1, 0xF0C, 0x00);
	ax_rw_3(1, 0xF0D, 0x03);
	ax_rw_3(1, 0xF10, 0x04);
	ax_rw_3(1, 0xF11, 0x00);
	ax_rw_3(1, 0xF1C, 0x07);
	ax_rw_3(1, 0xF21, 0x5C);
	ax_rw_3(1, 0xF22, 0x53);
	ax_rw_3(1, 0xF23, 0x76);
	ax_rw_3(1, 0xF26, 0x92);
	ax_rw_3(1, 0xF34, 0x28);
	ax_rw_3(1, 0xF35, 0x10); // TR 0xF35, fxtal < 24.8 MHz, ADCCLKMUX = 0 (0xF35[1:0] = 0)
	ax_rw_3(1, 0xF44, 0x24);
	ax_rw_3(1, 0xF72, 0x00);
	*/
	ax_rw_2(1, 0x10, 0x0B); // FM mode

	ax_rw_3(1, 0x102, 0x01); // o DECIMATION, 13 was default (too much)

	ax_rw_3(1, 0x103, 0x00); // o DATA RATE = sample rate for analog FM
	ax_rw_3(1, 0x104, 0x51); // o
	ax_rw_3(1, 0x105, 0xD7); // o
	
	ax_rw_3(1, 0x100, 0x06); // ? 25 kHz fif, MSB
	ax_rw_3(1, 0x101, 0x4C); // ? 25 kHz fif, LSB

	ax_rw_3(1, 0x117, 0x00); // receiver parameter set 0 only
	ax_rw_3(1, 0x124, 0x00); // disable bit timing recovery
	ax_rw_3(1, 0x125, 0x00); // DRGAIN0 off
	ax_rw_3(1, 0x106, 0x00); // MAXDROFFSET, MSB
	ax_rw_3(1, 0x107, 0x00); // MAXDROFFSET
	ax_rw_3(1, 0x108, 0x00); // MAXDROFFSET, LSB
	ax_rw_3(1, 0x109, 0x80); // MAXRFOFFSET max. 10 kHz, MSB | 0x80 - track at LO1
	ax_rw_3(1, 0x10A, 0x28); // MAXRFOFFSET
	ax_rw_3(1, 0x10B, 0x0A); // MAXRFOFFSET, LSB

	ax_rw_3(1, 0x127, 0x0F); // FREQGAINA0, off
	ax_rw_3(1, 0x128, 0x02); // FREQGAINB0, baseband AFC loop gain, freq detector, 0x02 ~75 kHz dev

	ax_rw_3(1, 0x129, 0x1F); // FREQGAINC0, off
	ax_rw_3(1, 0x12A, 0x08); // FREQGAIND0, RF AFC loop

	ax_rw_3(1, 0x116, 0x04); // baseband AFC loop leakage

	ax_rw_3(1, 0x332, 0x03);   // output TRKFREQUENCY
	//ax_rw_3(1, 0x332, 0x07); // output RSSI, A bullshit in datasheet!
	ax_rw_3(1, 0x330, 0x00);   // DACVALUE = 0
	ax_rw_3(1, 0x331, 0x0C);   // DACSHIFT = 12 bit
	ax_rw_2(1, 0x25, 0x05);    // ANTSEL as output
	//ax_rw_2(1, 0x26, 0x05);  // PWRAMP as output
	
	ax_rw_2(1, 0x02, 0x09); // Full RX
	
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

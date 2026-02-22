/*!
 * \addtogroup TestHDLCFECRx Test-HDLC-FEC-RX
 * \brief PilsenCUBE HDLC-FEC data packet reception test
 * @{
 */

/*!
 * \file    test_hdlcrx.c
 * \brief   PilsenCUBE HDLC-FEC data packet reception test
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
#include "i2c.h"
#include "pils_bus.h"
#include "tests.h"


#if defined(TEST_PROCEDURE) && TEST_PROCEDURE == TEST_RAD_FEC_RX


//! Message buffer for PilsenCUBE bus transmission
extern uint8_t msg_bf_tx[32];


/* Non returning test loop. */
void test_procedure(void)
{
	uint32_t i;
	uint8_t x = 0, a, b;

	i2c1_init(); // !!!!
	ax_init();
	ax_pwrmode(AX_PWRMODE_TXS);
	ax_frequency(0, FREQUENCY - 0L, 1);
	pa_low = 1;
	pa_prot_ctrl(1);

	// receiver mode config
	ax_rw_2(1, 0x11, 0x00); // encoding must be OFF

	// framing
	ax_rw_2(1, 0x12, 0x64); // HDLC + CRC32
	ax_crc_init();  // initialize the HW CRC generator

	// FEC
	ax_rw_2(1, 0x18, 0x15);

	// packet format
	ax_rw_3(1, 0x200, 0x01); // data lsb first
	ax_rw_3(1, 0x201, 0x80); // length config
	ax_rw_3(1, 0x202, 0x01); // packet length offset!
	ax_rw_3(1, 0x203, 0xF0); // max length

	// packet controller
	ax_rw_3(1, 0x220, 0x33);
	ax_rw_3(1, 0x221, 0x14);
	ax_rw_3(1, 0x223, 0x33);
	ax_rw_3(1, 0x224, 0x14);
	ax_rw_3(1, 0x225, 0x00);
	ax_rw_3(1, 0x226, 0x73);
	ax_rw_3(1, 0x227, 0x00);
	ax_rw_3(1, 0x228, 0x03);
	ax_rw_3(1, 0x229, 0x00); // 00 pream 1 timeout
	ax_rw_3(1, 0x22A, 0x17); // 17 pream 2 timeout
	ax_rw_3(1, 0x22B, 0x00); // 00 pream 2 timeout
	ax_rw_3(1, 0x22C, 0xF8);
	ax_rw_3(1, 0x22F, 0x00);
	ax_rw_3(1, 0x230, 13);   // Max chunk size 13 = 240 B !!!
	ax_rw_3(1, 0x231, 0x00);
	ax_rw_3(1, 0x232, 0x54); // store flags ANT RSSI, RSSI, RF FOFFS

//ax_rw_3(1, 0x233, 0x3F);  // accept as much you can
	ax_rw_3(1, 0x233, 0x00);  // accept nothing special
//ax_rw_3(1, 0x233, 0x04);  // accept wrong crc
//ax_rw_3(1, 0x233, 0x1C);  // accept wrong crc and address and size
//ax_rw_3(1, 0x233, 0x20);  // accept lrgp
//ax_rw_3(1, 0x233, 0x1F);	// accept all except long puckets
//ax_rw_3(1, 0x233, 0x08);  // experimental
	
	// receiver parameters
	ax_rw_3(1, 0x102, 0x0E); // DECIMATION - bitrate dependent
	ax_rw_3(1, 0x103, 0x00); // DATA RATE, MSB - bitrate independent
	ax_rw_3(1, 0x104, 0x3C); // DATA RATE
	ax_rw_3(1, 0x105, 0xE4); // DATA RATE, LSB
	ax_rw_3(1, 0x100, 0x03); // 25 kHz Fif, MSB - bitrate dependent
	ax_rw_3(1, 0x101, 0x01); // 25 kHz Fif, LSB
	ax_rw_3(1, 0x106, 0x00); // MAXDROFFSET, MSB - don't use if RX/TX timing error is less than 0.15%
	ax_rw_3(1, 0x107, 0x00); // MAXDROFFSET
	ax_rw_3(1, 0x108, 0x00); // MAXDROFFSET, LSB
	ax_rw_3(1, 0x109, 0x80); // MAXRFOFFSET | 0x80 - track at LO1
	ax_rw_3(1, 0x10A, 0x04); // 04 (~+/-1 kHz) MAXRFOFFSET
	ax_rw_3(1, 0x10B, 0x90); // 90 (~+/-1 kHz) MAXRFOFFSET, LSB (!! 0x00FF works surprisingly good, 2% but still V)
	ax_rw_3(1, 0x10C, 0x00); // 00 FSKDMAX1
	ax_rw_3(1, 0x10D, 0xA6); // A6 FSKDMAX0 - bitrate independent
	ax_rw_3(1, 0x10E, 0xFF); // FF FSKDMIN1
	ax_rw_3(1, 0x10F, 0x5A); // 5A FSKDMIN0 - bitrate independent
	ax_rw_3(1, 0x116, 0x00); // baseband AFC loop leakage, default 0

	// receiver parameter set config
	ax_rw_3(1, 0x117, 0xF4); // Sets 0, 1 and 3
	//ax_rw_3(1, 0x117, 0x00); // Sets 0 only

	// receiver paramater set 0
	ax_rw_3(1, 0x120, 0xB5); // AGCGAIN0 - bitrate dependent
	ax_rw_3(1, 0x121, 0x84); // AGCTARGET0 - bitrate independent
	ax_rw_3(1, 0x124, 0xF8); // TIMEGAIN0, default 0xF8
	ax_rw_3(1, 0x125, 0xF2); // DRGAIN0, default 0xF2
	ax_rw_3(1, 0x126, 0xC3); // PHASEGAIN0, default 0xC3
	ax_rw_3(1, 0x127, 0x0F); // FREQGAINA0, off
	ax_rw_3(1, 0x128, 0x1F); // FREQGAINB0, off
	ax_rw_3(1, 0x129, 0x09); // 09 FREQGAINC0 - bitrate dependent
	ax_rw_3(1, 0x12A, 0x09); // 09 FREQGAIND0 - bitrate dependent
	ax_rw_3(1, 0x12B, 0x06); // AMPLITUDEGAIN0, default 0x46
	ax_rw_3(1, 0x12C, 0x00); // FREQDEV10, default 0
	ax_rw_3(1, 0x12D, 0x00); // FREQDEV00, default 0
	ax_rw_3(1, 0x12E, 0x16); // FOURFSK0, default 16
	ax_rw_3(1, 0x12F, 0x00); // BBOFSRES0, default 0x88

	// receiver paramater set 1
	ax_rw_3(1, 0x130, 0xB5); // AGCGAIN1 - bitrate dependent
	ax_rw_3(1, 0x131, 0x84); // AGCTARGET1 - bitrate independent
	ax_rw_3(1, 0x134, 0xF6); // TIMEGAIN1, default 0xF8
	ax_rw_3(1, 0x135, 0xF1); // DRGAIN1, default 0xF2
	ax_rw_3(1, 0x136, 0xC3); // PHASEGAIN1, default 0xC3
	ax_rw_3(1, 0x137, 0x0F); // FREQGAINA1, off
	ax_rw_3(1, 0x138, 0x1F); // FREQGAINB1, off
	ax_rw_3(1, 0x139, 0x0E); // 09 FREQGAINC1 - bitrate dependent
	ax_rw_3(1, 0x13A, 0x0E); // 09 FREQGAIND1 - bitrate dependent
	ax_rw_3(1, 0x13B, 0x06); // AMPLITUDEGAIN1, default 0x46
	ax_rw_3(1, 0x13C, 0x00); // FREQDEV11, default 0
	ax_rw_3(1, 0x13D, 0x32); // FREQDEV01, default 0
	ax_rw_3(1, 0x13E, 0x16); // FOURFSK1, default 16
	ax_rw_3(1, 0x13F, 0x00); // BBOFSRES1, default 0x88

	// receiver paramater set 3
	ax_rw_3(1, 0x150, 0xFF); // AGCGAIN3 - bitrate independent
	ax_rw_3(1, 0x151, 0x84); // AGCTARGET3 - bitrate independent
	ax_rw_3(1, 0x154, 0xF5); // TIMEGAIN3, default 0xF8
	ax_rw_3(1, 0x155, 0xF0); // DRGAIN3, default 0xF2
	ax_rw_3(1, 0x156, 0xC3); // PHASEGAIN3, default 0xC3
	ax_rw_3(1, 0x157, 0x0F); // FREQGAINA3, off
	ax_rw_3(1, 0x158, 0x1F); // FREQGAINB3, off
	ax_rw_3(1, 0x159, 0x0D); // 0D FREQGAINC3 - bitrate dependent
	ax_rw_3(1, 0x15A, 0x0D); // 0D FREQGAIND3 - bitrate dependent
	ax_rw_3(1, 0x15B, 0x06); // AMPLITUDEGAIN3, default 0x46
	ax_rw_3(1, 0x15C, 0x00); // FREQDEV13, default 0
	ax_rw_3(1, 0x15D, 0x32); // FREQDEV03, default 0
	ax_rw_3(1, 0x15E, 0x16); // FOURFSK3, default 16
	ax_rw_3(1, 0x15F, 0x00); // BBOFSRES3, default 0x88

	// pattern match
	ax_rw_3(1, 0x210, 0xAA); // AA MATCH0PAT3
	ax_rw_3(1, 0x211, 0xCC); // CC MATCH0PAT2
	ax_rw_3(1, 0x212, 0xAA); // AA MATCH0PAT1
	ax_rw_3(1, 0x213, 0xCC); // CC MATCH0PAT0
	ax_rw_3(1, 0x214, 0x00); // MATCH0LEN
	ax_rw_3(1, 0x215, 0x00); // MATCH0MIN
	ax_rw_3(1, 0x216, 0x1F); // MATCH0MAX
	ax_rw_3(1, 0x218, 0x7E); // MATCH1PAT1 - bit reversed preamble!
	ax_rw_3(1, 0x219, 0x7E); // MATCH1PAT0 - bit reversed preamble!
	ax_rw_3(1, 0x21C, 0x8A); // MATCH1LEN !!! 0x0A without FEC, 0x8A with FEC !!!
	ax_rw_3(1, 0x21D, 0x00); // MATCH1MIN
	ax_rw_3(1, 0x21E, 0x0A); // MATCH1MAX

	// analog debug output
	//ax_rw_3(1, 0x332, 0x03); // output TRKFREQUENCY
	ax_rw_3(1, 0x332, 0x07);   // output RSSI, A bullshit in datasheet!
	ax_rw_3(1, 0x330, 0x00);   // DACVALUE = 0
	ax_rw_3(1, 0x331, 0x0C);   // DACSHIFT = 12 bit
	//ax_rw_3(1, 0x331, 0x08);   // DACSHIFT = 8 bit, TRKFRQ.. zoom
	ax_rw_2(1, 0x25, 0x05);    // ANTSEL as output
	//ax_rw_2(1, 0x26, 0x05);  // PWRAMP as output

	// digital debug output
	ax_rw_2(1, 0x22, 0x04); // DCLK -> modem clock output, more variants!
	ax_rw_2(1, 0x23, 0x07); // DATA -> modem data output a LOT of variants! (0x07 - raw data, NRZI decoded, no descrabled)

	// irq
	ax_rw_2(1, 0x24, 0x03); // PINFUNCIRQ - IRQ output
//ax_rw_2(1, 0x07, 0x40); // Radio controller event
	ax_rw_2(1, 0x07, 0x01); // FIFO not empty
	ax_rw_2(1, 0x09, 0x04); // RADIOEVENTMASK

	ax_rw_2(1, 0x02, 0x09); // Full RX
	
	while (1) {

		// watchdog reset
		wdt_trig();

		// check the radio IRQ output
		if (GPIOB->IDR & 0x0200) {
			/* radioevent debug message
			msg_bf_tx[6] = ax_rw_2(0, 0x1C, 0x00); // radio state
			msg_bf_tx[0] = ax_rw_2(0, 0x0C, 0x00); // int rq 1
			msg_bf_tx[1] = ax_rw_2(0, 0x0D, 0x00); // int rq 0
			msg_bf_tx[2] = ax_rw_2(0, 0x0E, 0x00); // radio event rq 1
			msg_bf_tx[3] = ax_rw_2(0, 0x0F, 0x00); // radio event rq 0
			msg_bf_tx[4] = ax_rw_2(0, 0x2B, 0x00); // fifo count L
			msg_bf_tx[5] = ax_rw_3(0, 0x118, 0x00); // current parameter set
			pilbus_send(1, 'X', 'Y', 7, msg_bf_tx);
			x++;
			*/
			// read the radio event request
			//ax_rw_2(0, 0x0E, 0x00);
			//ax_rw_2(0, 0x0F, 0x00);

			// incomming data packet processing
			while ((a = ax_rw_2(0, 0x2B, 0x00)) > 0) { // fifo count L
				if (a > 32) {
					a = 32;
					b = TST_RXD;
				} else b = TST_RXDF; // fianl part of incomming packet
				ax_rw_N(0, 0x29, msg_bf_tx, a);
				while (us1_tx_str != US_TXSR_IDLE)
					;
				pilbus_send(1, TST_ADR, b, a, msg_bf_tx);
			}
			x++;
		}
		
		// increment the main loop counter
		i++;
		led_disp(x);
	}		
}
#endif

/*! @} */

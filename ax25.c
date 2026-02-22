/*!
 * \addtogroup Ax25 AX.25
 * \brief Ham radio AX.25 telemetry message transmitter
 * @{
 */
 
/*!
 * \file    ax25.c
 * \brief   Ham radio AX.25 telemetry message transmitter, source
 * \author  OK1CTR
 * \version 1.0
 * \date    12.09.2018
 */
 

#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "stm32f10x.h"
#include "ax5043.h"
#include "ax25.h"
 
 
//! Almost constant header of the AX.25 telemetry frames
uint8_t ax25_hdr[16];
//! Header of the HDLC ground contact frames
uint8_t hdlc_hdr[16];


/* Init the AX.25 telemetry transmitter */
void ax25_init(void)
{
	// header - add calsign and ssid
	ax25_enc_call((unsigned char *) AX25_CALL_DEST, AX25_SSID_DEST, ax25_hdr, 0);
	ax25_enc_call((unsigned char *) AX25_CALL_SRC, AX25_SSID_SRC, ax25_hdr + 7, 1);
	// header - add control and PID fields
	ax25_hdr[14] = AX25_CMD;
	ax25_hdr[15] = AX25_PID;
	// *** PRELIMINARY ground contact header
	memcpy(hdlc_hdr, ax25_hdr, 16);
	return;
}


/* Convert a callsign in ASCII to AX.25 format */
void ax25_enc_call(uint8_t *call, uint8_t ssid, uint8_t *buf, uint8_t last)
{
	uint8_t i, l;

	l = strlen((char*) call);	memset(buf, ' ' << 1, 6);
	if (l > 6) l = 6; // limit the 6-char callsign
	for (i = 0; i < l; i++)	buf[i] = toupper(call[i]) << 1;
	buf[6] = 0x60 | ((ssid & 0x0F) << 1); // limit the 0 - 15 SSID
	if (last) buf[6] |= 0x01;

	return;
}


/* CRC-CCITT calculation for AX.25 Frame Check Sequence (FCS) */
uint16_t ax25_crc_calc(uint8_t c, uint16_t f)
{
	uint8_t bit;

	for (bit = 8; bit; bit--) {
		if ((f ^ c) & 1) f = (f >> 1) ^ 0x8408;
			else f = f >> 1;
		c = c >> 1;
	}

	return(f);
}


/*! \brief Send the TXCTL command into the AX5043 FIFO
 *  \param param The parameter of the TXCTL command
 *  \note Add FIFO timeout !!!
 */
void ax25_send_txctl(uint8_t param)
{
	static uint8_t ch_txctl[] = {
		0x3C, // TXCTL chunk
		AX_TXCTL_PAOFF, // parameter
	};
	
	ch_txctl[1] = param;
	while (!(ax_status & AX_ST_FIFOTHRFREE)) // read FIFO_FREE_0
		ax_rw_2(0, 0x2D, 0);
	ax_rw_N(1, 0x29, ch_txctl, 2);
	ax_rw_2(1, 0x28, FIFOCMD_COMMIT);	

	return;
}


/* Send HDLC/AX.25/FEC universal flag chunk into the AX5043 FIFO */
void hdlc_send_flag(uint8_t pattern, uint8_t length, uint8_t parameter)
{
	static uint8_t ch_pream[] = {
		0x62, // preamble chunk
		0x18, // parameter (0x18 for AX.25 or 0x38 for HDLC)
		0x10, // (default) length
		0x00  // pattern character
	};

	ch_pream[1] = 0x08 | (parameter & 0x30);
	ch_pream[3] = pattern;
	if (length != 0) ch_pream[2] = length - 1;
	while (!(ax_status & AX_ST_FIFOTHRFREE)) // read FIFO_FREE_0
		ax_rw_2(0, 0x2D, 0);
	ax_rw_N(1, 0x29, ch_pream, 4);
	ax_rw_2(1, 0x28, FIFOCMD_COMMIT);	

	return;
}


/* Send AX.25 data chunk into the AX5043 FIFO */
void ax25_send_inf(uint8_t *data, uint8_t length, uint8_t pacstart)
{
	static uint8_t ch_data[] = {
		0xE1, // data chunk
		0x00, // (default) length
		0x00  // (default) parameter
	};

	ch_data[1] = length + 1;
	ch_data[2] = pacstart;
	while (!(ax_status & AX_ST_FIFOTHRFREE)) // read FIFO_FREE_0
		ax_rw_2(0, 0x2D, 0);
	ax_rw_N(1, 0x29, ch_data, 3);
//ax_rw_2(1, 0x28, FIFOCMD_COMMIT);	// hangs transmitter sometimes because early commit
//If commited now, the transmitter will process the 3 byte header quickly. The following data
//are not loaded yet, they are missing and the transmitter will hang.
	while (!(ax_status & AX_ST_FIFOTHRFREE)) // read FIFO_FREE_0
		ax_rw_2(0, 0x2D, 0);
	ax_rw_N(1, 0x29, data, length);
	ax_rw_2(1, 0x28, FIFOCMD_COMMIT);		

	return;
}


/* Send AX.25 data chunk with CRC into the AX5043 FIFO */
void ax25_send_crc(uint16_t crc)
{
	static uint8_t ch_crc[] = {
		0xE1, // data chunk
		0x03, // length
		0x02, // parameter
		0x00, // CRC-L
		0x00  // CRC-H
	};
	
	ch_crc[3] = crc & 0xFF;
	ch_crc[4] = crc >> 8;
	while (!(ax_status & AX_ST_FIFOTHRFREE)) // read FIFO_FREE_0
		ax_rw_2(0, 0x2D, 0);
	ax_rw_N(1, 0x29, ch_crc, 5);
	ax_rw_2(1, 0x28, FIFOCMD_COMMIT);	

	return;
}


/* Send AX.25 telemetry message */
void ax25_send_msg(uint8_t *msg, uint8_t length)
{
	uint8_t i, l, sg, *p;
	uint16_t crc = 0xFFFF;

	if (length == 0) l = strlen((char *) msg); else l = length;
	ax_rw_2(1, 0x28, FIFOCMD_CLR_DFL);  // clear FIFO and flags
	ax25_send_txctl(AX_TXCTL_PAON);  // PA on
	hdlc_send_flag(HDLC_FLAG, AX25_PRE_LEN, AX25_FLAG_PAR);  // send flag (peramble)
	ax25_send_inf(ax25_hdr, 16, 0);  // send AX.25 header
	// calculate header CRC
	for (i = 0, p = ax25_hdr; i < 16; i++, p++) crc = ax25_crc_calc(*p, crc);
	// 16-Byte segmented transmission of the data
	sg = l / 16;
	for (i = 0; i < sg; i++)
		ax25_send_inf(msg + (i << 4), 16, 0);
	sg = l % 16;
	if (sg) ax25_send_inf(msg + (i << 4), sg, 0);
	// calculate data CRC
	for (i = 0, p = msg; i < l; i++, p++) crc = ax25_crc_calc(*p, crc);
	crc ^= 0xFFFF;
	// send CRC and finish the transmission
	ax25_send_crc(crc);  // send CRC
	hdlc_send_flag(HDLC_FLAG, AX25_TAIL_LEN, AX25_FLAG_PAR);  // send flag
	ax25_send_txctl(AX_TXCTL_PAOFF);  // PA off

	return;
}


/* Send HDLC ground contact message */
void hdlc_send_msg(uint8_t *msg, uint8_t length, uint8_t position)
{
	uint8_t i, l, sg, sgr;

	// prepare message length
	if (length == 0) l = strlen((char *) msg); else l = length;
	// beginning of the transmission
	if (position & SER_POS_1) {
		ax_rw_2(1, 0x28, FIFOCMD_CLR_DFL);  // clear FIFO and flags
		ax_crc_init();  // initialize the HW CRC generator
		ax25_send_txctl(AX_TXCTL_PAON);  // PA on
		//hdlc_send_flag(0x00, HDLC_SCRP_LEN, 0x10);  // send AFC preamble
		hdlc_send_flag(HDLC_FLAG, HDLC_SCRP_LEN, AX25_FLAG_PAR);  // send flag (peramble)
	}
	// FIFO anti-freeze delay
	ax25_send_inf(hdlc_hdr, 16, AX_PACKET_START);  // send AX.25 header
	// 16-Byte segmented transmission of the data
	sg = l / 16; sgr = l % 16;
	for (i = 0; i < sg; i++)
		ax25_send_inf(msg + (i << 4), 16, (sgr == 0 && i == sg - 1) ? AX_PACKET_END : 0);
	if (sgr) ax25_send_inf(msg + (i << 4), sgr, AX_PACKET_END);
	hdlc_send_flag(HDLC_FLAG, AX25_TAIL_LEN, AX25_FLAG_PAR);  // send final flag
	// Don't use less than 3 tail flag characters, or the packet will fail when PA_OFF is sent after.
	if (position & SER_POS_N) {
		// finish the transmission
		ax25_send_txctl(AX_TXCTL_PAOFF);  // PA off
	}
	return;
}

/*! @} */

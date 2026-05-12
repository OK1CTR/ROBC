/**
 * @file       ax25.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      AX.25 and HDLC protocol module
 *
 * @addtogroup grAx25
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <ax25.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <ax5043.h>

/* Private defines -----------------------------------------------------------*/
/* Private typedefs ----------------------------------------------------------*/

/* AX.25 header */
typedef union
{
    uint8_t bytes[16];
    struct
    {
        uint8_t call_dst[7];  ///< destination callsign + SSID
        uint8_t call_src[7];  ///< source callsign + SSID
        uint8_t cmd;          ///< frame control field
        uint8_t pid;          ///< packet type identifier
    };
} ax25_hdr_t;

/* Private variables ---------------------------------------------------------*/
 
/*! Header for the AX.25 telemetry frames */
ax25_hdr_t hdr_ax25;
/*! Header for the HDLC ground contact frames */
ax25_hdr_t hdr_hdlc;

/* Private function prototypes -----------------------------------------------*/

/**
 * @nrief Convert a callsign in ASCII to AX.25 format
 * @param buf Buffer to store encoded callsign
 * @param call Callsing to be encoded
 * @param ssid Station SSID 0 .. 15
 * @param is_last True, if the callsign is last in the address field
 *
 */
static void ax25_encode_call(uint8_t *buf, char *call, uint8_t ssid, bool is_last);

/* Functions -----------------------------------------------------------------*/

/* Initialize the AX.25 and HDLC transmitter */
void ax25_init(void)
{
    // header - add calsign and SSID
    ax25_encode_call(hdr_ax25.call_dst, AX25_CALL_DEST, AX25_SSID_DEST, 0);
    ax25_encode_call(hdr_ax25.call_src, AX25_CALL_SRC, AX25_SSID_SRC, 1);
    // header - add control and PID fields
    hdr_ax25.cmd = AX25_CMD;
    hdr_ax25.pid = AX25_PID;

    // *** PRELIMINARY ground contact header
    hdr_hdlc = hdr_ax25;
}


/* CRC-CCITT calculation for AX.25 Frame Check Sequence (FCS) */
uint16_t ax25_crc_calc(uint8_t c, uint16_t f)
{
    uint8_t bit;

    for (bit = 8; bit; bit--)
    {
        if ((f ^ c) & 1)
        {
            f = (f >> 1) ^ 0x8408;
        }
        else
        {
            f = f >> 1;
        }
        c = c >> 1;
    }
    return(f);
}


/* Send HDLC/AX.25/FEC universal flag chunk into the AX5043 FIFO */
void hdlc_send_flag(uint8_t pattern, uint8_t length, uint8_t parameter)
{
    static uint8_t ch_pream[] =
    {
            0x62, // preamble chunk
            0x18, // parameter (0x18 for AX.25 or 0x38 for HDLC)
            0x10, // (default) length
            0x00  // pattern character
    };

    ch_pream[1] = 0x08 | (parameter & 0x30);
    ch_pream[3] = pattern;
    if (length != 0)
    {
        ch_pream[2] = length - 1;
    }
    ax_fifo_write(ch_pream, 4, true);
}


/* Send AX.25 data chunk into the AX5043 FIFO */
void ax25_send_inf(uint8_t *data, uint8_t length, uint8_t pacstart)
{
    static uint8_t ch_data[] =
    {
            0xE1, // data chunk
            0x00, // (default) length
            0x00  // (default) parameter
    };

    ch_data[1] = length + 1;
    ch_data[2] = pacstart;
    ax_fifo_write(ch_data, 3, false);
    // Do not commit here, it hangs transmitter sometimes because early commit.
    // If commited now, the transmitter will process the 3 byte header quickly. The following data
    // are not loaded yet, they are missing and the transmitter will hang.
    ax_fifo_write(data, length, true);
}


/* Send AX.25 data chunk with CRC into the AX5043 FIFO */
void ax25_send_crc(uint16_t crc)
{
    static uint8_t ch_crc[] =
    {
            0xE1, // data chunk
            0x03, // length
            0x02, // parameter
            0x00, // CRC-L
            0x00  // CRC-H
    };

    ch_crc[3] = crc & 0xFF;
    ch_crc[4] = crc >> 8;
    ax_fifo_write(ch_crc, 5, true);
}


/* Send AX.25 telemetry message */
void ax25_send_msg(uint8_t *msg, uint8_t length)
{
    uint8_t i, l, sg, *p;
    uint16_t crc = 0xFFFF;

    if (length == 0)
    {
        l = strlen((char *) msg);
    }
    else
    {
        l = length;
    }

    ax_fifo_cmd(ax_fifo_cmd_clr_data_flags);
    ax_fifo_txctrl(ax_txctrl_paon);
	hdlc_send_flag(HDLC_FLAG, AX25_PRE_LEN, AX25_FLAG_PAR);  // send flag (peramble)
	ax25_send_inf(hdr_ax25.bytes, sizeof(hdr_ax25), 0);  // send AX.25 header

	// calculate header CRC
	for (i = 0, p = hdr_ax25.bytes; i < sizeof(hdr_ax25); i++, p++)
	{
	    crc = ax25_crc_calc(*p, crc);
	}

	// 16-Byte segmented transmission of the data
	sg = l / 16;
	for (i = 0; i < sg; i++)
	{
		ax25_send_inf(msg + (i << 4), 16, 0);
	}
	sg = l % 16;
	if (sg)
	{
	    ax25_send_inf(msg + (i << 4), sg, 0);
	}

	// calculate data CRC
	for (i = 0, p = msg; i < l; i++, p++)
	{
	    crc = ax25_crc_calc(*p, crc);
	}
	crc ^= 0xFFFF;

	// send CRC and finish the transmission
	ax25_send_crc(crc);  // send CRC
	hdlc_send_flag(HDLC_FLAG, AX25_TAIL_LEN, AX25_FLAG_PAR);  // send flag
	ax_fifo_txctrl(ax_txctrl_paoff);
}


/* Send HDLC ground contact message */
void hdlc_send_msg(uint8_t *msg, uint8_t length, uint8_t position)
{
    uint8_t i, l, sg, sgr;
    ax_fifo_flags_t flag_pkt_start = {.pkt_start = 1};
    ax_fifo_flags_t flag_pkt_end = {.pkt_end = 1};

    // prepare message length
    if (length == 0)
    {
        l = strlen((char *) msg);
    }
    else
    {
        l = length;
    }

    // beginning of the transmission
    if (position & SER_POS_1)
    {
        ax_fifo_cmd(ax_fifo_cmd_clr_data_flags);
        ax_crc_init();  // initialize the HW CRC generator
        ax_fifo_txctrl(ax_txctrl_paon);
        //hdlc_send_flag(0x00, HDLC_SCRP_LEN, 0x10);  // send AFC preamble
        hdlc_send_flag(HDLC_FLAG, HDLC_SCRP_LEN, AX25_FLAG_PAR);  // send flag (peramble)
    }
    // FIFO anti-freeze delay
    ax25_send_inf(hdr_hdlc.bytes, sizeof(hdr_ax25), flag_pkt_start.value);  // send AX.25 header
    // 16-Byte segmented transmission of the data
    sg = l / 16;
    sgr = l % 16;
    for (i = 0; i < sg; i++)
    {
        ax25_send_inf(msg + (i << 4), 16, (sgr == 0 && i == sg - 1) ? flag_pkt_end.value : 0);
    }

    if (sgr)
    {
        ax25_send_inf(msg + (i << 4), sgr, flag_pkt_end.value);
    }
    hdlc_send_flag(HDLC_FLAG, AX25_TAIL_LEN, AX25_FLAG_PAR);  // send final flag

    // Don't use less than 3 tail flag characters, or the packet will fail when PA_OFF is sent after.
    if (position & SER_POS_N)
    {
        // finish the transmission
        ax_fifo_txctrl(ax_txctrl_paoff);
    }
}

/* Private functions ---------------------------------------------------------*/

/* Convert a callsign in ASCII to AX.25 format */
static void ax25_encode_call(uint8_t *buf, char *call, uint8_t ssid, bool is_last)
{
    uint32_t i, l = strlen(call);
    memset(buf, ' ' << 1, 6);

    l = (l > 6) ? 6 : l;  // limit the 6-char callsign

    for (i = 0; i < l; i++)
    {
        buf[i] = (uint8_t)(toupper(call[i]) << 1);
    }

    buf[6] = 0x60 | ((ssid & 0x0F) << 1);  // limit the 0 - 15 SSID

    if (is_last)
    {
        buf[6] |= 0x01;
    }
}

/* ---------------------------------------------------------------------------*/

/** @} */

/*!
 * \addtogroup AXRadio AX5043
 * \brief AX5043 radio driver with HW/SW SPI support
 * @{
 */
 
/*!
 * \file    ax5043.h
 * \brief   AX5043 radio driver with HW/SW SPI support, header
 * \author  OK1CTR
 * \version 1.0
 * \date    21.08.2018
 */


#ifndef _AX5043_H_
#define _AX5043_H_


//! VFO selection A - FREQA register will be used
#define AX_VFO_SEL_A 0
//! VFO selection B - FREQB register will be used
#define AX_VFO_SEL_B 1

//! G3RUH mode type for 1200 Bd
#define G3RUH_1200 1
//! G3RUH mode type for 2400 Bd
#define G3RUH_2400 2
//! G3RUH mode type for 4800 Bd
#define G3RUH_4800 3
//! G3RUH mode type for 9600 Bd
#define G3RUH_9600 4
//! G3RUH mode type for 19200 Bd
#define G3RUH_19200 5

//! GMSK Gaussian filter off
#define GAUSS_0 0
//! GMSK Gaussian filter BT = 0.3
#define GAUSS_3 2
//! GMSK Gaussian filter BT = 0.6
#define GAUSS_5 3  // better preamble sync

//! AX power (operation) mode - Full TX
#define AX_PWRMODE_TX 0xD
//! AX power (operation) mode - Full RX
#define AX_PWRMODE_RX 0x9
//! AX power (operation) mode - Synthesizer only TX
#define AX_PWRMODE_TXS 0xC
//! AX power (operation) mode - Synthesizer only RX
#define AX_PWRMODE_RXS 0x8

//! FIFO Command - clear overrun and underrun error flags.
#define FIFOCMD_CLR_ERR 0x02
//! FIFO Command - clear data and flags.
#define FIFOCMD_CLR_DFL 0x03
//! FIFO Command - commit
#define FIFOCMD_COMMIT 0x04

//! AN5043 status word flags
#define AX_ST_FIFOTHRFREE 0x0800

//! Safe thdreshold for chunk write to FIFO (The same maximal length of chunk must be kept.)
#define FIFO_FREE_THRESHOLD  20

//! AX5043 TXCTL command - PA ON
#define AX_TXCTL_PAON    0x03
//! AX5043 TXCTL command - PA OFF
#define AX_TXCTL_PAOFF   0x02

//! AX5043 FIFO packet start flag
#define AX_PACKET_START 0x01
//! AX5043 FIFO packet end flag
#define AX_PACKET_END   0x02

// AX5043 frame structure - CRC OFF
#define AX_CRC_OFF            0
// AX5043 frame structure - CRC-32
#define AX_CRC_CRC32          6
// CRC generator initialization word
#define AX_CRC_INIT  0xFFFFFFFF

// Encoding disabled
#define AX_ENC_DISABLED       0
// NRZI code without scrambling
#define AX_ENC_NRZI        0x03
// NRZI code with scrambling
#define AX_ENC_SCRAMBLER   0x07


/*! @name Radio configuration constants - Performance Tunning Registers
 *  @{
 */
#define RCON_PTRG_LEN 16 // parameter set length
#define RCON_F00 0x0F
#define RCON_F0C 0x00
#define RCON_F0D 0x03
#define RCON_F10 0x04
#define RCON_F11 0x00
#define RCON_F1C 0x07
#define RCON_F21 0x68 // 0x5C
#define RCON_F22 0xFF // 0x53
#define RCON_F23 0x84 // 0x76
#define RCON_F26 0x92
#define RCON_F34 0x28
#define RCON_F35 0x10 // TR 0xF35, fxtal < 24.8 MHz, ADCCLKMUX = 0 (0xF35[1:0] = 0)
#define RCON_F44 0x25 // 0x24
#define RCON_F72 0x00
#define RCON_188 0x0F
#define RCON_189 0x77
/*! @} */

/*! @name Radio configuration constants - FM transmitter deviation
 *  @{
 */
#define RCON_FMTX_LEN 3 // parameter set length
#define FMTX_DEV2  0x00
#define FMTX_DEV1  0xC0
#define FMTX_DEV0  0x04
/*! @} */

/*! @name Radio configuration constants - AFSK transmitter data rate and tone settings
 *  @{
 */
#define RCON_AFTX_LEN 7 // parameter set length
#define AFTX_RATE2 0x00
#define AFTX_RATE1 0x04
#define AFTX_RATE0 0xCF
#define AFTX_SPCE1 0x00
#define AFTX_SPCE0 0x13
#define AFTX_MARK1 0x00
#define AFTX_MARK0 0x23
/*! @} */

/*! @name Radio configuration constants - GMSK transmitter dividers and shaping constant
 *  @{
 */
#define RCON_GDTX_LEN    6 // parameter set length
#define GDTX_1200   0x04CE
#define GDTX_2400   0x099C
#define GDTX_4800   0x1338
#define GDTX_9600   0x2670
#define GDTX_19200  0x4CE0
#define GSHAPING    GAUSS_5    
/*! @} */

/*! @name State codes of the AX5043 packet tracnsceiver
 *  @{
 */
//! Packet receiver is waiting for incomming packet over radio
#define AX_TRX_WAIT  0
//! New data available in AX5043 buffer
#define AX_TRX_DATA  1
//! The packet overrun error
#define AX_TRX_ERROR 2
/*! @} */


//! Last status word of the AX5043 radio
extern uint16_t ax_status;

/*! \brief AX5043 startup status
 *  \note bit0 - AX5043 Signature byte is 0x51.
 *  \note bit1 - AX5043 Default scratchpad byte is 0xC5.
 *  \note bit2 - AX5043 Number 0xAA written in AX5043 scratchpad and read back.
 *  \note bit 8..15 - Power status register after basic AX5043 startup.
 *  \note bit 16..23 - Status of the last VCO autoranging.
 */
extern uint32_t ax_startup;

/*! \brief State of the AX5043 packet tracnsceiver
 */
extern volatile uint8_t ax_trx_st;

/*! \brief Load default AX5043 parameter sets from FLASH to RAM
 *  \note Call before ax_init() function. You can modify parameter set after loading.
 */
extern void ax_load_par(void);

/*!\brief The AX5043 radio short (16bit) Rd/Wr access
 * \details The data word read or wrote is 8 bit long, address is 7 bit long, and upper 7 bits of the AX5043 status is read. 
 * \param write Read = 0 or Write = 1 flag
 * \param adr Register address
 * \param data Data to write into the register. Ignored when reading.
 * \note Upper 7 bits of the last status word held in ax_status are updaed. Lower part of the ax_status variable is set 0.
 */
extern uint8_t ax_rw_2(uint8_t write, uint8_t adr, uint8_t data);

/*!\brief The AX5043 radio medium (24bit) Rd/Wr access
 * \details The data word read or wrote is 8 bit long, address is 12 bit long, and all 15 bits of the AX5043 status is read.
 * \param write Read = 0 or Write = 1 flag
 * \param adr Register address
 * \param data Data to write into the register. Ignored when reading.
 * \note The last status word held in ax_status is updated.
 */
extern uint8_t ax_rw_3(uint8_t write, uint16_t adr, uint8_t data);

/*!\brief The AX5043 radio N-byte Rd/Wr access
 * \details The N bytes of data are read and wrote, address is 7 bit long. Upper 7 bits of the AX5043 status is updated. 
 * \param write Read = 0 or Write = 1 flag
 * \param adr Register address
 * \param data Pointer to data to be written, pointer to array for read data storage.
 * \param num Number of data bytes to read or write
 * \note Lower part of the ax_status variable is set 0.
 */
void ax_rw_N(uint8_t write, uint8_t adr, uint8_t *data, uint16_t num);

/*!\brief SPI interface and AX5043 configuration
 * \note Configure I/O ports for SPI before.
 */
extern void ax_init(void);

/*!\brief The AX5043 radio synthesizer frequency setting
 * \param VFO Selects FREQA if AX_VFO_SEL_A or FREQB if AX_VFO_SEL_B.
 * \param frq Frequency in Hz
 * \param vcoran If =1, VCO autoranging is used. When =0, the PLL lock flag is tested only.
 * \note Should be handled with a timeout!!! (If it fails, use autoranging again.)
 * \note The power status and VCO & PLL is saved into ax_startup variable. VCO autoranging is always done.
 */
extern void ax_frequency(uint8_t vfo, uint32_t frq, uint8_t vcoran);

/*!\brief The AX5043 radio synthesizer VFO selection
 * \param VFO Selects FREQA if AX_VFO_SEL_A or FREQB if AX_VFO_SEL_B.
 * \note Only ax_status variable is updated.
 */
extern void ax_vfo(uint8_t vfo);

/*!\brief Reset and setup the AX5043 FIFO
 */
extern void ax_fifo_init(void);

/*!\brief Initialize the AX5043 CRC generator
 */
extern void ax_crc_init(void);

/*!\brief Sets the AX5043 radio as continuous FM transmitter
 * \note The ax_status and the power status is updated.
 */
extern void ax_mode_fm(void);

/*!\brief Sets the AX5043 radio as wire mode ASK transmitter
 * \note The ax_status and the power status is updated.
 */
extern void ax_mode_askw(void);

/*!\brief Sets the AX5043 radio as AFSK FIFO transceiver
 * \note The ax_status and the power status is updated.
 */
extern void ax_mode_afsk(uint8_t crc_mode);

/*!\brief Sets the AX5043 radio as GMSK G3RUH FIFO transceiver
 * \param type The G3RUH type specification
 * \param crc_mode Specifies the CRC type or CRC disable
 * \param encoding Specifies the encoding or scramnling
 * \note The ax_status and the power status is updated.
 */
extern void ax_mode_g3ruh(uint8_t type, uint8_t crc_mode, uint8_t encoding);

/*!\brief Set the power mode of AX5043 to RX, TX, ...
 * \param mode The power mode register, bit 0..3 data
 */
extern void ax_pwrmode(uint8_t mode);


#endif

/*! @} */

/**
 * @file       ax5043.h
 * @author     OK1CTR
 * @date       Apr 2026
 * @brief      Radio transceiver chip AX5043 driver
 *
 * @addtogroup grAx5043
 * @{
 */

#ifndef _AX5043_H_
#define _AX5043_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <common.h>

/* Typedefs ------------------------------------------------------------------*/

/*! Radio initialization state */
typedef enum
{
    ax_init_reset = 0,                   ///< just after reset
    ax_init_default,                     ///< default parameter sets loaded
    ax_init_ready,                       ///< basic initialization successful
    ax_init_frequency,                   ///< frequency was set successfully
    ax_init_fm,                          ///< configured for FM
    ax_init_askw,                        ///< configured for ASK Wire mode
    ax_init_afsk,                        ///< configured for AFSK
    ax_init_g3ruh                        ///< configured for G3RUH
} ax_init_state_e;

/*! AX5043 VFO selection */
typedef enum
{
    ax_vfo_a = 0,
    ax_vfo_b = 1
} ax_vfo_e;

/*! AX5043 power mode */
typedef enum
{
    ax_pwrmode_pd = 0,                   ///< power down, maintain registers
    ax_pwrmode_off = 1,                  ///< power off, register loss
    ax_pwrmode_standby = 5,              ///< oscillator running
    ax_pwrmode_rx_synt = 0x08,           ///< RX mode, synthesizer only
    ax_pwrmode_rx = 0x09,                ///< RX mode
    ax_pwrmode_tx_synt = 0x0C,           ///< TX mode, synthesizer only
    ax_pwrmode_tx = 0x0D                 ///< TX mode
} ax_pwrmode_e;

/*! AX5043 commands given through FIFO */
typedef enum
{
    ax_fifo_cmd_nop = 0,                 ///< no operation
    ax_fifo_cmd_clr_fifo = 1,            ///< clear FIFO (Gemini)
    ax_fifo_cmd_clr_errors = 2,          ///< clear overrun and underrun error flags
    ax_fifo_cmd_clr_data_flags = 3,      ///< clear data and flags
    ax_fifo_cmd_commit = 4               ///< commit
} ax_fifo_cmd_e;

/*! AX5043 TXCTRL command parameters */
typedef enum
{
    ax_txctrl_paon = 0x03,               ///< PA ON
    ax_txctrl_paoff = 0x02               ///< PA OFF
} ax_txctrl_param_e;

/*! AX5043 FIFO flags*/
typedef union
{
    uint8_t value;
    struct
    {
        uint8_t pkt_start:1;             ///< packet start
        uint8_t pkt_end:1;               ///< packet end
        uint8_t residue:1;               ///< residue from non-standard length packet
        uint8_t crc_fail:1;              ///< CRC check failed
        uint8_t addr_fail:1;             ///< address check failed
        uint8_t size_fail:1;             ///< packet size mismatch
        uint8_t abort:1;                 ///< abort operation mode
        uint8_t res:1;
    };
} ax_fifo_flags_t;

/*! AX5043 G3RUH mode baud rate selection */
typedef enum
{
    ax_g3ruh_rate_default = 0,
    ax_g3ruh_rate_1200,
    ax_g3ruh_rate_2400,
    ax_g3ruh_rate_4800,
    ax_g3ruh_rate_9600,
    ax_g3ruh_rate_19200,
} ax_g3ruh_rate_e;

/*! AX5043 CRC mode */
typedef enum
{
    ax_crc_mode_off = 0,                 ///< disable hardware CRC
    ax_crc_mode_crc32 = 6                ///< hardware CRC32
} ax_crc_mode_e;

/*! AX5043 encoding mode */
typedef enum
{
    ax_enc_mode_off = 0,                 ///< encoding disabled
    ax_enc_mode_nrzi = 3,                ///< NRZI code without scrambling
    ax_enc_mode_scramnler = 7            ///< NRZI code with scrambling
} ax_enc_mode_e;

/*! AX5043 Gaussian filtering selection */
typedef enum
{
    ax_gauss_0 = 0,                      ///< none
    ax_gauss_3 = 2,                      ///< BT = 0.3
    ax_gauss_5 = 3                       ///< BT = 0.6 (better preamble sync)
} ax_gauss_e;

/*! AX5043 radio state */
typedef enum
{
    ax_radiostate_idle = 0,              ///< do nothing
    ax_radiostate_powerdown = 0x1,       ///< power down
    ax_radiostate_tx_pll_set = 0x4,      ///< TX PLL settings
    ax_radiostate_tx = 0x6,              ///< TX
    ax_radiostate_tx_tail = 0x7,         ///< TX tail
    ax_radiostate_rx_pll_set = 0x8,      ///< RX PLL settings
    ax_radiostate_rx_ant_set = 0x9,      ///< RX antenna settings
    ax_radiostate_preamble_1 = 0xC,      ///< preamble 1
    ax_radiostate_preamble_2 = 0xD,      ///< preamble 2
    ax_radiostate_preamble_3 = 0xE,      ///< preamble 3
    ax_radiostate_rx = 0xF,              ///< RX
    ax_radiostate_wrong = 0x10           ///< physically non existent undefined value
} ax_radio_state_e;

/*! AX5043 radio status */
typedef union
{
    uint16_t value;
    struct
    {
        uint16_t res:1;                  ///< reserved
        uint16_t gpadc_irq:1;            ///< GPADC interrupt pending
        uint16_t lposc_irq:1;            ///< KPOSC interrupt pending
        uint16_t wakeup_irq:1;           ///< wake-up interrupt pending
        uint16_t xtal:1;                 ///< XTAL oscillator running flag
        uint16_t event:1;                ///< radio event pending
        uint16_t power:1;                ///< power interrupt pending
        uint16_t pwrgood:1;              ///< powergood (not brownout) flag
        uint16_t fifo_empty:1;           ///< FIFO empty flag
        uint16_t fifo_full:1;            ///< FIFO full flag
        uint16_t thr_count:1;            ///< threshold count (FIFO count > FIFO threshold)
        uint16_t thr_free:1;             ///< threshold free (FIFO free > FIFO threshold)
        uint16_t fifo_under:1;           ///< FIFO under flag
        uint16_t fifo_over:1;            ///< FIFO over flag
        uint16_t pll_lock:1;             ///< PLL lock flag
        uint16_t one:1;                  ///< reserved
    };
} ax_status_t;

/*! AX5043 startup status */
typedef struct
{
    uint32_t revision:1;                 ///< silicon revision - should be 0x51
    uint32_t scratchpad:1;               ///< default scratchpad content should be 0xC5
    uint32_t write_test:1;               ///< scratchpad write and readback test result
    uint32_t res1:5;
    uint32_t power_status:8;             ///< power status register
    uint32_t pll_ranging:8;              ///< status of the last VCO autoranging.
    uint32_t res2:8;
} ax_startup_t;

/* Functions -----------------------------------------------------------------*/

/**
 * @brief SPI interface and AX5043 configuration
 * @note Configure I/O ports for SPI before.
 */
extern void ax_init(void);

/**
 * @brief Load default configuration from FLASH to RAM
 * @note Call before ax_init() function. You can modify parameter set after loading.
 */
extern void ax_config_default(void);

/**
 * @\brief Set the power mode of AX5043 to RX, TX, ...
 * @param mode Selected power mode
 */
extern void ax_pwrmode(ax_pwrmode_e mode);

/**
 * @brief The AX5043 radio synthesizer frequency setting
 * @param VFO Selects FREQA if ax_vfo_a or FREQB if ax_vfo_b.
 * @param frq Frequency in Hz
 * @param vcoran If =1, VCO autoranging is used. When =0, the PLL lock flag is tested only.
 * @note Should be handled with a timeout!!! (If it fails, use autoranging again.)
 * @note The power status and VCO & PLL is saved into ax_startup variable. VCO autoranging is always done.
 */
extern void ax_frequency(ax_vfo_e vfo, uint32_t frq, uint8_t vcoran);

/**
 * @brief The AX5043 radio synthesizer VFO selection
 * @param VFO Selects FREQA if ax_vfo_a or FREQB if ax_vfo_b.
 * @note Only ax_status variable is updated.
 */
extern void ax_vfo(ax_vfo_e vfo);

/**
 * @brief Set the AX5043 transmission rate
 * @param reate Setting for TXRATE registers
 */
extern void ax_txrate(uint32_t rate);

/**
 * @brief Reset and setup the AX5043 FIFO
 */
extern void ax_fifo_init(void);

/**
 * @brief Send command to the AX5043 FIFO
 * @param cmd Command
 */
extern void ax_fifo_cmd(ax_fifo_cmd_e cmd);

/**
 * @brief Send the TXCTRL command into the AX5043 FIFO
 * @param parameter The parameter of the TXCTRL command
 */
void ax_fifo_txctrl(ax_txctrl_param_e parameter);

/**
 * @brief Initialize the AX5043 CRC generator
 */
extern void ax_crc_init(void);

/*
 * @brief Sets the AX5043 radio as continuous FM transmitter
 * @note The ax_status and the power status is updated.
 */
extern void ax_mode_fm(void);

/**
 * @brief Sets the AX5043 radio as wire mode ASK transmitter
 * @param reate Setting for TXRATE registers
 * @note The ax_status and the power status is updated.
 */
extern void ax_mode_ask_wire(uint32_t rate);

/**
 * @brief Sets the AX5043 radio as AFSK FIFO transceiver
 * @param crc_mode Mode of HW CRC unit
 * @note The ax_status and the power status is updated.
 */
extern void ax_mode_afsk(ax_crc_mode_e crc_mode);

/**
 * @brief Sets the AX5043 radio as GMSK G3RUH FIFO transceiver
 * @param type The G3RUH baud rate
 * @param crc_mode Specifies the CRC type or CRC disable
 * @param encoding Specifies the encoding or scramnling
 * @note The ax_status and the power status is updated.
 */
extern void ax_mode_g3ruh(ax_g3ruh_rate_e type, uint8_t crc_mode, uint8_t encoding);

/**
 * @brief Write given number of bytes into the transmit FIFO
 * @param data Data source buffer
 * @param length Data lenght
 * @param commit If true, data are commited
 */
extern void ax_fifo_write(uint8_t *data, uint8_t length, bool commit);

/**
 * @brief Read received data from FIFO
 * @param data Pointer to receive data buffer
 * @param buffer_limit Maximal number of bytes available in the receive buffer
 * @return Final number of bytes read
 */
extern uint32_t ax_fifo_read(uint8_t *data, uint32_t buffer_limit);

/**
 * @brief Control the transmitter PA
 * @param on Set PA on if true, off if false
 */
extern void ax_set_power_amp(bool on);

/**
 * @brief Get the last AC5043 startup status
 * @return Actual power status
 */
extern ax_startup_t ax_get_startup_status();

/**
 * @brief Get the last AC5043 status
 * @return Actual status
 */
extern ax_status_t ax_get_status();

/**
 * @brief Get the actual AC5043 radio state value
 * @return Actual radio state
 */
extern ax_radio_state_e ax_get_radio_state();

/**
 * @brief Get the actual initialization state of the radio
 * @return Actual initialization state
 */
extern ax_init_state_e ax_get_init_state();

/**
 * @brief Get the background RSSI raw value
 * @return Raw value of background RSSI or 128 when used non-properly
 */
extern uint32_t ax_get_rssi_bg();

/**
 * @brief Enable or disable needed interrupts
 * @note Do not overload an IRQ signal! Iy is an OR of all enabled flags!
 * @para put_en FIFO ready to data put in
 * @para get_en FIFO ready to data get out
 * @para state_en radio state changed
 * @param enable Enable state
 */
extern void ax_irq_enable(bool put_en, bool get_en, bool state_en);

/**
 * @brief New feature optional test function
 */
extern void ax_test();

/* ---------------------------------------------------------------------------*/

#endif  /* _AX5043_H_ */

/** @} */

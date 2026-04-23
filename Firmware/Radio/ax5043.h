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

/* Defines -------------------------------------------------------------------*/
/* Typedefs ------------------------------------------------------------------*/

/*! AX5043 VFO selection */
typedef enum
{
    ax_vfo_a = 0,
    ax_vfo_b = 1
} ax_vfo_e;

/*! AX5043 power mode */
typedef enum
{
    ax_pwrmode_tx = 0x0D,       ///< TX mode
    ax_pwrmode_rx = 0x09,       ///< RX mode
    ax_pwrmode_tx_synt = 0x0C,  ///< TX mode, synthesizer only
    ax_pwrmode_rx_synt = 0x08   ///< RX mode, synthesizer only
} ax_pwrmode_e;

/*! AX5043 FIFO commands */
typedef enum
{
    ax_fifo_cmd_clr_errors = 0x02,      ///< clear overrun and underrun error flags
    ax_fifo_cmd_clr_data_flags = 0x03,  ///< clear data and flags
    ax_fifo_cmd_commit = 0x43           ///< commit
} ax_fifo_cmd_e;

/*! AX5043 TXCTL command */
typedef enum
{
    ax_txctl_cmd_paon = 0x03,  ///< PA ON
    ax_txctl_cmd_paoff = 0x02  ///< PA OFF
} ax_txctl_cmd_e;

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

/*! AX5043 Gaussian filtering selection */
typedef enum
{
    ax_gauss_0 = 0,   ///< none
    ax_gauss_3 = 2,   ///< BT = 0.3
    ax_gauss_5 = 3    ///< BT = 0.6 (better preamble sync)
} ax_gauss_e;

/*! AX5043 receive state */
typedef enum
{
    ax_rx_wait = 0,  ///< packet receiver is waiting for incoming packet over radio
    ax_rx_data = 1,  ///< new data available in AX5043 buffer
    ax_rx_error = 2  ///< packet overrun error
} ax_rx_state_e;

/*! AX5043 startup status */
typedef struct
{
    uint32_t revision:1;      ///< silicon revision - should be 0x51
    uint32_t scratchpad:1;    ///< default scratchpad content should be 0xC5
    uint32_t write_test:1;    ///< scratchpad write and readback test result
    uint32_t res1:5;
    uint32_t power_status:8;  ///< power status register
    uint32_t pll_ranging:8;   ///< status of the last VCO autoranging.
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
 * @brief Reset and setup the AX5043 FIFO
 */
extern void ax_fifo_init(void);

/**
 * @brief Send command to the AX5043 FIFO
 * @param cmd Command
 */
extern void ax_fifo_cmd(ax_fifo_cmd_e cmd);

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
 * @note The ax_status and the power status is updated.
 */
extern void ax_mode_askw(void);

/**
 * @brief Sets the AX5043 radio as AFSK FIFO transceiver
 * @note The ax_status and the power status is updated.
 */
extern void ax_mode_afsk(uint8_t crc_mode);

/**
 * @brief Sets the AX5043 radio as GMSK G3RUH FIFO transceiver
 * @param type The G3RUH baud rate
 * @param crc_mode Specifies the CRC type or CRC disable
 * @param encoding Specifies the encoding or scramnling
 * @note The ax_status and the power status is updated.
 */
extern void ax_mode_g3ruh(ax_g3ruh_rate_e type, uint8_t crc_mode, uint8_t encoding);

/**
 * @brief Get the last AC5043 startup status
 * @return Actual power status
 */
extern ax_startup_t ax_get_startup_status();

/**
 * @brief Get the last AC5043 status
 * @return Actual status
 */
extern uint16_t ax_get_status();

/**
 * @brief Get the last AC5043 packet receiver status
 * @return Actual state
 */
extern ax_rx_state_e ax_get_rx_state();

/* ---------------------------------------------------------------------------*/

#endif  /* _AX5043_H_ */

/** @} */

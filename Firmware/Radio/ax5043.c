/**
 * @file       ax5043.c
 * @author     OK1CTR
 * @date       Apr 2026
 * @brief      Radio transceiver chip AX5043 driver
 *
 * @addtogroup grAx5043
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <ax5043.h>
#include <main.h>
#include <critical.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

//! Safe threshold for chunk write to FIFO (The same maximal length of chunk must be kept.)
#define FIFO_FREE_THRESHOLD                    20

/*! AN5043 status word flags */
#define AX_ST_FIFOTHRFREE                 0x0800

/*! AX5043 FIFO packet start flag */
#define AX_PACKET_START                   0x01
/*! AX5043 FIFO packet end flag */
#define AX_PACKET_END                     0x02

/*! AX5043 frame structure - CRC OFF */
#define AX_CRC_OFF                        0
/*! AX5043 frame structure - CRC-32 */
#define AX_CRC_CRC32                      6
/*! CRC generator initialization word */
#define AX_CRC_INIT                       0xFFFFFFFF

/*! Encoding disabled */
#define AX_ENC_DISABLED                   0
/*! NRZI code without scrambling */
#define AX_ENC_NRZI                       0x03
/*! NRZI code with scrambling */
#define AX_ENC_SCRAMBLER                  0x07

/* Private typedefs ----------------------------------------------------------*/

/*! Radio configuration - Performance Tuning Registers */
typedef struct
{
    uint8_t reg_F00;
    uint8_t reg_F0C;
    uint8_t reg_F0D;
    uint8_t reg_F10;
    uint8_t reg_F11;
    uint8_t reg_F1C;
    uint8_t reg_F21;
    uint8_t reg_F22;
    uint8_t reg_F23;
    uint8_t reg_F26;
    uint8_t reg_F34;
    uint8_t reg_F35;
    uint8_t reg_F44;
    uint8_t reg_F72;
    uint8_t reg_188;
    uint8_t reg_189;
} _PACKED_ rcfg_ptrg_t;

/*! Radio configuration - FM transmitter deviation */
typedef struct
{
    uint8_t reg_dev0;
    uint8_t reg_dev1;
    uint8_t reg_dev2;
} _PACKED_ rcfg_fm_t;

/*! Radio configuration - AFSK transmitter data rate and tone settings */
typedef struct
{
    uint8_t reg_rate0;
    uint8_t reg_rate1;
    uint8_t reg_rate2;
    uint8_t reg_spce0;
    uint8_t reg_spce1;
    uint8_t reg_mark1;
    uint8_t reg_mark0;
} _PACKED_ rcfg_afsk_t;

/*! Radio configuration - GMSK transmitter dividers and shaping constant */
typedef struct
{
    uint16_t gmsk_cfg_1200;
    uint16_t gmsk_cfg_2400;
    uint16_t gmsk_cfg_4800;
    uint16_t gmsk_cfg_9600;
    uint16_t gmsk_cfg_19200;
    ax_gauss_e gmsk_cfg_shaping;
} _PACKED_ gmsk_cfg_t;

/* Private constants ---------------------------------------------------------*/

/*! Radio configuration constants - Performance Tuning Registers */
rcfg_ptrg_t ptrg_init ={
        .reg_F00 = 0x0F,
        .reg_F0C = 0x00,
        .reg_F0D = 0x03,
        .reg_F10 = 0x04,
        .reg_F11 = 0x00,
        .reg_F1C = 0x07,
        .reg_F21 = 0x68, ///< 0x5C
        .reg_F22 = 0xFF, ///< 0x53
        .reg_F23 = 0x84, ///< 0x76
        .reg_F26 = 0x92,
        .reg_F34 = 0x28,
        .reg_F35 = 0x10, ///< TR 0xF35, fxtal < 24.8 MHz, ADCCLKMUX = 0 (0xF35[1:0] = 0)
        .reg_F44 = 0x25, ///< 0x24
        .reg_F72 = 0x00,
        .reg_188 = 0x0F,
        .reg_189 = 0x77
};

/*! Radio configuration constants - FM transmitter deviation */
rcfg_fm_t fm_init = {
        .reg_dev0 = 0x04,
        .reg_dev1 = 0xC0,
        .reg_dev2 = 0x00
};

/*! Radio configuration constants - AFSK transmitter data rate and tone settings */
rcfg_afsk_t afsk_init = {
        .reg_rate0 = 0xCF,
        .reg_rate1 = 0x04,
        .reg_rate2 = 0x00,
        .reg_spce0 = 0x13,
        .reg_spce1 = 0x00,
        .reg_mark0 = 0x23,
        .reg_mark1 = 0x00
};

/*! Radio configuration constants - GMSK transmitter dividers and shaping constant */
gmsk_cfg_t gmsk_cfg_init = {
        .gmsk_cfg_1200 = 0x04CE,
        .gmsk_cfg_2400 = 0x099C,
        .gmsk_cfg_4800 = 0x1338,
        .gmsk_cfg_9600 = 0x2670,
        .gmsk_cfg_19200 = 0x4CE0,
        .gmsk_cfg_shaping = ax_gauss_5
};

/* Private variables ---------------------------------------------------------*/

/*! Radio configuration in RAM - Performance Tuning Registers */
static rcfg_ptrg_t rcfg_ptrg_ram;
/*! Radio configuration in RAM - FM transmitter deviation */
static rcfg_fm_t rcfg_fm_ram;
/*! Radio configuration in RAM - AFSK transmitter data rate and tone settings */
static rcfg_afsk_t rcfg_afsk_ram;
/*! Radio configuration in RAM - GMSK transmitter dividers and shaping constant */
static gmsk_cfg_t gmsk_cfg_ram;
/*! State of the AX5043 packet receiver*/
static volatile ax_rx_state_e ax_rx_st = ax_rx_wait;

/* Exported variables --------------------------------------------------------*/

/*! Last status word of the AX5043 radio */
uint16_t ax_status = 0;
/*! AX5043 startup status */
uint32_t ax_startup = 0;

/* Private macros ------------------------------------------------------------*/

//! SEL line software control - H
#define ax_sel_H() LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_8)
//! SEL line software control - L
#define ax_sel_L() LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8)

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief The AX5043 radio short (16bit) Rd/Wr access
 * @details The data word read or wrote is 8 bit long, address is 7 bit long, and upper 7 bits of the AX5043 status is read.
 * @param write Read = 0 or Write = 1 flag
 * @param adr Register address
 * @param data Data to write into the register. Ignored when reading.
 * @note Upper 7 bits of the last status word held in ax_status are updaed. Lower part of the ax_status variable is set 0.
 */
static uint8_t ax_rw_2(uint8_t write, uint8_t adr, uint8_t data);

/**
 * @brief The AX5043 radio medium (24bit) Rd/Wr access
 * @details The data word read or wrote is 8 bit long, address is 12 bit long, and all 15 bits of the AX5043 status is read.
 * @param write Read = 0 or Write = 1 flag
 * @param adr Register address
 * @param data Data to write into the register. Ignored when reading.
 * @note The last status word held in ax_status is updated.
 */
static uint8_t ax_rw_3(uint8_t write, uint16_t adr, uint8_t data);

/**
 * @brief The AX5043 radio N-byte Rd/Wr access
 * @details The N bytes of data are read and wrote, address is 7 bit long. Upper 7 bits of the AX5043 status is updated.
 * @param write Read = 0 or Write = 1 flag
 * @param adr Register address
 * @param data Pointer to data to be written, pointer to array for read data storage.
 * @param num Number of data bytes to read or write
 * @note Lower part of the ax_status variable is set 0.
 */
static void ax_rw_N(uint8_t write, uint8_t adr, uint8_t *data, uint16_t num);

/* Functions -----------------------------------------------------------------*/

/* Default AX5043 configuration */
void ax_init(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    uint32_t n;

    // SEL
    LL_GPIO_SetOutputPin(RSEL_GPIO_Port, RSEL_Pin);
    GPIO_InitStruct.Pin = RSEL_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(RSEL_GPIO_Port, &GPIO_InitStruct);

    // AX5043 reset
    ax_rw_2(1, 0x02, 0xE0);
    for (n = 0; n < 255; n++) __NOP();
    ax_rw_2(1, 0x02, 0x60);  // 0x04 or 0x60 for external XO?
    for (n = 0; n < 255; n++) __NOP();

    // Ax5043 register read and write test
    n = 1;
    if (ax_rw_3(0, 0x00, 0x00) == 0x51) ax_startup |= n;
    n <<= 1;
    if (ax_rw_3(0, 0x01, 0x00) == 0xC5) ax_startup |= n;
    n <<= 1;
    ax_rw_2(1, 0x01, 0xAA);
    if (ax_rw_3(0, 0x01, 0x00) == 0xAA) ax_startup |= n;
    n <<= 1;
    ax_rw_3(1, 0x164, 0x06);  // single ended transmitter
    ax_rw_2(1, 0x26, 0x06);  // PWRAMP pin inverted PA control
    ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
    ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status register read

    // performance tuning registers initialization
    ax_rw_3(1, 0xF00, rcfg_ptrg_ram.reg_F00);
    ax_rw_3(1, 0xF0C, rcfg_ptrg_ram.reg_F0C);
    ax_rw_3(1, 0xF0D, rcfg_ptrg_ram.reg_F0D);
    ax_rw_3(1, 0xF10, rcfg_ptrg_ram.reg_F10);
    ax_rw_3(1, 0xF11, rcfg_ptrg_ram.reg_F11);
    ax_rw_3(1, 0xF1C, rcfg_ptrg_ram.reg_F1C);
    ax_rw_3(1, 0xF21, rcfg_ptrg_ram.reg_F21);
    ax_rw_3(1, 0xF22, rcfg_ptrg_ram.reg_F22);
    ax_rw_3(1, 0xF23, rcfg_ptrg_ram.reg_F23);
    ax_rw_3(1, 0xF26, rcfg_ptrg_ram.reg_F26);
    ax_rw_3(1, 0xF34, rcfg_ptrg_ram.reg_F34);
    ax_rw_3(1, 0xF35, rcfg_ptrg_ram.reg_F35);
    ax_rw_3(1, 0xF44, rcfg_ptrg_ram.reg_F44);
    ax_rw_3(1, 0xF72, rcfg_ptrg_ram.reg_F72);

    // baseband tuning initialization
    ax_rw_3(1, 0x188, rcfg_ptrg_ram.reg_188);
    ax_rw_3(1, 0x189, rcfg_ptrg_ram.reg_189);

    // IRQ PIN
    GPIO_InitStruct.Pin = RIRQ_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(RIRQ_GPIO_Port, &GPIO_InitStruct);

    // IRQ EXTI
    LL_GPIO_AF_SetEXTISource(RIRQ_GPIO_AF_EXTI_Port, RIRQ_GPIO_AF_EXTI_Line);
    EXTI_InitStruct.Line_0_31 = RIRQ_EXTI_Line;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_ClearFlag_0_31(RIRQ_EXTI_Line);
    NVIC_EnableIRQ(RIRQn);

    // analog debug output
#ifdef _DEBUG_ANALOG_
    //ax_rw_3(1, 0x332, 0x03);  // output TRKFREQUENCY
    ax_rw_3(1, 0x332, 0x07);  // output RSSI, A bullshit in datasheet!
    ax_rw_3(1, 0x330, 0x00);  // DACVALUE = 0
    ax_rw_3(1, 0x331, 0x0C);  // DACSHIFT = 12 bit
    //ax_rw_3(1, 0x331, 0x08);  // DACSHIFT = 8 bit, TRKFRQ.. zoom
    ax_rw_2(1, 0x25, 0x05);  // ANTSEL as output
    //ax_rw_2(1, 0x26, 0x05);  // PWRAMP as output
#endif

    // digital debug output
#ifdef _DEBUG_DIGITAL_
    ax_rw_2(1, 0x22, 0x04);  // DCLK -> modem clock output, more variants!
    ax_rw_2(1, 0x23, 0x07);  // DATA -> modem data output a LOT of variants! (0x07 - raw data, NRZI decoded, no descrabled)
#endif
}


/* Load default configuration from FLASH to RAM */
void ax_config_default(void)
{
    rcfg_ptrg_ram = ptrg_init;
    rcfg_fm_ram = fm_init;
    rcfg_afsk_ram = afsk_init;
    gmsk_cfg_ram = gmsk_cfg_init;
    return;
}


/* Set the power mode of AX5043 to RX, TX, etc. */
void ax_pwrmode(ax_pwrmode_e pwrmode)
{
    ax_rw_2(1, 0x02, 0x60 | ((uint8_t)pwrmode & 0x0F));
}


/* The AX5043 radio synthesizer frequency setting */
void ax_frequency(ax_vfo_e vfo, uint32_t frq, uint8_t vcoran)
{
    uint64_t x;
    uint8_t adr;

    if (vfo == ax_vfo_a)
    {
        // FREQA register set will be used, selected in 0x30 register, internal PLL filter
        adr = 0x37; ax_rw_2(1, 0x30, 0x09);
    }
    else
    {
        // FREQB register set will be used, selected in 0x30 register, internal PLL filter
        adr = 0x3F; ax_rw_2(1, 0x30, 0x89);
    }

    ax_rw_2(1, 0x31, 0x08);  // Default charge pump current
    ax_rw_2(1, 0x32, 0x04);  // fpd = fxtal, RF prescaler =2
    x = (uint64_t) frq * (uint64_t) 0x1000000L / (uint64_t) RXTAL_FREQUENCY;

#if (RXTAL_F_ERROR_COMP_EN)
    x = x * (uint64_t) 1000000000L / ((uint64_t) RXTAL_F_ERROR_COMP + (uint64_t) 1000000000L);
#endif

    // Frequency register setting
    ax_rw_2(1, adr, (x & 0xFF) | 0x01);  // +1/2
    x >>= 8; adr--;
    ax_rw_2(1, adr, x & 0xFF);
    x >>= 8; adr--;
    ax_rw_2(1, adr, x & 0xFF);
    x >>= 8; adr--;
    ax_rw_2(1, adr, x & 0xFF);

    // VCO autoranging
    if (vcoran)
    {
        ax_rw_2(1, 0x33, 0x10);
        while ((x = ax_rw_2(0, 0x33, 0x00)) & 0x10)
        {
        }
    }
    else
    {
        // no autoranging, PLL lock test only
        // should be handled with a timeout!!! (If it fails, use autoranging again.)
        while ((x = ax_rw_2(0, 0x33, 0x00)) & 0x40)
        {
        }
    }

    // save VCO & PLL status
    ax_startup |= x << 16;
}


/* The AX5043 radio synthesizer VFO selection */
void ax_vfo(ax_vfo_e vfo)
{
    ax_rw_2(1, 0x30, 0x09 | (vfo == ax_vfo_a) ? 0x00 : 0x80);
}


/* Reset and setup the AX5043 FIFO */
void ax_fifo_init(void)
{
    ax_fifo_cmd(ax_fifo_cmd_clr_data_flags);
    ax_fifo_cmd(ax_fifo_cmd_clr_errors);
    ax_rw_2(1, 0x2E, 0);
    ax_rw_2(1, 0x2F, FIFO_FREE_THRESHOLD);  // safe thdreshold for writes to FIFO
}


/* Send command to the AX5043 FIFO */
void ax_fifo_cmd(ax_fifo_cmd_e cmd)
{
    ax_rw_2(1, 0x28, (uint8_t)cmd);
}


/* Initialize the AX5043 CRC generator */
void ax_crc_init(void)
{
    static uint8_t crc_init[] =
    {
        AX_CRC_INIT >> 24,
        (AX_CRC_INIT >> 16) & 0xFF,
        (AX_CRC_INIT >> 8) & 0xFF,
        AX_CRC_INIT & 0xFF
    };

    ax_rw_N(1, 0x14, crc_init, 4);
}


/* Sets the AX5043 radio as continuous FM transmitter */
void ax_mode_fm(void)
{
    ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
    ax_rw_2(1, 0x10, 0x0B);  // FM

    ax_rw_3(1, 0x161, rcfg_fm_ram.reg_dev2);  // fskdev2
    ax_rw_3(1, 0x162, rcfg_fm_ram.reg_dev1);  // fskdev1
    ax_rw_3(1, 0x163, rcfg_fm_ram.reg_dev0);  // fskdev0, dev = +/-fxtal / 2^(fskfev0[2:0] + 1) => min. zdvih 62.5 kHz - DEFINE!!

    ax_rw_3(1, 0x301, 0x05);  // gpadcperiod, sr = fxtal / (32 * gpadcperiod) => 100 kHz
    ax_rw_3(1, 0x300, 0x06);  // gpadcctrl, continuous sampling, gpadc13=1 => Modulation input is GPADC1 and GPADC2 differentially
    ax_rw_2(1, 0x23, 0x04);  // pinfuncdata, undocumented code to switch TX on permanently
    ax_pwrmode(ax_pwrmode_tx);
    ax_rw_2(1, 0x27, 0x01);  // PWRAMP - PA on
    ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
}


/* Sets the AX5043 radio as wire mode ASK transmitter */
void ax_mode_askw(void)
{
    ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
    ax_rw_2(1, 0x10, 0x00);  // ASK
    ax_rw_2(1, 0x11, 0x00);  // No differential encoding
    ax_rw_3(1, 0x165, 0x00);  // txrate2
    ax_rw_3(1, 0x166, 0x00);  // txrate1
    ax_rw_3(1, 0x167, 0x00);  // txrate0
    //ax_rw_3(1, 0x167, 0x11);  // rxrate0 => DEFINEd in morse.h, used in morse_init
    // TXRATE = (WPM * 2^24) / (2.4 * fxtal)
    ax_rw_2(1, 0x23, 0x04);  // pinfuncdata, wire mode?
    ax_rw_2(1, 0x22, 0x05);  // pinfuncdata, wire mode?
    ax_rw_3(1, 0x164, 0x02);  // Single ended transmitter, no amplitude shaping
    ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
}


/* Sets the AX5043 radio as AFSK FIFO transceiver */
void ax_mode_afsk(uint8_t crc_mode)
{
    ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
    ax_rw_2(1, 0x10, 0x0A);  // AFSK
    ax_rw_2(1, 0x11, 0x03);  // encoding, NRZI, scrambler is off
#ifdef _TONE_DEBUG_
    ax_rw_2(1, 0x11, 0x00);  // encoding off for tone debug
    ax_rw_2(1, 0x12, 0x00);  // no framing for RAW data
#endif
    ax_rw_2(1, 0x12, 0x04 | (crc_mode & 0x07) << 4);  // HDLC framing, CRC mode
    // TXRATE = (bitrate / fxtal * 2^24) + 1 -> put 1200 Bd
    ax_rw_3(1, 0x165, rcfg_afsk_ram.reg_rate2);  // txrate2
    ax_rw_3(1, 0x166, rcfg_afsk_ram.reg_rate1);  // txrate1
    ax_rw_3(1, 0x167, rcfg_afsk_ram.reg_rate0);  // txrate0
    // AFSK 1200 Hz / 2200 Hz
    ax_rw_3(1, 0x110, rcfg_afsk_ram.reg_spce1);  // afskspace1
    ax_rw_3(1, 0x111, rcfg_afsk_ram.reg_spce0);  // afskspace0
    ax_rw_3(1, 0x112, rcfg_afsk_ram.reg_mark1);  // afskmark1
    ax_rw_3(1, 0x113, rcfg_afsk_ram.reg_mark0);  // afskmark0
    ax_rw_3(1, 0x164, 0x06); // single ended transmitter, amplitude shaping
    // FM deviation 3 or 5 kHz peak? Where to set?
    ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
}


/* Sets the AX5043 radio as GMSK G3RUH FIFO transceiver */
void ax_mode_g3ruh(ax_g3ruh_rate_e type, uint8_t crc_mode, uint8_t encoding)
{
    uint32_t txrate;

    switch (type)
    {
        case ax_g3ruh_rate_1200:
            txrate = gmsk_cfg_ram.gmsk_cfg_1200;
            break;
        case ax_g3ruh_rate_2400:
            txrate = gmsk_cfg_ram.gmsk_cfg_2400;
            break;
        case ax_g3ruh_rate_4800:
            txrate = gmsk_cfg_ram.gmsk_cfg_4800;
            break;
        case ax_g3ruh_rate_9600:
            txrate = gmsk_cfg_ram.gmsk_cfg_9600;
            break;
        case ax_g3ruh_rate_19200:
            txrate = gmsk_cfg_ram.gmsk_cfg_19200;
            break;
        default:
            txrate = gmsk_cfg_ram.gmsk_cfg_9600;  // preferred as default
            break;
    }
    txrate++;

    ax_rw_2(1, 0x27, 0x00);  // PWRAMP - PA off
    ax_rw_2(1, 0x10, 0x08);  // FSK, because MSK is not working
    ax_rw_2(1, 0x11, encoding);  // encoding, NRZI, scrambler

    ax_rw_2(1, 0x12, 0x04 | (crc_mode & 0x07) << 4);  // HDLC framing, CRC mode
    // TXRATE = (bitrate / fxtal * 2^24) -> put 1200 Bd
    ax_rw_3(1, 0x167, txrate & 0xFF);  // TXRATE0
    ax_rw_3(1, 0x166, (txrate >> 8) & 0xFF);  // TXRATE1
    ax_rw_3(1, 0x165, 0);  // TXRATE2
    // FSK deviation, don't set automatically
    txrate >>= 3; txrate++;  // from the AX config tool
    ax_rw_3(1, 0x163, txrate & 0xFF);  // FSKDEV0
    ax_rw_3(1, 0x162, (txrate >> 8) & 0xFF);  // FSKDEV1
    ax_rw_3(1, 0x161, 0);  // FSKDEV2
    // transmitter setup
    ax_rw_3(1, 0x160, (uint8_t)gmsk_cfg_ram.gmsk_cfg_shaping); // frequency shaping
    ax_rw_3(1, 0x164, 0x06);  // single ended transmitter, amplitude shaping

    // receiver - packet format
    ax_rw_3(1, 0x200, 0x01);  // data lsb first
    ax_rw_3(1, 0x201, 0x80);  // length config
    ax_rw_3(1, 0x202, 0x01);  // packet length offset!
    ax_rw_3(1, 0x203, 0xF0);  // max length

    // receiver - packet controller
    ax_rw_3(1, 0x220, 0x33);
    ax_rw_3(1, 0x221, 0x14);
    ax_rw_3(1, 0x223, 0x33);
    ax_rw_3(1, 0x224, 0x14);
    ax_rw_3(1, 0x225, 0x00);
    ax_rw_3(1, 0x226, 0x73);
    ax_rw_3(1, 0x227, 0x00);
    ax_rw_3(1, 0x228, 0x03);
    ax_rw_3(1, 0x229, 0x00);  // pream 1 timeout
    ax_rw_3(1, 0x22A, 0x17);  // pream 2 timeout
    ax_rw_3(1, 0x22B, 0x00);  // pream 2 timeout
    ax_rw_3(1, 0x22C, 0xF8);
    ax_rw_3(1, 0x22F, 0x00);
    ax_rw_3(1, 0x230, 13);    // max chunk size 13 = 240 B !!!
    ax_rw_3(1, 0x231, 0x00);
    ax_rw_3(1, 0x232, 0x54);  // store flags ANT RSSI, RSSI, RF FOFFS

#ifdef _DEBUG_ACCEPT_
    #if _DEBUG_ACCEPT_ == ALL
    ax_rw_3(1, 0x233, 0x3F);  // accept as much you can
    #elif _DEBUG_ACCEPT_ == CRC
    ax_rw_3(1, 0x233, 0x04);  // accept wrong crc
    #elif _DEBUG_ACCEPT_ == CRCADRS
    ax_rw_3(1, 0x233, 0x1C);  // accept wrong crc and address and size
    #elif _DEBUG_ACCEPT_ == LARGE
    ax_rw_3(1, 0x233, 0x20);  // accept lrgp
    #elif _DEBUG_ACCEPT_ == ALLWOLARGE
    ax_rw_3(1, 0x233, 0x1F);  // accept all except long puckets
    ax_rw_3(1, 0x233, 0x08);  // experimental
    #endif
#else
    ax_rw_3(1, 0x233, 0x00);  // accept nothing special
#endif

    // receiver parameters
    switch (type)
    {
        case ax_g3ruh_rate_1200:
            // NOT DEFINED !!!
            break;
        case ax_g3ruh_rate_2400:
            // NOT DEFINED !!!
            break;
        case ax_g3ruh_rate_4800:
            // NOT DEFINED !!!
            break;
        case ax_g3ruh_rate_9600:
            ax_rw_3(1, 0x102, 0x0E);  // DECIMATION
            ax_rw_3(1, 0x100, 0x03);  // Fif, MSB
            ax_rw_3(1, 0x101, 0x01);  // Fif, LSB
            break;
        case ax_g3ruh_rate_19200:
            ax_rw_3(1, 0x102, 0x07);  // DECIMATION
            ax_rw_3(1, 0x100, 0x06);  // Fif, MSB
            ax_rw_3(1, 0x101, 0x02);  // Fif, LSB
            break;
        default:
            // NOT DEFINED !!!
            break;
    }

    ax_rw_3(1, 0x103, 0x00);  // DATA RATE, MSB - bitrate independent
    ax_rw_3(1, 0x104, 0x3C);  // DATA RATE
    ax_rw_3(1, 0x105, 0xE4);  // DATA RATE, LSB
    ax_rw_3(1, 0x106, 0x00);  // MAXDROFFSET, MSB - don't use if RX/TX timing error is less than 0.15%
    ax_rw_3(1, 0x107, 0x00);  // MAXDROFFSET
    ax_rw_3(1, 0x108, 0x00);  // MAXDROFFSET, LSB
    ax_rw_3(1, 0x109, 0x80);  // MAXRFOFFSET | 0x80 - track at LO1
    ax_rw_3(1, 0x10A, 0x04);  // 04 (~+/-1 kHz) MAXRFOFFSET
    ax_rw_3(1, 0x10B, 0x90);  // 90 (~+/-1 kHz) MAXRFOFFSET, LSB (!! 0x00FF works surprisingly good, 2% but still V)
    ax_rw_3(1, 0x10C, 0x00);  // 00 FSKDMAX1
    ax_rw_3(1, 0x10D, 0xA6);  // A6 FSKDMAX0 - bitrate independent
    ax_rw_3(1, 0x10E, 0xFF);  // FF FSKDMIN1
    ax_rw_3(1, 0x10F, 0x5A);  // 5A FSKDMIN0 - bitrate independent
    ax_rw_3(1, 0x116, 0x00);  // baseband AFC loop leakage, default 0

    // receiver parameter set config
    ax_rw_3(1, 0x117, 0xF4);  // dets 0, 1 and 3
#ifdef _DEBUG_RECSET_
    ax_rw_3(1, 0x117, 0x00);  // dets 0 only - debug only
#endif

    // receiver paramater set 0
    ax_rw_3(1, 0x120, 0xB5);  // AGCGAIN0 - bitrate dependent
    ax_rw_3(1, 0x121, 0x84);  // AGCTARGET0 - bitrate independent
    ax_rw_3(1, 0x124, 0xF8);  // TIMEGAIN0, default 0xF8
    ax_rw_3(1, 0x125, 0xF2);  // DRGAIN0, default 0xF2
    ax_rw_3(1, 0x126, 0xC3);  // PHASEGAIN0, default 0xC3
    ax_rw_3(1, 0x127, 0x0F);  // FREQGAINA0, off
    ax_rw_3(1, 0x128, 0x1F);  // FREQGAINB0, off
    ax_rw_3(1, 0x129, 0x09);  // 09 FREQGAINC0 - bitrate dependent
    ax_rw_3(1, 0x12A, 0x09);  // 09 FREQGAIND0 - bitrate dependent
    ax_rw_3(1, 0x12B, 0x06);  // AMPLITUDEGAIN0, default 0x46
    ax_rw_3(1, 0x12C, 0x00);  // FREQDEV10, default 0
    ax_rw_3(1, 0x12D, 0x00);  // FREQDEV00, default 0
    ax_rw_3(1, 0x12E, 0x16);  // FOURFSK0, default 16
    ax_rw_3(1, 0x12F, 0x00);  // BBOFSRES0, default 0x88

    // receiver paramater set 1
    ax_rw_3(1, 0x130, 0xB5);  // AGCGAIN1 - bitrate dependent
    ax_rw_3(1, 0x131, 0x84);  // AGCTARGET1 - bitrate independent
    ax_rw_3(1, 0x134, 0xF6);  // TIMEGAIN1, default 0xF8
    ax_rw_3(1, 0x135, 0xF1);  // DRGAIN1, default 0xF2
    ax_rw_3(1, 0x136, 0xC3);  // PHASEGAIN1, default 0xC3
    ax_rw_3(1, 0x137, 0x0F);  // FREQGAINA1, off
    ax_rw_3(1, 0x138, 0x1F);  // FREQGAINB1, off
    ax_rw_3(1, 0x139, 0x0E);  // 09 FREQGAINC1 - bitrate dependent
    ax_rw_3(1, 0x13A, 0x0E);  // 09 FREQGAIND1 - bitrate dependent
    ax_rw_3(1, 0x13B, 0x06);  // AMPLITUDEGAIN1, default 0x46
    ax_rw_3(1, 0x13C, 0x00);  // FREQDEV11, default 0
    ax_rw_3(1, 0x13D, 0x32);  // FREQDEV01, default 0
    ax_rw_3(1, 0x13E, 0x16);  // FOURFSK1, default 16
    ax_rw_3(1, 0x13F, 0x00);  // BBOFSRES1, default 0x88

    // receiver paramater set 3
    ax_rw_3(1, 0x150, 0xFF);  // AGCGAIN3 - bitrate independent
    ax_rw_3(1, 0x151, 0x84);  // AGCTARGET3 - bitrate independent
    ax_rw_3(1, 0x154, 0xF5);  // TIMEGAIN3, default 0xF8
    ax_rw_3(1, 0x155, 0xF0);  // DRGAIN3, default 0xF2
    ax_rw_3(1, 0x156, 0xC3);  // PHASEGAIN3, default 0xC3
    ax_rw_3(1, 0x157, 0x0F);  // FREQGAINA3, off
    ax_rw_3(1, 0x158, 0x1F);  // FREQGAINB3, off
    ax_rw_3(1, 0x159, 0x0D);  // 0D FREQGAINC3 - bitrate dependent
    ax_rw_3(1, 0x15A, 0x0D);  // 0D FREQGAIND3 - bitrate dependent
    ax_rw_3(1, 0x15B, 0x06);  // AMPLITUDEGAIN3, default 0x46
    ax_rw_3(1, 0x15C, 0x00);  // FREQDEV13, default 0
    ax_rw_3(1, 0x15D, 0x32);  // FREQDEV03, default 0
    ax_rw_3(1, 0x15E, 0x16);  // FOURFSK3, default 16
    ax_rw_3(1, 0x15F, 0x00);  // BBOFSRES3, default 0x88

    // pattern match
    ax_rw_3(1, 0x210, 0xAA);  // AA MATCH0PAT3
    ax_rw_3(1, 0x211, 0xCC);  // CC MATCH0PAT2
    ax_rw_3(1, 0x212, 0xAA);  // AA MATCH0PAT1
    ax_rw_3(1, 0x213, 0xCC);  // CC MATCH0PAT0
    ax_rw_3(1, 0x214, 0x00);  // MATCH0LEN
    ax_rw_3(1, 0x215, 0x00);  // MATCH0MIN
    ax_rw_3(1, 0x216, 0x1F);  // MATCH0MAX
    ax_rw_3(1, 0x218, 0x7E);  // MATCH1PAT1 - bit reversed preamble!
    ax_rw_3(1, 0x219, 0x7E);  // MATCH1PAT0 - bit reversed preamble!
    ax_rw_3(1, 0x21C, 0x0A);  // MATCH1LEN - 0x0A without FEC, 0x8A with FEC !!!
    ax_rw_3(1, 0x21D, 0x00);  // MATCH1MIN
    ax_rw_3(1, 0x21E, 0x0A);  // MATCH1MAX

    // irq
    ax_rw_2(1, 0x24, 0x03);  // PINFUNCIRQ - IRQ output
    //ax_rw_2(1, 0x07, 0x40);  // Radio controller event
    ax_rw_2(1, 0x07, 0x01);  // FIFO not empty
    ax_rw_2(1, 0x09, 0x04);  // RADIOEVENTMASK

    ax_startup |= ax_rw_2(0, 0x03, 0x00) << 8;  // power status
}

/* Private functions ---------------------------------------------------------*/

/* The AX5043 radio short (16bit) Rd/Wr access */
static uint8_t ax_rw_2(uint8_t write, uint8_t adr, uint8_t data)
{
    uint8_t a;

    critical_enter();
    ax_sel_L();

    // octet 1
    a = rspi_trx8(((write) ? 0x80 : 0) | (adr & 0x7F));
    //ax_status &= 0x7F00; TEST !!!!
    ax_status &= 0x00FF;
    ax_status |= (((uint16_t) a) << 8) & 0x7F00;

    // octet 2
    a = rspi_trx8((write) ? data : 0);

    ax_sel_H();
    critical_exit();

    return(a);
}


/* The AX5043 radio medium (24bit) Rd/Wr access */
static uint8_t ax_rw_3(uint8_t write, uint16_t adr, uint8_t data)
{
    uint8_t a;

    critical_enter();
    ax_sel_L();

    // octet 1
    a = rspi_trx8(((write) ? 0x80 : 0) | 0x70 | ((adr >> 8) & 0x0F));
    ax_status = (((uint16_t) a) << 8) & 0x7F00;

    // octet 1
    a = rspi_trx8(adr & 0xFF);
    ax_status |= a;

    // octet 3
    a = rspi_trx8((write) ? data : 0);

    ax_sel_H();
    critical_exit();

    return(a);
}


/* The AX5043 radio N-byte Rd/Wr access */
static void ax_rw_N(uint8_t write, uint8_t adr, uint8_t *data, uint16_t num)
{
    uint8_t a, i;

    critical_enter();
    ax_sel_L();

    // octet 1
    a = rspi_trx8(((write) ? 0x80 : 0) | (adr & 0x7F));
    //ax_status &= 0x7F00; TEST !!!!
    ax_status &= 0x00FF;
    ax_status |= (((uint16_t) a) << 8) & 0x7F00;

    // octet 2..N
    for (i = 0; i < num; i++)
    {
        a = rspi_trx8((write) ? *data : 0);
        if (!write)
        {
            *data = a;
        }
        data++;
    }

    ax_sel_H();
    critical_exit();
}

/* ISR -----------------------------------------------------------------------*/

/* External interrupt handler - RIRQ at AX5043, PB9 in STM32 */
void RIRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag_0_31(RIRQ_EXTI_Line))
    {
        LL_EXTI_ClearFlag_0_31(RIRQ_EXTI_Line);
        if (ax_rx_st == ax_rx_wait)
        {
            ax_rx_st = ax_rx_wait;  // register the incoming data
        }
        else
        {
            ax_rx_st = ax_rx_error;  // or packet overrun error
        }
    }
}

/* ---------------------------------------------------------------------------*/

/** @} */

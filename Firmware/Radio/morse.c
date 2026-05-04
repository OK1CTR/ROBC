/**
 * @file       morse.c
 * @author     OK1CTR
 * @date       Apr 2026
 * @brief      Morse code transmitter module working with the AX radio in ASK mode
 *
 * @addtogroup grMorse
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <morse.h>
#include <main.h>
#include <ctype.h>
#include <critical.h>
#include <ax5043.h>

/* Private defines -----------------------------------------------------------*/

/*! Morse telegram buffer length */
#define MORSE_BFLEN                     64

/*! Dot */
#define DOT                              1
/*! Dash */
#define DASH                     (5 * DOT)
/*! Space between characters */
#define DASH_SP                  (5 * DOT)
/*! Space between words */
#define WORD_SP        (2 * DASH_SP + DOT)

/* Private macros ------------------------------------------------------------*/

//* Morse tone generator on */
#define set_tone_on() LL_GPIO_SetOutputPin(RDDA_GPIO_Port, RDDA_Pin)
/*! Morse tone generator off */
#define set_tone_off() LL_GPIO_ResetOutputPin(RDDA_GPIO_Port, RDDA_Pin)

/* Private typedefs ----------------------------------------------------------*/

/*! State of conversion string to characters */
typedef enum
{
    character_walk_free = 0,              ///> free to transmit
    character_walk_run                    ///> transmission in progress
} character_walk_t;

/*! State of conversion characters to symbols */
typedef enum
{
    symbol_walk_free = 0,                 ///> free to transmit
    symbol_walk_enter,                    ///> new character enter
    symbol_walk_run                       ///> character transmission in progress
} symbol_walk_t;

/*! Morse symbol type */
typedef enum
{
    symbol_dot_mark = 0,
    symbol_dash_mark,
    symbol_dot_space,
    symbol_dash_space,
    symbol_word_space,
} symbol_t;

/*! Additional flags */
typedef struct
{
    uint32_t space_rq:1;
    uint32_t pa_active:1;
    uint32_t res:30;
} flags_t;

/*! Morse context structure */
typedef struct
{
    volatile uint32_t morse_bf_head;           ///> output buffer write pointer - head
    volatile uint32_t morse_bf_tail;           ///> output buffer read pointer - tail
    volatile character_walk_t character_walk;  ///> state of conversion string to characters
    volatile symbol_walk_t symbol_walk;        ///> state of conversion characters to symbols
    uint32_t n;                                ///> bit mask variable
    uint32_t code;                             ///> binary coded morse
    uint32_t lng;                              ///> binary code length
    symbol_t symbol;                           ///> symbol to transmit
    uint32_t count;                            ///> number of subsymbols to transmit
    bool tone;                                 ///> transmit subsymbol mark or space
    flags_t flags;                             ///> additional state flags
} morse_context_t;

/* Private constants ---------------------------------------------------------*/

/*! Letters, marks from MSB, 2 LSB bits = length - 1 */
const uint8_t morse_c[] =
{
        //! A     B      C     D     E     F
        0x41, 0x83, 0xA3, 0x82, 0x00, 0x23,
   
        //! G     H      I     J     K     L
        0xC2, 0x03, 0x01, 0x73, 0xA2, 0x83,
   
        //! M     N      O     P     Q     R
        0xC1, 0x81, 0xE2, 0x63, 0xD3, 0x42,
   
        //! s     T      U     V     W     X
        0x02, 0x80, 0x22, 0x13, 0x62, 0x93,
   
        //! Y     Z
        0XB3, 0XC3
};

/*! Numbers, marks from MSB, constant length 5 marks */
const uint8_t morse_n[] =
{
        //! 0     1     2      3     4
        0xF8, 0x78, 0x38, 0x18, 0x08,
   
        //! 5     6     7      8     9
        0x00, 0x80, 0xC0, 0xE0, 0xF0
};

/*! Special characters, character table */
const uint8_t morse_zn[] =
{
        '.',  ',',  '?',  '-',  '/',  '+',  '@',  '_'
};

/*! Special characters, marks from MSB, LSB is length: 0=5, 1=6 marks */
const uint8_t morse_zc[] =
{
        0x55, 0xCD, 0x31, 0x85, 0x90, 0x50, 0x15, 0x88
};

/* Private variables ---------------------------------------------------------*/

/*! Morse telegram buffer */
static uint8_t morse_bf[MORSE_BFLEN];
/*! Morse private variables */
static morse_context_t mc = {0};

/* Functions -----------------------------------------------------------------*/

/* Morse keyer initialization and rate setting */
void morse_init(uint32_t wpm)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

    mc.morse_bf_head = 0;
    mc.morse_bf_tail = 0;
    mc.symbol_walk = symbol_walk_free;
    mc.n = 0x80;
    mc.code = 0;
    mc.lng = 0;
    mc.flags.pa_active = 0;
    mc.character_walk = character_walk_free;
    mc.symbol_walk = symbol_walk_free;

    if (wpm < 5)
    {
        wpm = 5;
    }
    if (wpm > 120)
    {
        wpm = 120;
    }

    // RDCL IRQ PIN
    GPIO_InitStruct.Pin = RDCL_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    LL_GPIO_Init(RDCL_GPIO_Port, &GPIO_InitStruct);

    // RDCL IRQ EXTI
    LL_GPIO_AF_SetEXTISource(RDCL_GPIO_AF_EXTI_Port, RCDL_GPIO_AF_EXTI_Line);
    EXTI_InitStruct.Line_0_31 = RDCL_EXTI_Line;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_ClearFlag_0_31(RDCL_EXTI_Line);
    NVIC_EnableIRQ(RDCLn);

    // RDDA output
    LL_GPIO_ResetOutputPin(RDDA_GPIO_Port, RDDA_Pin);
    GPIO_InitStruct.Pin = RDDA_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    LL_GPIO_Init(RDDA_GPIO_Port, &GPIO_InitStruct);

    // txrate = (WPM * 2^24) / (2.4 * fxtal)
    ax_mode_ask_wire((((uint64_t) wpm) * 6990507L) / RXTAL_FREQUENCY);
}


/* Morse keyer deinitialization */
void morse_deinit()
{
    ax_txrate(0);
    NVIC_DisableIRQ(RDCLn);
    LL_GPIO_SetPinPull(RDDA_GPIO_Port, RDDA_Pin, LL_GPIO_PULL_DOWN);
}


/* Morse keyer regular job */
void morse_job(void)
{
    uint8_t chr;

    if (mc.character_walk == character_walk_free)  // nothing in process
    {
        if (mc.morse_bf_tail == mc.morse_bf_head)  // nothing in buffer
        {
            if (mc.flags.pa_active == 1)  // PA is on
            {
                // turn PA off
                mc.flags.pa_active = 0;
                ax_set_power_amp(false);
                LL_GPIO_SetPinMode(RDDA_GPIO_Port, RDDA_Pin, LL_GPIO_MODE_INPUT);
            }
        }
        else  // at least one more character in buffer
        {
            mc.character_walk = character_walk_run;
            mc.n = 0x80;

            // pull a character from the transmit buffer
            critical_enter();
            chr = morse_bf[mc.morse_bf_tail];
            mc.morse_bf_tail = (uint32_t)(mc.morse_bf_tail + 1) % MORSE_BFLEN;
            critical_exit();

            // convert new character to the binary coded morse
            if (chr >= '0' && chr <= '9')  // number coding
            {
                mc.code = morse_n[chr - '0'];
                mc.lng = 5;
            }
            else
            {
                if (chr >= 'A' && chr <= 'Z')  // alphabet coding
                {
                    mc.code = morse_c[chr - 'A'];
                    mc.lng = 1 + (mc.code & 0x03);
                }
                else
                {
                    for (mc.lng = 0, mc.code = 0xFF; mc.lng < 8; mc.lng++)  // characters
                    {
                        if (chr == morse_zn[mc.lng])
                        {
                            mc.code = 0x00;
                            break;
                        }
                    }
                    if (mc.code != 0xFF)  // specials
                    {
                        mc.code = morse_zc[mc.lng];
                        mc.lng = 5 + (mc.code & 0x01);
                    }
                    else
                    {
                        // space or other character - word space
                        mc.lng = 0x00;
                        mc.code = 0xFF;
                    }
                }
            }
        }
    }

    // mark and space transmission in accordance to the code
    if (mc.character_walk == character_walk_run && mc.symbol_walk == symbol_walk_free)
    {

        if (mc.flags.space_rq)
        {
            mc.flags.space_rq = 0;

            // space transmission
            if (mc.lng >= 1)
            {
                mc.symbol = symbol_dot_space;
            }
            else
            {
                mc.symbol = symbol_dash_space;
            }
            mc.symbol_walk = symbol_walk_enter;
        }
        else
        {
            // mark transmission
            if (mc.lng > 0)
            {
                if (mc.code & mc.n)
                {
                    mc.symbol = symbol_dash_mark;
                }
				else
                {
                    mc.symbol = symbol_dot_mark;
                }
                mc.symbol_walk = symbol_walk_enter;
                // make space
                mc.flags.space_rq = 1;
                // increment
                mc.lng--; mc.n >>= 1;
            }
            else
            {
                mc.character_walk = character_walk_free;
                // word space
                if (mc.code == 0xFF)
                {
                    mc.symbol = symbol_word_space;
                    mc.symbol_walk = symbol_walk_enter;
                }
            }
        }
    }
}


/* Store new telegram message to transmit buffer and start sending */
uint32_t morse_send(char *telegram)
{
    uint8_t *c = (uint8_t*)telegram;
    uint8_t ret = 0;

    mc.flags.pa_active = 1;
    ax_set_power_amp(true);
    LL_GPIO_SetPinMode(RDDA_GPIO_Port, RDDA_Pin, LL_GPIO_MODE_OUTPUT);

    // copy the message info buffer with an overrun protection
    while (*c != '\0')
    {
        // push character into buffer
        critical_enter();
        uint32_t i = (uint32_t)(mc.morse_bf_head + 1) % MORSE_BFLEN;
        if (i != mc.morse_bf_tail)
        {
            morse_bf[mc.morse_bf_head] = toupper(*c);
            mc.morse_bf_head = i;
            ret++;
            c++;
        }
        critical_exit();
    }

    return(ret);
}


/* Return true if Morse transmission is still in progress */
bool morse_is_transmit()
{
    return mc.flags.pa_active;
}


/* Return true if telegram FIFO is empty */
bool morse_is_fifo_empty()
{
    return mc.morse_bf_tail == mc.morse_bf_head;
}

/* ISR -----------------------------------------------------------------------*/

/*! External Interrupt Handler - AX5043 RCLK */
void RDCLHandler(void)
{
#ifdef _TEST_INT_RATE_
    static uint8_t k = 0;
#endif

    if (LL_EXTI_IsActiveFlag_0_31(RDCL_EXTI_Line))
    {
        LL_EXTI_ClearFlag_0_31(RDCL_EXTI_Line);

#ifdef _TEST_INT_RATE_
        if (k & 1)
        {
            beep_on();
        }
        else
        {
            beep_off();
        }
        k++;
#else
        if (mc.symbol_walk == symbol_walk_enter)
        {
            // find mark or space length
            mc.symbol_walk = symbol_walk_run;

            switch (mc.symbol)
            {
                case symbol_dot_mark:
                    mc.count = DOT;
                    mc.tone = true;
                    break;
                case symbol_dash_mark:
                    mc.count = DASH;
                    mc.tone = true;
                    break;
                case symbol_dot_space:
                    mc.count = DOT;
                    mc.tone = false;
                    break;
                case symbol_dash_space:
                    mc.count = DASH_SP;
                    mc.tone = false;
                    break;
                case symbol_word_space:
                    mc.count = WORD_SP;
                    mc.tone = false;
                    break;
                default:
                    mc.count = DOT;
                    mc.tone = false;
                    break;
            }
        }
        else if (mc.symbol_walk == symbol_walk_run)
        {
            // marks and space timing
            if (mc.count)
            {
                mc.count--;
            }
            else
            {
                mc.symbol_walk = symbol_walk_free;
                mc.tone = false;
            }
        }
        else
        {
            // do nothing
            mc.tone = false;
        }

        // tone control output
        if (mc.tone)
        {
            set_tone_on();
        }
        else
        {
            set_tone_off();
        }
#endif
    }
}

/* ---------------------------------------------------------------------------*/

/*! @} */

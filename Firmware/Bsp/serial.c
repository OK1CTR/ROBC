/**
 * @file       serial.c
 * @author     OK1CTR
 * @date       Feb 2026
 * @brief      USART serial ring buffer communication module for ports
 *
 * @addtogroup grSerial
 * @{
 */

/* Includes ------------------------------------------------------------------*/

#include <serial.h>
#include <main.h>
#include <critical.h>

/* Exported C functions ------------------------------------------------------*/

#ifdef CONSOLE_SERIAL_1

/* Write a character into the output buffer */
int _write(int file, char *ptr, int len)
{
    __NOP();
    return serial_insert_tx((uint8_t)ptr[0]);
}


/* Wait to all characters are sent */
void _flush()
{
    serial_wait_tx_empty();
}

#endif  /* CONSOLE_SERIAL_1 */

/* Private typedefs ----------------------------------------------------------*/

/* Interface state */
typedef enum
{
    state_reset = 0,
    state_init
} state_e;

/* Private defines -----------------------------------------------------------*/

#define TX_BF_LEN                          64
#define RX_BF_LEN                          16

/* Private macros ------------------------------------------------------------*/

/* RS485 driver enable, switch to transmit */
#define driver_enable() LL_GPIO_SetOutputPin(DE1_GPIO_Port, DE1_Pin)
/* RS485 driver disable, switch to receive */
#define driver_disable() LL_GPIO_ResetOutputPin(DE1_GPIO_Port, DE1_Pin)

/* Private variables ---------------------------------------------------------*/

/* Serial port output ring buffer */
static uint8_t ser_out_bf[TX_BF_LEN];
/* Serial port output buffer write pointer - head */
static uint32_t ser_out_head = 0;
/* Serial port output buffer read pointer - tail */
static uint32_t ser_out_tail = 0;
/* Transmission in progress */
static bool is_transmit = true;

/* Serial port input ring buffer */
static uint8_t ser_in_bf[RX_BF_LEN];
/* Serial port input buffer write pointer - head */
static uint32_t ser_in_head = 0;
/* Serial port input buffer read pointer - tail */
static uint32_t ser_in_tail = 0;
/* Received characters are available */
static bool is_received = true;

/* Interface state */
static volatile state_e state;

/* Functions -----------------------------------------------------------------*/

/* Initialize the serial interface and data buffer */
void serial_init()
{
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (state != state_reset)
    {
        return;
    }

    USART1_CLOCK_EN();

    // transmit pin initialization
    GPIO_InitStruct.Pin = TX1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(TX1_GPIO_Port, &GPIO_InitStruct);

    // receive pin initialization
    GPIO_InitStruct.Pin = RX1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(RX1_GPIO_Port, &GPIO_InitStruct);

    // control pin initialization
    LL_GPIO_ResetOutputPin(DE1_GPIO_Port, DE1_Pin);
    GPIO_InitStruct.Pin = DE1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    LL_GPIO_Init(DE1_GPIO_Port, &GPIO_InitStruct);

    // USART1 initialization
    NVIC_SetPriority(USART1_IRQ_N, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(USART1_IRQ_N);
    USART_InitStruct.BaudRate = USART1_BAUD_RATE;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1_USART, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USART1_USART);
    LL_USART_Enable(USART1_USART);

    state = state_init;

    LL_USART_EnableIT_RXNE(USART1_USART);  // receive interrupt enable
    LL_USART_DisableIT_TXE(USART1_USART);  // transmit interrupt disable
    LL_USART_EnableIT_TC(USART1_USART);  // transmit complete interrupt disable

    // initialize variables - transmitter
    is_transmit = false;
    ser_out_head = 0;
    ser_out_tail = 0;

    // initialize variables - receiver
    is_received = false;
    ser_in_head = 0;
    ser_in_tail = 0;
}


/* Serial interface regular job */
bool serial_job()
{
    return is_transmit | !LL_USART_IsActiveFlag_TC(USART1_USART);
}


/* Insert one character into the transmit data buffer */
int serial_insert_tx(uint8_t c)
{
    int i, ret = 0;

    critical_enter();
    i = (uint32_t)(ser_out_head + 1) % (TX_BF_LEN);
    if (i != ser_out_tail)
    {
        ser_out_bf[ser_out_head] = c;
        ser_out_head = i;
        ret = 1;
    }
    critical_exit();

    // start the transmission, if not running
    if (ret && !is_transmit)
    {
        serial_transmit();
    }
    return ret;
}


/* Wait for empty TX buffer */
void serial_wait_tx_empty()
{
    while (is_transmit || !LL_USART_IsActiveFlag_TXE(USART1_USART))
    {
        __NOP();
    }
}


/* Are any characters in the RX buffer */
bool serial_is_rx_not_empty()
{
    return is_received;
}


/* Pull a character from the RX buffer */
char serial_pull_rx()
{
    uint8_t c;

    critical_enter();
    if (ser_in_head != ser_in_tail)
    {
        c = ser_in_bf[ser_in_tail];
        ser_in_tail = (uint32_t)(ser_in_tail + 1) % (RX_BF_LEN);
    }
    else
    {
        c = 0;
    }

    if (ser_in_head == ser_in_tail && is_received)
    {
        is_received = false;
    }
    critical_exit();

    return (char)c;
}


/* Transmit next character from buffer via serial interface */
void serial_transmit()
{
    if (state != state_init)
    {
        // discard everything
        is_transmit = false;
        ser_out_head = 0;
        ser_out_tail = 0;
        return;
    }

    is_transmit = true;
    driver_enable();

    critical_enter();
    uint8_t c = ser_out_bf[ser_out_tail];
    ser_out_tail = (uint32_t)(ser_out_tail + 1) % (TX_BF_LEN);
    critical_exit();

    LL_USART_TransmitData8(USART1_USART, c);
    LL_USART_EnableIT_TXE(USART1_USART);
}

/* ISR -----------------------------------------------------------------------*/

void USART1_IRQ_HANDLER()
{
    // character transmit handler
    if (LL_USART_IsActiveFlag_TXE(USART1_USART))
    {
        if (ser_out_head != ser_out_tail)
        {
            serial_transmit();
            is_transmit = true;  // still set
        } else {
            LL_USART_DisableIT_TXE(USART1_USART);
            is_transmit = false;
        }
    }

    // character receive handler
    if (LL_USART_IsActiveFlag_RXNE(USART1_USART))
    {
        uint32_t i;
        bool ret = false;

        critical_enter();
        i = (uint32_t)(ser_in_head + 1) % (RX_BF_LEN);
        if (i != ser_in_tail)
        {
            ser_in_bf[ser_in_head] = LL_USART_ReceiveData8(USART1_USART);
            ser_in_head = i;
            is_received = true;
            ret = true;
        }
        critical_exit();

        if (!ret)
        {
            LL_USART_ClearFlag_RXNE(USART1_USART);  // just discard received character
        }
    }

    // end of transmission
    if (LL_USART_IsActiveFlag_TC(USART1_USART))
    {
        driver_disable();
    }
}

/* ---------------------------------------------------------------------------*/

/** @} */

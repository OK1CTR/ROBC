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
#include <stdio.h>
#include <critical.h>

/* Exported C functions ------------------------------------------------------*/

#ifdef CONSOLE_SERIAL_1

/* Write a character into the output buffer */
int _write(int file, char *data, int len)
{
    return serial_insert_tx((uint8_t)data[0]);
    return len;
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

/*! UART private variables */
typedef struct
{
    uint32_t index;                       ///> serial port index
    volatile bool is_transmit;            ///> transmission in progress
    volatile uint32_t ser_out_head;       ///> output buffer write pointer - head
    volatile uint32_t ser_out_tail;       ///> output buffer read pointer - tail
    volatile bool is_received;            ///> received characters are available
    volatile uint32_t ser_in_head;        ///> input buffer write pointer - head
    volatile uint32_t ser_in_tail;        ///> input buffer read pointer - tail
    uint32_t ser_out_len;                 ///> output ring buffer length
    uint8_t *ser_out_bf;                  ///> output ring buffer pointer
    uint32_t ser_in_len;                  ///> input ring buffer length
    uint8_t *ser_in_bf;                   ///> input ring buffer pointer
} uart_context_t;

/* Private defines -----------------------------------------------------------*/

/*! UART1 transmit data buffer length */
#define TX1_BF_LEN                          64
/*! UART1 receive data buffer length */
#define RX1_BF_LEN                          16
/*! UART2 transmit data buffer length */
#define TX2_BF_LEN                          64
/*! UART2 receive data buffer length */
#define RX2_BF_LEN                          16

/* Private macros ------------------------------------------------------------*/

/*! RS485 driver 1 enable, switch to transmit */
#define driver1_enable() LL_GPIO_SetOutputPin(DE1_GPIO_Port, DE1_Pin)
/*! RS485 driver 1 disable, switch to receive */
#define driver1_disable() LL_GPIO_ResetOutputPin(DE1_GPIO_Port, DE1_Pin)
/*! RS485 driver 2 enable, switch to transmit */
#define driver2_enable() LL_GPIO_SetOutputPin(DE2_GPIO_Port, DE2_Pin)
/*! RS485 driver 2 disable, switch to receive */
#define driver2_disable() LL_GPIO_ResetOutputPin(DE2_GPIO_Port, DE2_Pin)

/* Private variables ---------------------------------------------------------*/

/*! UART1 private variables */
static uart_context_t us1 = {0};
/*! UART2 private variables */
static uart_context_t us2 = {0};
/*! UART1 output ring buffer */
uint8_t ser_out_bf1[TX1_BF_LEN];
/*! UART1 input ring buffer */
uint8_t ser_in_bf1[RX1_BF_LEN];
/*! UART2 output ring buffer */
uint8_t ser_out_bf2[TX2_BF_LEN];
/*! UART2 input ring buffer */
uint8_t ser_in_bf2[RX2_BF_LEN];
/*! Interface state */
static state_e state;

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
    USART2_CLOCK_EN();

    // transmit pin initialization
    GPIO_InitStruct.Pin = TX1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(TX1_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = TX2_Pin;
    LL_GPIO_Init(TX2_GPIO_Port, &GPIO_InitStruct);

    // receive pin initialization
    GPIO_InitStruct.Pin = RX1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(RX1_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RX2_Pin;
    LL_GPIO_Init(RX2_GPIO_Port, &GPIO_InitStruct);

    // control pin initialization
    LL_GPIO_ResetOutputPin(DE1_GPIO_Port, DE1_Pin);
    LL_GPIO_ResetOutputPin(DE2_GPIO_Port, DE2_Pin);
    GPIO_InitStruct.Pin = DE1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    LL_GPIO_Init(DE1_GPIO_Port, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = DE2_Pin;
    LL_GPIO_Init(DE2_GPIO_Port, &GPIO_InitStruct);

    // USART1 initialization
    NVIC_SetPriority(USART1_IRQ_N, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(USART2_IRQ_N, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    USART_InitStruct.BaudRate = USART1_BAUD_RATE;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART1_USART, &USART_InitStruct);
    USART_InitStruct.BaudRate = USART2_BAUD_RATE;
    LL_USART_Init(USART2_USART, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USART1_USART);
    LL_USART_ConfigAsyncMode(USART2_USART);
    LL_USART_Enable(USART1_USART);
    LL_USART_Enable(USART2_USART);

    // initialize variables
    us1.index = 1;
    us2.index = 2;

    // initialize variables - transmitter
    us1.ser_out_bf = ser_out_bf1;
    us2.ser_out_bf = ser_out_bf2;
    us1.ser_out_len = TX1_BF_LEN;
    us2.ser_out_len = TX2_BF_LEN;
    us1.is_transmit = false;
    us2.is_transmit = false;
    us1.ser_out_head = 0;
    us2.ser_out_head = 0;
    us1.ser_out_tail = 0;
    us2.ser_out_tail = 0;

    // initialize variables - receiver
    us1.ser_in_bf = ser_in_bf1;
    us2.ser_in_bf = ser_in_bf2;
    us1.ser_in_len = RX1_BF_LEN;
    us2.ser_in_len = RX2_BF_LEN;
    us1.is_received = false;
    us2.is_received = false;
    us1.ser_in_head = 0;
    us2.ser_in_head = 0;
    us1.ser_in_tail = 0;
    us2.ser_in_tail = 0;

    // receive interrupt enable
    LL_USART_EnableIT_RXNE(USART1_USART);
    LL_USART_EnableIT_RXNE(USART2_USART);

    // transmit interrupt disable
    LL_USART_DisableIT_TXE(USART1_USART);
    LL_USART_DisableIT_TXE(USART2_USART);

    // transmit complete interrupt disable
    LL_USART_DisableIT_TC(USART1_USART);
    LL_USART_DisableIT_TC(USART2_USART);

    NVIC_EnableIRQ(USART1_IRQ_N);
    NVIC_EnableIRQ(USART2_IRQ_N);

    state = state_init;
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
        LL_USART_EnableIT_TC(USART1_USART);
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
        LL_USART_DisableIT_TC(USART1_USART);
        driver_disable();
    }
}

/* ---------------------------------------------------------------------------*/

/** @} */

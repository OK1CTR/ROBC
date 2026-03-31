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
    return serial_insert_tx(serial1, (uint8_t)data[0]);
    return len;
}


/* Wait to all characters are sent */
void _flush()
{
    serial_wait_tx_empty(serial1);
}

#endif  /* CONSOLE_SERIAL_1 */

/* Private typedefs ----------------------------------------------------------*/

/* Interface state */
typedef enum
{
    state_reset = 0,
    state_init
} state_e;

/*! UART context structure */
typedef struct
{
    USART_TypeDef *USARTx;                ///> serial port address offset
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

/*! UART private variables */
static uart_context_t uc[2] = {0};

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

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Transmit next character from buffer via serial interface
 * @param serial Serial port selection
 */
static void serial_transmit(serial_e ser);

/**
 * @brief Serial port ISR function
 * @param serial Serial port selection
 */
static void serial_isr(serial_e ser);

/* Functions -----------------------------------------------------------------*/

/* Initialize the serial interface and data buffer */
void serial_init()
{
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

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
    uc[1].USARTx = USART1_USART;
    uc[2].USARTx = USART2_USART;

    // initialize variables - transmitter
    uc[1].ser_out_bf = ser_out_bf1;
    uc[2].ser_out_bf = ser_out_bf2;
    uc[1].ser_out_len = TX1_BF_LEN;
    uc[2].ser_out_len = TX2_BF_LEN;
    uc[1].is_transmit = false;
    uc[2].is_transmit = false;
    uc[1].ser_out_head = 0;
    uc[2].ser_out_head = 0;
    uc[1].ser_out_tail = 0;
    uc[2].ser_out_tail = 0;

    // initialize variables - receiver
    uc[1].ser_in_bf = ser_in_bf1;
    uc[2].ser_in_bf = ser_in_bf2;
    uc[1].ser_in_len = RX1_BF_LEN;
    uc[2].ser_in_len = RX2_BF_LEN;
    uc[1].is_received = false;
    uc[2].is_received = false;
    uc[1].ser_in_head = 0;
    uc[2].ser_in_head = 0;
    uc[1].ser_in_tail = 0;
    uc[2].ser_in_tail = 0;

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
    return false;
}


/* Insert one character into the transmit data buffer */
int serial_insert_tx(serial_e ser, uint8_t c)
{
    int i, ret = 0;
    uart_context_t *us = &uc[(uint32_t)ser];

    // push character into output buffer
    critical_enter();
    i = (uint32_t)(us->ser_out_head + 1) % us->ser_out_len;
    if (i != us->ser_out_tail)
    {
        us->ser_out_bf[us->ser_out_head] = c;
        us->ser_out_head = i;
        ret = 1;
    }
    critical_exit();

    // start the transmission, if not running
    if (ret && !us->is_transmit)
    {
        serial_transmit(ser);
    }
    return ret;
}


/* Wait for empty TX buffer */
void serial_wait_tx_empty(serial_e ser)
{
    uart_context_t *us = &uc[(uint32_t)ser];

    __DSB();
    while (us->is_transmit || !LL_USART_IsActiveFlag_TXE(us->USARTx))
    {
    }
}


/* Are any characters in the RX buffer */
bool serial_is_rx_not_empty(serial_e ser)
{
    uart_context_t *us = &uc[(uint32_t)ser];
    return us->is_received;
}


/* Pull a character from the RX buffer */
char serial_pull_rx(serial_e ser)
{
    uint8_t c;
    uart_context_t *us = &uc[(uint32_t)ser];

    // pull a character from the input buffer
    critical_enter();
    if (us->ser_in_head != us->ser_in_tail)
    {
        c = us->ser_in_bf[us->ser_in_tail];
        us->ser_in_tail = (uint32_t)(us->ser_in_tail + 1) % us->ser_in_len;
    }
    else
    {
        c = 0;
    }

    if (us->ser_in_head == us->ser_in_tail && us->is_received)
    {
        us->is_received = false;
    }
    critical_exit();

    return (char)c;
}

/* Private functions ---------------------------------------------------------*/

/* Transmit next character from buffer via serial interface */
static void serial_transmit(serial_e ser)
{
    uart_context_t *us = &uc[(uint32_t)ser];

    if (state != state_init)
    {
        // discard everything
        us->is_transmit = false;
        us->ser_out_head = 0;
        us->ser_out_tail = 0;
        return;
    }

    // transmit state
    us->is_transmit = true;
    switch (ser)
    {
        case serial1: driver1_enable(); break;
        case serial2: driver2_enable(); break;
        default: break;
    }

    // pull character from output buffer
    critical_enter();
    uint8_t c = us->ser_out_bf[us->ser_out_tail];
    us->ser_out_tail = (uint32_t)(us->ser_out_tail + 1) % us->ser_out_len;
    critical_exit();

    LL_USART_TransmitData8(us->USARTx, c);
    LL_USART_EnableIT_TXE(us->USARTx);
}


/* Serial port ISR function */
static void serial_isr(serial_e ser)
{
    uart_context_t *us = &uc[(uint32_t)ser];

    // character transmit handler
    if (LL_USART_IsActiveFlag_TXE(us->USARTx))
    {
        LL_USART_EnableIT_TC(us->USARTx);
        if (us->ser_out_head != us->ser_out_tail)
        {
            serial_transmit(ser);
            us->is_transmit = true;  // still set
        } else {
            LL_USART_DisableIT_TXE(us->USARTx);
            us->is_transmit = false;
        }
    }

    // character receive handler
    if (LL_USART_IsActiveFlag_RXNE(us->USARTx))
    {
        uint32_t i;
        bool ret = false;

        // receive character and push it into the receive buffer
        critical_enter();
        i = (uint32_t)(us->ser_in_head + 1) % us->ser_in_len;
        if (i != us->ser_in_tail)
        {
            us->ser_in_bf[us->ser_in_head] = LL_USART_ReceiveData8(us->USARTx);
            us->ser_in_head = i;
            us->is_received = true;
            ret = true;
        }
        critical_exit();

        if (!ret)
        {
            LL_USART_ClearFlag_RXNE(us->USARTx);  // just discard received character
        }
    }

    // end of transmission
    if (LL_USART_IsActiveFlag_TC(us->USARTx))
    {
        LL_USART_DisableIT_TC(us->USARTx);
        switch (ser)
        {
            case serial1: driver1_disable(); break;
            case serial2: driver2_disable(); break;
            default: break;
        }
    }
}

/* ISR -----------------------------------------------------------------------*/

void USART1_IRQ_HANDLER()
{
    serial_isr(serial1);
}

void USART2_IRQ_HANDLER()
{
    serial_isr(serial2);
}

/* ---------------------------------------------------------------------------*/

/** @} */

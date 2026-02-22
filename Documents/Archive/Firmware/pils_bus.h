/*!
 * \addtogroup PilsBus Bus
 * \brief PilsenCUBE PilsenCUBE Satellite Communication Bus Driver
 * @{
 */
 
/*!
 * \file    pils_bus.h
 * \brief   PilsenCUBE satellite communication on-board bus driver header
 * \author  OK1CTR
 * \version 1.0
 * \date    17.07.2018
 */


#ifndef _PILS_BUS_H_
#define _PILS_BUS_H_


// PilsenCUBE bus transfer terminator
#define PILS_MSG_TERM 0xAA

/*! @name USART transmitter state request codes
 *  @{
 */
//! Request to new message, transmitter idle
#define US_TXSR_IDLE      0x00
//! Request to send 2nd byte - command
#define US_TXSR_COMMAND   0x01
//! Request to send next byte - high nibble od data as ascii character
#define US_TXSR_DATAH     0x02
//! Request to send next byte - low nibble od data as ascii character
#define US_TXSR_DATAL     0x03
//! Request to send next byte - high nibble od sum as ascii character
#define US_TXSR_SUMH      0x04
//! Request to send next byte - low nibble od sum as ascii character
#define US_TXSR_SUML      0x05
//! Request to send next byte - termination
#define US_TXSR_TERM      0x06
//! Request to deactivate the transmitter
#define US_TXSR_CLOSE     0x07
/*! @} */


/*! @name USART receiver state codes
 *  @{
 */
//! Receiver stopped
#define US_RX_STOP        0x00
//! The receiver is idle
#define US_RX_IDLE        0x01
//! The receiver was addressed
#define US_RX_ADDRESSED   0x02
//! The command was received
#define US_RX_COMMAND     0x03
//! A high data byte received
#define US_RX_DATAH       0x04
//! Received message finished ok
#define US_RX_OK          0x05
//! Received message finished with error
#define US_RX_ERROR       0x06
/*! @} */


//! RS485 PilsenCUBE bus driver enable control, channel 1 - driver ON
#define usart1_de_on()  GPIOA->BSRR = (uint32_t) (0x00000001L << 11)
//! RS485 PilsenCUBE bus driver enable control, channel 1 - driver OFF
#define usart1_de_off() GPIOA->BRR  = (uint32_t) (0x00000001L << 11)
//! RS485 PilsenCUBE bus driver enable control, channel 2 - driver ON
#define usart2_de_on()  GPIOA->BSRR = (uint32_t) (0x00000001L << 1)
//! RS485 PilsenCUBE bus driver enable control, channel 2 - driver OFF
#define usart2_de_off() GPIOA->BRR  = (uint32_t) (0x00000001L << 1)


//! State request of the USART1 transmitter
extern volatile uint8_t us1_tx_str;
//! State request of the USART2 transmitter
extern volatile uint8_t us2_tx_str;
//! State of the USART1 receiver
extern volatile uint8_t us1_rx_st;
//! State of the USART2 receiver
extern volatile uint8_t us2_rx_st;
//! Received command from device on USART1
extern volatile uint8_t us1_rx_cmd;
//! Received command from device on USART2
extern volatile uint8_t us2_rx_cmd;
//! Number of bytes received via USART1
extern volatile uint8_t us1_nrx;
//! Number of bytes receivec via USART2
extern volatile uint8_t us2_nrx;

//! PilsenCUBE communication bus receive buffer, channel 1
extern uint8_t pilbus1_rxbf[PILS_RXBF1_SIZE];
//! PilsenCUBE communication bus receive buffer, channel 2
extern uint8_t pilbus2_rxbf[PILS_RXBF2_SIZE];


/*! \brief Initialize both channel USART&RS485 based PilsenCUBE bus transceivers.
 *  \details Reinitialize GPIO pins, HW reset and initialization the SPI1 peripherial and RX buffer.
 */
extern void pilbus_init(void);

/*! \brief Send message on given  USART channel.
 *  \param ch Channel number, 1 or 2 for USART 1 and 2 respecitvely.
 *  \param adr Destination OBU address.
 *  \param cmd Command for the OBU.
 *  \param n Number of payload bytes.
 *  \param *msg Pointer to payload buffer.
 */
extern void pilbus_send(uint8_t ch, uint8_t adr, uint8_t cmd, uint8_t n, uint8_t *msg);

/*! \brief Receiver buffer and state machine reinitialization.
 *  \param ch Channel number, 1 or 2 for USART 1 and 2 respecitvely.
 */
extern void pilbus_rxreinit(uint8_t ch);

#endif

/*! @} */

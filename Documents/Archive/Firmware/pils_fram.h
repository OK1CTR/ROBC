/*!
 * \addtogroup Fram FRAM
 * \brief PilsenCUBE COM-OBC ferroelectric RAM storage low level driver
 * \note FRAM chip type is CY15B104Q organized by 512 k * 8 bits.
 * @{
 */

/*!
 * \file    pils_fram.h
 * \brief   PilsenCUBE COM-OBC ferroelectric RAM storage low level driver header
 * \author  OK1CTR
 * \version 1.0
 * \date    08.2019
 */
 
#ifndef _FRAM_H_
#define _FRAM_H_


/*! @name FRAM chip command set
 *  @{
 */
//! FRAM write enable command
#define FRAM_CMD_WREN  0x06
//! FRAM write disable command
#define FRAM_CMD_WRDI  0x04
//! FRAM status register read command
#define FRAM_CMD_RDSR  0x05
//! FRAM write status register command
#define FRAM_CMD_WRSR  0x01
//! FRAM read command
#define FRAM_CMD_READ  0x03
//! FRAM fast read command
#define FRAM_CMD_FSTRD 0x0B
//! FRAM write command
#define FRAM_CMD_WRITE 0x02
//! FRAM sleep command
#define FRAM_CMD_SLEEP 0xB9
//! FRAM read ID code command
#define FRAM_CMD_RDID  0x9F
/*! @} */


/*! \brief Initialize the SPI1 interface for FRAM memory array, test and list memory chips.
 *  \details Reinitialize GPIO pins, relocation of SPI1 an JTAG is not touched here. HW reset and initialize the SPI1 peripherial.
 *  \param chips Specifiy what chip position to chechk using bits 0, 1, 2, 3.
 *  \return bit 0, 1, 2, 3 = 1 means FRAM chip 0, 1, 2, 3 is available.
 *  \note Configure the SPI relocation and switch JTAG off before. GPIO block is not HW reset.
 *  \note SPI1 clock division ratio is fixed by a constant in config.h file.
 *  \note Verion 2 of ROBC board don't have an address lines. Returns 1, if FRAM chip is detected or 0 othervise.
 */
extern uint8_t fram_init(uint8_t chips);

/*! \brief Switch the SPI1 clock off.
 *  \details Useful for resolving internal STM32 collisions and for the power saving.
 */
extern void fram_suspend(void);

/*! \brief Select the requested FRAM memory bank (FRAM chip) and produces needed CS signal waveform.
 *  \details The chips are selected by the 1 / 4 decoder. Selecting of chip deselects other three ones.
 *  \note The last state of selection is maintained. If the same chip is selected, a pulse is generated.
 *  \param bank The number 0..3 of the bank (FRAM chip) to be selected.
 *  \return The final selection state 0..3.
 */
extern uint8_t fram_bank_cs(uint8_t bank);

/*! \brief Set the block write protection in selected FRAM chip.
 *  \details The FRAM Status register is unblocked for writing, write protection bits are modified, then the register is blocked and read back.
 *  \param bank The number 0..3 of the bank (FRAM chip) to write to.
 *  \return Zero in case of success, one otherwise.
 */
extern uint8_t fram_block_prot(uint8_t bank, uint8_t protection);

/*! \brief Write the buffer contents to the FRAM memory
 *  \details The FRAM Status register is unblocked for writing and data are written.
 *  \param bank The number 0..3 of the bank (FRAM chip) to write to.
 *  \param address Address of the first byte written.
 *  \param count Number of bytes to write.
 *  \param *data Pointer to the data buffer.
 */
extern void fram_write(uint8_t bank, uint32_t address, uint32_t count, uint8_t *data);

/*! \brief Read the FRAM memory data and store it into the buffer
 *  \param bank The number 0..3 of the bank (FRAM chip) to read from.
 *  \param address Address of the first byte read.
 *  \param count Number of bytes to read.
 *  \param *data Pointer to the data buffer.
 */
extern void fram_read(uint8_t bank, uint32_t address, uint32_t count, uint8_t *data);

/*! \brief Simple SPI1 byte exchange using status register polling.
 *  \details End of transmission is identified by a SPI1 status registers. The program wait until the operation is finished.
 *  \param data The data to be written or zero.
 *  \return The read data.
 */
extern uint8_t spi1_poll(uint8_t data);


#endif

/*! @} */

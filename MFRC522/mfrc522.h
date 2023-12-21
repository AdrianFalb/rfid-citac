/*
 * mfrc522.h
 *
 *  Created on: Nov 26, 2023
 *      Author: Adrian Falb
 */

// An RFID system consists of an RFID reader known as a Proximity Coupling device (PCD)
// and RFID tags known as Proximity Integrated Circuit Cards (PICC).

#ifndef MFRC522_H_
#define MFRC522_H_

/* Includes */
#include "main.h"
#include "spi.h"

//Maximum length of the array
#define MAX_LEN 16

/* MFRC522 Command word -------------------------	*/
#define PCD_IDLE              				0x00    // No action; Cancels current command execution
#define PCD_MEM				  				0x01	// Stores 25 bytes into the internal buffer
#define PCD_GENERATE_RANDOM_ID				0x02	// Generates a 10-byte random ID number
#define PCD_CALC_CRC		    			0x03	// Activates the CRC coprocessor or performs a self test
#define PCD_TRANSMIT						0x04	// Transmits data from the FIFO buffer
#define PCD_NO_CMD_CHANGE	  				0x07 	// No command change, can be used to modify the COMMAND_REG register bits without affecting the command
#define PCD_RECEIVE        					0x08	// Activates the receiver circuits
#define PCD_TRANSCEIVE						0x0C   	// Transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
#define PCD_MF_AUTHENT						0x0E	// Performs the MIFARE standard authentication as a reader
#define PCD_SOFT_RESET		  				0x0F	// Resets the MFRC522

/* The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4) */
#define	PICC_CMD_REQA						0x26	// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
#define	PICC_CMD_WUPA						0x52	// Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
#define	PICC_CMD_CT							0x88	// Cascade Tag. Not really a command, but used during anti collision.
#define	PICC_CMD_SEL_CL1					0x93	// Anti collision/Select, Cascade Level 1
#define	PICC_CMD_SEL_CL2					0x95	// Anti collision/Select, Cascade Level 2
#define	PICC_CMD_SEL_CL3					0x97	// Anti collision/Select, Cascade Level 3
#define	PICC_CMD_HLTA						0x50	// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
#define	PICC_CMD_RATS           			0xE0    // Request command for Answer To Reset.
		// The commands used for MIFARE Classic (from http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf, Section 9)
		// Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
		// The read/write commands can also be used for MIFARE Ultralight.
#define PICC_CMD_MF_AUTH_KEY_A				0x60	// Perform authentication with Key A
#define	PICC_CMD_MF_AUTH_KEY_B				0x61	// Perform authentication with Key B
#define PICC_CMD_MF_READ					0x30	// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
#define PICC_CMD_MF_WRITE					0xA0	// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
#define	PICC_CMD_MF_DECREMENT				0xC0	// Decrements the contents of a block and stores the result in the internal data register.
#define	PICC_CMD_MF_INCREMENT				0xC1	// Increments the contents of a block and stores the result in the internal data register.
#define	PICC_CMD_MF_RESTORE					0xC2	// Reads the contents of a block into the internal data register.
#define	PICC_CMD_MF_TRANSFER				0xB0	// Writes the contents of the internal data register to a block.
		// The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
		// The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
#define	PICC_CMD_UL_WRITE					0xA2	// Writes one 4 byte page to the PICC.

/* Return codes ------------------------------- */
#define STATUS_OK  		   	            	0
#define	STATUS_ERROR						1		// Error in communication
#define	STATUS_COLLISION					2		// Collission detected
#define	STATUS_TIMEOUT						3		// Timeout in communication.
#define	STATUS_NO_ROOM						4		// A buffer is not big enough.
#define	STATUS_INTERNAL_ERROR				5		// Internal error in the code. Should not happen ;-)
#define	STATUS_INVALID						6		// Invalid argument.
#define	STATUS_CRC_WRONG					7		// The CRC_A does not match
#define	STATUS_MIFARE_NACK					0xFF	// A MIFARE PICC responded with NAK.

/* MFRC522 Registers -------------------------- */
/* Page 0: Command and status */
#define COMMAND_REG							0x01 	// Starts and stops command execution
#define COM_I_EN_REG						0x02	// Enable and disable interrupt request control bits
#define DIV_I_EN_REG						0x03	// Enable and disable interrupt request control bits
#define COM_IRQ_REG							0x04	// Interrupt request bits
#define DIV_IRQ_REG							0x05	// Interrupt request bits
#define ERROR_REG							0x06	// Error bits showing the error status of the last commande executed
#define STATUS_1_REG						0x07	// Communication status bits
#define STATUS_2_REG						0x08	// Receiver and transmitter status bits
#define FIFO_DATA_REG						0x09	// Input and output of 64 byte FIFO buffer
#define FIFO_LEVEL_REG						0x0A	// Number of bytes stored in the FIFO buffer
#define	WATER_LEVEL_REG						0x0B	// Level for FIFO underflow and overflow warning
#define CONTROL_REG							0x0C	// Miscellaneous control registers
#define BIT_FRAMING_REG						0x0D	// Adjustments for bit-oriented frames
#define COLL_REG							0x0E	// Bit position of the first bit-collision detected on the RF interface

/* Page 1: Command */
#define MODE_REG							0x11	// Defines general modes for transmitting and receiving
#define TX_MODE_REG							0x12	// Defines transmission data rate and framing
#define RX_MODE_REG							0x13	// Defines reception data rate and framing
#define TX_CONTROL_REG						0x14	// Controls the logical behavior of the antenna driver pins TX1 and TX2
#define TX_ASK_REG							0x15	// Controls the setting of the transmission modulation
#define TX_SEL_REG							0x16	// Selects the internal sources for the antenna driver
#define RX_SEL_REG							0x17	// Selects internal receiver settings
#define RX_THRESHOLD_REG					0x18	// Selects thresholds for the bit decoder
#define DEMOD_REG							0x19	// Defines demodulator settings
#define MF_TX_REG							0x1C	// Controls some MIFARE communication transmit parameters
#define MF_RX_REG							0x1D	// Controls some MIFARE communication receive parameters

/* Page 2: Configuration */
#define CRC_RESULT_REG_HIGH					0x21	// Shows the MSB and LSB values of the CRC calculation
#define CRC_RESULT_REG_LOW					0x22	//
#define MOD_WIDTH_REG						0x24	// Controls the ModWidth setting
#define RFC_FG_REG							0x26	// Configures the receiver gain
#define	GS_N_REG							0x27	// Selects the conductance of the antenna driver pins TX1 and TX2 for modulation
#define CWG_SP_REG							0x28	// Defines the conductance of the p-driver output during periods of no modulation
#define MOD_GSP_REG							0x29	// Defines the conductance of the p-driver output during periods of modulation
#define T_MODE_REG							0x2A	// Defines settings for the internal timer
#define T_PRESCALER_REG						0x2B	//
#define T_RELOAD_REG_HIGH					0x2C	// Defines the 16-bit timer reload value
#define T_RELOAD_REG_LOW					0x2D	//
#define T_COUNTER_VAL_REG_HIGH				0x2E	// Shows the 16-bit timer value
#define T_COUNTER_VAL_REG_LOW				0x2F	//

/* Page 3: Test register */
#define VERSION_REG							0x37	// Shows the software version

/* Function definitions ----------------------- */

/////////////////////////////////////////////////////////////////////////////////////
// Functions for manipulating the MFRC522
/////////////////////////////////////////////////////////////////////////////////////
void MFRC522_PCD_Init();
void MFRC522_PCD_SoftReset(void);
void MFRC522_PCD_AntennaOn(void);
void MFRC522_PCD_AntennaOff(void);

/////////////////////////////////////////////////////////////////////////////////////
// Basic interface functions for communicating with the MFRC522
/////////////////////////////////////////////////////////////////////////////////////
void MFRC522_PCD_Write(uint8_t reg_addr, uint8_t value);
void MFRC522_PCD_WriteArray(uint8_t reg_addr, uint8_t *data, uint8_t length);
uint8_t MFRC522_PCD_Read(uint8_t reg_addr);
void MFRC522_PCD_ReadArray(uint8_t reg_addr, uint8_t* data, uint8_t count);
void MFRC522_PCD_SetBitMask(uint8_t reg, uint8_t mask);
void MFRC522_PCD_ClearBitMask(uint8_t reg, uint8_t mask);
uint8_t MFRC522_PCD_CalculateCRC(uint8_t *p_data, uint8_t len, uint8_t *result);

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with PICCs
/////////////////////////////////////////////////////////////////////////////////////
uint8_t MFRC522_PICC_RequestA(uint8_t req_mode, uint8_t *tag_type);
uint8_t MFRC522_PICC_ToCard(uint8_t command, uint8_t *send_data, uint8_t send_len, uint8_t *back_data, uint8_t *back_len);
uint8_t MFRC522_PICC_Anticollision(uint8_t *p_ser_num);

/////////////////////////////////////////////////////////////////////////////////////
// Debug functions
/////////////////////////////////////////////////////////////////////////////////////
void MFRC522_PCD_GetVersion(uint8_t* buf, uint8_t buf_size);

#endif /* MFRC522_H_ */

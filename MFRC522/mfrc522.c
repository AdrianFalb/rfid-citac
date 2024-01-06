/*
 * mfrc522.c
 *
 */

/* Includes */
#include "mfrc522.h"

/* Defines */
#define RC522_CS_GPIO_Port GPIOA
#define RC522_CS_Pin GPIO_PIN_9

/////////////////////////////////////////////////////////////////////////////////////
// Basic interface functions for communicating with the MFRC522
/////////////////////////////////////////////////////////////////////////////////////

/**
  * @brief 	Write a byte to the specified register in the MFRC522 reader/writer IC.
  * @note 	Communication is done through the SPI interface.
  * @param 	reg_addr register address
  * @param 	value byte to write
  * @retval None
  */
void MFRC522_PCD_Write(uint8_t reg_addr, uint8_t value)
{
	// Set the chip select line so we can start transferring
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);

	// Prepare address for write mode
	uint8_t addr = (((reg_addr << 1) & 0x7E));

	HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
	HAL_SPI_Transmit(&hspi1, &value, 1, 500);

	// Clear the select line - release the slave
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief 	Write an array of bytes to the specified register in the MFRC522 reader/write IC.
  * @note 	Communication is done through the SPI interface.
  * @param 	reg_addr register address
  * @param 	p_data data to be written
  * @param	length number of bytes to be written
  * @retval None
  */
void MFRC522_PCD_WriteArray(uint8_t reg_addr, uint8_t *p_data, uint8_t length) {

	// Set the chip select line so we can start transferring
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);

	// Prepare address for write mode
	uint8_t addr = (((reg_addr << 1) & 0x7E));

	HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
	HAL_SPI_Transmit(&hspi1, p_data, length, 500);

	// Clear the select line - release the slave
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief 	Read a byte from the specified register in the MFRC522 reader/writer IC.
  * @note 	Communication is done through the SPI interface.
  * @param 	reg_addr register address
  * @retval value value that was read from the specified register
  */
uint8_t MFRC522_PCD_Read(uint8_t reg_addr) {

	uint8_t value;

	// Prepare address for read mode
	uint8_t addr = (((reg_addr << 1) & 0x7E) | 0x80);

	// Set the select line so we can start transferring - select slave
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, &addr, 1, 500);
	HAL_SPI_Receive(&hspi1, &value, 1, 500);

	// Clear the select line - release slave
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
	return (uint8_t) value;
}

/**
  * @brief 	Read array of bytes from the specified register in the MFRC522 reader/writer IC.
  * @note 	Communication is done through the SPI interface.
  * @param 	reg_addr register address
  * @param	p_data pointer to data buffer for storing the read bytes
  * @param	count number of bytes to read
  * @retval none
  */
void MFRC522_PCD_ReadArray(uint8_t reg_addr, uint8_t* p_data, uint8_t count) {

	// Prepare address for read mode
	uint8_t addr = (((reg_addr << 1) & 0x7E) | 0x80);

	// set the select line so we can start transferring - select slave
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, &addr, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, p_data, count, HAL_MAX_DELAY);

	// Clear the select line - release slave
	HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, GPIO_PIN_SET);
}

/**
  * @brief 	Sets the bits given in mask in specified register.
  * @param 	reg register address
  * @param 	mask the bits to set
  * @retval none
  */
void MFRC522_PCD_SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp;
    tmp = MFRC522_PCD_Read(reg);
    MFRC522_PCD_Write(reg, tmp | mask);  // Set bit mask
}

/**
  * @brief 	Clears the bits given in mask from specified register.
  * @param 	reg register address
  * @param 	mask the bits to clear
  * @retval none
  */
void MFRC522_PCD_ClearBitMask(uint8_t reg, uint8_t mask)
{
	uint8_t tmp;
    tmp = MFRC522_PCD_Read(reg);
    MFRC522_PCD_Write(reg, tmp & (~mask));  // Clear bit mask
}

/**
  * @brief 	Use the CRC coprocessor in the MFRC522 to calculate a CRC_A.
  * @param 	p_data pointer to the data to transfer to the FIFO for CRC calculation
  * @param 	len the number of bytes to transfer
  * @param	result pointer to the result buffer. Result is written low byte first
  * @retval STATUS_OK on success, STATUS_TIMEOUT otherwise.
  */
uint8_t MFRC522_PCD_CalculateCRC(uint8_t *p_data, uint8_t len, uint8_t *result)
{
	uint8_t i, n;

	MFRC522_PCD_Write(COMMAND_REG, PCD_IDLE);			// Stop any active command.
	MFRC522_PCD_Write(DIV_IRQ_REG, 0x04);				// Clear the CRCIRq interrupt request bit
	MFRC522_PCD_Write(FIFO_LEVEL_REG, 0x80);			// FlushBuffer = 1, FIFO initialization

	// Writing data to the FIFO
	for (i = 0; i < len; i++)
	{
		MFRC522_PCD_Write(FIFO_DATA_REG, *(p_data+i));
	}
	MFRC522_PCD_Write(COMMAND_REG, PCD_CALC_CRC); 		// Start calculation

	/* Wait for when CRC calculation is complete */
	i = 0xFF;
	do
	{
		n = MFRC522_PCD_Read(DIV_IRQ_REG);

		if (n & 0x04)
		{
			MFRC522_PCD_Write(COMMAND_REG, PCD_IDLE);	// Stop calculating

			// Read CRC calculation result
			result[0] = MFRC522_PCD_Read(CRC_RESULT_REG_LOW);
			result[1] = MFRC522_PCD_Read(CRC_RESULT_REG_HIGH);
			return STATUS_OK;
		}

		i--;
	}
	while (i != 0);					//CRCIrq = 1

	// Time passed and nothing happened. Communication with the MFRC522 might be down
	return STATUS_TIMEOUT;
}

/////////////////////////////////////////////////////////////////////////////////////
// Functions for manipulating the MFRC522
/////////////////////////////////////////////////////////////////////////////////////

/**
  * @brief 	Initializes the MFRC522 reader/writer IC.
  */
void MFRC522_PCD_Init()
{
	/* Performs a soft reset */
	MFRC522_PCD_SoftReset();

	/* Reset baud rates */
	MFRC522_PCD_Write(TX_MODE_REG, 0x00);
	MFRC522_PCD_Write(RX_MODE_REG, 0x00);
	MFRC522_PCD_Write(MOD_WIDTH_REG, 0x26);

	/* When communicating with a PICC we need a timeout if something goes wrong. */
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	MFRC522_PCD_Write(T_MODE_REG, 0x80);			// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	MFRC522_PCD_Write(T_PRESCALER_REG, 0xA9);		// TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25μs.
	MFRC522_PCD_Write(T_RELOAD_REG_HIGH, 0x03);		// Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
	MFRC522_PCD_Write(T_RELOAD_REG_LOW, 0xE8);

	MFRC522_PCD_Write(TX_ASK_REG, 0x40);			// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	MFRC522_PCD_Write(MODE_REG, 0x3D);				// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)

	/* Turn antenna on */
	MFRC522_PCD_AntennaOn();
}

/**
  * @brief 	Performs a soft reset on the MFRC522 reader/writer IC.
  */
void MFRC522_PCD_SoftReset(void)
{
	MFRC522_PCD_Write(COMMAND_REG, PCD_SOFT_RESET);
}

/**
  * @brief 	Turns the antenna on by enabling pins Tx1RFEn and Tx2RFEn.
  * @note 	After a reset these pins are disabled.
  */
void MFRC522_PCD_AntennaOn(void) {
	MFRC522_PCD_SetBitMask(TX_CONTROL_REG, 0x03);
}

/**
  * @brief 	Turns the antenna off by disabling pins Tx1RFEn and Tx2RFEn.
  * @note	These pins are disabled after a reset automatically.
  */
void MFRC522_PCD_AntennaOff(void) {
	MFRC522_PCD_ClearBitMask(TX_CONTROL_REG, 0x03);
}

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with PICCs
/////////////////////////////////////////////////////////////////////////////////////

/**
  * @brief 	Anti-collision detection, reads selected card's UID.
  * @note	Shows all known firmware versions.
  * @param	p_ser_num returns 4 bytes of the card's serial number, the first 5 bytes for the checksum byte
  * @retval	STATUS_OK on success, STATUS_ERROR otherwise
  */
uint8_t MFRC522_PICC_Anticollision(uint8_t *p_ser_num)
{
	uint8_t status;
	uint8_t i;
	uint8_t ser_num_check = 0;
	uint32_t un_len;

	//ClearBitMask(Status2Reg, 0x08);		//TempSensclear
	//ClearBitMask(CollReg,0x80);			//ValuesAfterColl
	MFRC522_PCD_Write(BIT_FRAMING_REG, 0x00);		//TxLastBists = BitFramingReg[2..0]

	p_ser_num[0] = PICC_CMD_SEL_CL1;
	p_ser_num[1] = 0x20;
	status = MFRC522_PICC_ToCard(PCD_TRANSCEIVE, p_ser_num, 2, p_ser_num, &un_len);

	if (status == STATUS_OK)
	{
		//Check card serial number
		for (i = 0; i < 4; i++)
		{
			ser_num_check ^= p_ser_num[i];
		}
		if (ser_num_check != p_ser_num[i])
		{
			status = STATUS_ERROR;
		}
  }

  return status;
}

/**
  * @brief Transmits a REQuest command, Type A.
  * @note  Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
  * @param req_mode - find cards way
  * @param tag_type - Return Card Type
  *    			0x4400 = Mifare_UltraLight
  *    			0x0400 = Mifare_One(S50)
  *    			0x0200 = Mifare_One(S70)
  *    			0x0800 = Mifare_Pro(X)
  *    			0x4403 = Mifare_DESFire
  * @retval STATUS_OK on success, STATUS_ERROR otherwise.
  */
uint8_t MFRC522_PICC_RequestA(uint8_t req_mode, uint8_t *tag_type)
{
  uint8_t status;
  uint32_t back_bits; // The received data bits

  MFRC522_PCD_Write(BIT_FRAMING_REG, 0x07);   // TxLastBists = BitFramingReg[2..0]

  tag_type[0] = req_mode;

  status = MFRC522_PICC_ToCard(PCD_TRANSCEIVE, tag_type, 1, tag_type, &back_bits);
  if ((status != STATUS_OK) || (back_bits != 0x10)) {
    status = STATUS_ERROR;
  }

  return status;
}

//-----------------------------------------------

/**
  * @brief MFRC522 and ISO14443 card communication
  * @param command MFRC522 command word
  * @param send_data pointer to data sent by MFRC522
  * @param sen_len length of data sent
  * @param back_data pointer to received data,
  * @param back_len return data bit length
  * @retval STATUS_OK on succecss, STATUS_ERROR otherwise
  */
uint8_t MFRC522_PICC_ToCard(uint8_t command, uint8_t *send_data, uint8_t send_len, uint8_t *back_data, uint8_t *back_len)
{
  uint8_t status = STATUS_ERROR;
  uint8_t irq_en = 0x00;
  uint8_t wait_irq = 0x00;
  uint8_t last_bits;
  uint8_t n;
  uint32_t i;

  switch (command)
  {
  	  case PCD_MF_AUTHENT:	// Certification cards close
      {
			irq_en = 0x12;
			wait_irq = 0x10;
			break;
      }
  	  case PCD_TRANSCEIVE:  // Transmit FIFO data
      {
			irq_en = 0x77;
			wait_irq = 0x30;
			break;
      }
  	  default:
  		  break;
  }

  MFRC522_PCD_Write(COM_I_EN_REG, irq_en|0x80); // Interrupt request
  MFRC522_PCD_ClearBitMask(COM_IRQ_REG, 0x80);  // Clear all interrupt request bit
  MFRC522_PCD_SetBitMask(FIFO_LEVEL_REG, 0x80); // FlushBuffer=1, FIFO Initialization

  MFRC522_PCD_Write(COMMAND_REG, PCD_IDLE); // NO action; Cancel the current command

  // Writing data to the FIFO
  for (i =0; i < send_len; i++)
  {
	  MFRC522_PCD_Write(FIFO_DATA_REG, send_data[i]);
  }

  // Execute the command
  MFRC522_PCD_Write(COMMAND_REG, command);
  if (command == PCD_TRANSCEIVE)
  {
    MFRC522_PCD_SetBitMask(BIT_FRAMING_REG, 0x80);      // StartSend=1,transmission of data starts
  }

  // Waiting to receive data to complete
  i = 2000;	// i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms
  do
  {
    // CommIrqReg[7..0]
    // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
    n = MFRC522_PCD_Read(COM_IRQ_REG);
    i--;
  }
  while ((i != 0) && !(n & 0x01) && !(n & wait_irq));

  MFRC522_PCD_ClearBitMask(BIT_FRAMING_REG, 0x80);      // StartSend=0

  if (i != 0)
  {
	  if(!(MFRC522_PCD_Read(ERROR_REG) & 0x1B))  // BufferOvfl Collerr CRCErr ProtecolErr
	  {
		  status = STATUS_OK;
		  if (n & irq_en & 0x01)
		  {
			status = STATUS_ERROR;             // ??
		  }

		  if (command == PCD_TRANSCEIVE)
		  {
			  n = MFRC522_PCD_Read(FIFO_LEVEL_REG);
			  last_bits = MFRC522_PCD_Read(CONTROL_REG) & 0x07;

			  if (last_bits)
			  {
				  *back_len = (n-1)*8 + last_bits;
			  }
			  else
			  {
				  *back_len = n*8;
			  }

			  if (n == 0)
			  {
				  n = 1;
			  }
			  if (n > MAX_LEN)
			  {
				  n = MAX_LEN;
			  }

			  // Reading the received data in FIFO
			  for (i = 0; i < n; i++)
			  {
				  back_data[i] = MFRC522_PCD_Read(FIFO_DATA_REG);
			  }
		  }
	  }
	  else
	  {
		  //printf("~~~ buffer overflow, collerr, crcerr, or protecolerr\r\n");
		  status = STATUS_ERROR;
	  }
  }
  else
  {
    //printf("~~~ request timed out\r\n");
  }

  return status;
}

/////////////////////////////////////////////////////////////////////////////////////
// Debug functions
/////////////////////////////////////////////////////////////////////////////////////

/**
  * @brief 	Returns the firmware version of the connected MFRC522 reader/writer IC.
  * @note	Shows all known firmware versions.
  * @param	buf pointer to data buffer to store the returned string.
  * @param	buf_size size of the data buffer
  * @retval	none
  */
void MFRC522_PCD_GetVersion(uint8_t* buf, uint8_t buf_size) {
	uint8_t ver = MFRC522_PCD_Read(VERSION_REG);

	switch(ver)
	{
		case 0x88:
			snprintf(buf, buf_size, "Firmware Version: 0x%x = (clone)", ver);
			//return buf;
			break;
		case 0x90:
			snprintf(buf, buf_size, "Firmware Version: 0x%x = v0.0", ver);
			//return buf;
			break;
		case 0x91:
			snprintf(buf, buf_size, "Firmware Version: 0x%x = v1.0", ver);
			//return buf;
			break;
		case 0x92:
			snprintf(buf, buf_size, "Firmware Version: 0x%x = v2.0", ver);
			//return buf;
			break;
		case 0x12:
			snprintf(buf, buf_size, "Firmware Version: 0x%x = counterfeit chip", ver);
			//return buf;
			break;
		case (0x00 || 0xFF):
			snprintf(buf, buf_size, "WARNING: Communication failure, is the MFRC522 properly connected?");
			//return buf;
			break;
		default:
			snprintf(buf, buf_size, "Firmware Version: 0x%x = unknown", ver);
			//return buf;
			break;
	}
}

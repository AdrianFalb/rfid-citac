/**
 ******************************************************************************
  * @file    user_diskio_spi.c
  * @brief   This file contains the implementation of the user_diskio_spi FatFs
  *          driver.
  ******************************************************************************
  * Portions copyright (C) 2014, ChaN, all rights reserved.
  * Portions copyright (C) 2017, kiwih, all rights reserved.
  *
  * This software is a free software and there is NO WARRANTY.
  * No restriction on use. You can use, modify and redistribute it for
  * personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
  * Redistributions of source code must retain the above copyright notice.
  *
  ******************************************************************************
  */

//This code was ported by kiwih from a copywrited (C) library written by ChaN
//available at http://elm-chan.org/fsw/ff/ffsample.zip
//(text at http://elm-chan.org/fsw/ff/00index_e.html)

//This file provides the FatFs driver functions and SPI code required to manage
//an SPI-connected MMC or compatible SD card with FAT

//It is designed to be wrapped by a cubemx generated user_diskio.c file.

#include "stm32f3xx_hal.h" /* Provide the low-level HAL functions */
#include "user_diskio_spi.h"

//Make sure you set #define SD_SPI_HANDLE as some hspix in main.h
//Make sure you set #define SD_CS_GPIO_Port as some GPIO port in main.h
//Make sure you set #define SD_CS_Pin as some GPIO pin in main.h
extern SPI_HandleTypeDef SD_SPI_HANDLE;

/* Function prototypes */

//(Note that the _256 is used as a mask to clear the prescalar bits as it provides binary 111 in the correct position)
#define FCLK_SLOW() { MODIFY_REG(SD_SPI_HANDLE.Instance->CR1, SPI_BAUDRATEPRESCALER_256, SPI_BAUDRATEPRESCALER_128); }	/* Set SCLK = slow, approx 280 KBits/s*/
#define FCLK_FAST() { MODIFY_REG(SD_SPI_HANDLE.Instance->CR1, SPI_BAUDRATEPRESCALER_256, SPI_BAUDRATEPRESCALER_8); }	/* Set SCLK = fast, approx 4.5 MBits/s */

#define CS_HIGH()	{HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);}
#define CS_LOW()	{HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);}

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* MMC/SD command */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */

static volatile
DSTATUS Stat = STA_NOINIT;	/* Physical drive status */


static
BYTE CardType;			/* Card type flags */

uint32_t spiTimerTickStart;
uint32_t spiTimerTickDelay;

void SPI_Timer_On(uint32_t waitTicks) {
    spiTimerTickStart = HAL_GetTick();
    spiTimerTickDelay = waitTicks;
}

uint8_t SPI_Timer_Status() {
    return ((HAL_GetTick() - spiTimerTickStart) < spiTimerTickDelay);
}

/*-----------------------------------------------------------------------*/
/* SPI controls (Platform dependent)                                     */
/*-----------------------------------------------------------------------*/

/* Exchange a byte */
static BYTE transmitByte (
	BYTE dat	/* Data to send */
)
{
	BYTE rxDat;
    HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, &dat, &rxDat, 1, 50);
    return rxDat;
}


/* Receive multiple byte */
static void recieveMultiByte (
	BYTE *buff,		/* Pointer to data buffer */
	UINT btr		/* Number of bytes to receive (even number) */
)
{
	for(UINT i=0; i<btr; i++) {
		*(buff+i) = transmitByte(0xFF);
	}
}


#if _USE_WRITE
/* Send multiple byte */
static void transmitMultiByte (
	const BYTE *buff,	/* Pointer to the data */
	UINT btx			/* Number of bytes to send (even number) */
)
{
	HAL_SPI_Transmit(&SD_SPI_HANDLE, buff, btx, HAL_MAX_DELAY);
}
#endif


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static int waitForSDReadyState (UINT wt)
{
	BYTE d;
	//waitForSDReadyState needs its own timer, unfortunately, so it can't use the
	//spi_timer functions
	uint32_t waitSpiTimerTickStart;
	uint32_t waitSpiTimerTickDelay;

	waitSpiTimerTickStart = HAL_GetTick();
	waitSpiTimerTickDelay = (uint32_t)wt;
	do {
		d = transmitByte(0xFF);
		/* This loop takes a time. Insert rot_rdq() here for multitask envilonment. */
	} while (d != 0xFF && ((HAL_GetTick() - waitSpiTimerTickStart) < waitSpiTimerTickDelay));	/* Wait for card goes ready or timeout */

	return (d == 0xFF) ? 1 : 0;
}



/*-----------------------------------------------------------------------*/
/* SPI_deselectSlave card and release SPI                                         */
/*-----------------------------------------------------------------------*/

static void SPI_deselectSlave (void)
{
	CS_HIGH();		/* Set CS# high */
	transmitByte(0xFF);	/* Dummy clock (force DO hi-z for multiple slave SPI) */

}



/*-----------------------------------------------------------------------*/
/* Select card and wait for ready                                        */
/*-----------------------------------------------------------------------*/

static int SPI_selectSlave (void)	/* 1:OK, 0:Timeout */
{
	CS_LOW();		/* Set CS# low */
	transmitByte(0xFF);	/* Dummy clock (force DO enabled) */
	if (waitForSDReadyState(500)) return 1;	/* Wait for card ready */

	SPI_deselectSlave();
	return 0;	/* Timeout */
}

static void SPI_selectSlavenowait (void)
{
	transmitByte(0xFF);	/* Dummy clock (force DO enabled) */
	CS_LOW();		/* Set CS# low */
	transmitByte(0xFF);	/* Dummy clock (force DO enabled) */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the MMC                                    */
/*-----------------------------------------------------------------------*/

static int recieveDatablock (	/* 1:OK, 0:Error */
	BYTE *buff,			/* Data buffer */
	UINT btr			/* Data block length (byte) */
	)
{
	BYTE token;


	SPI_Timer_On(200);
	do {							/* Wait for DataStart token in timeout of 200ms */
		token = transmitByte(0xFF);
		/* This loop will take a time. Insert rot_rdq() here for multitask envilonment. */
	} while ((token == 0xFF) && SPI_Timer_Status());
	if(token != 0xFE) return 0;		/* Function fails if invalid DataStart token or timeout */

	recieveMultiByte(buff, btr);		/* Store trailing data to the buffer */
	transmitByte(0xFF); transmitByte(0xFF);			/* Discard CRC */

	return 1;						/* Function succeeded */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to the MMC                                         */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
static int transmitDatablock (	/* 1:OK, 0:Failed */
	const BYTE *buff,	/* Ponter to 512 byte data to be sent */
	BYTE token			/* Token */
)
{
	BYTE resp;


	if (!waitForSDReadyState(500)) return 0;		/* Wait for card ready */

	transmitByte(token);					/* Send token */
	if (token != 0xFD) {				/* Send data if token is other than StopTran */
		transmitMultiByte(buff, 512);		/* Data */
		transmitByte(0xFF); transmitByte(0xFF);	/* Dummy CRC */

		resp = transmitByte(0xFF);				/* Receive data resp */
		if ((resp & 0x1F) != 0x05) return 0;	/* Function fails if the data packet was not accepted */
	}
	return 1;
}
#endif


/*-----------------------------------------------------------------------*/
/* Send a command packet to the MMC                                      */
/*-----------------------------------------------------------------------*/

static BYTE sendCommandToSD_init ()
{
	BYTE n, res;
	BYTE cmd = CMD0;
	DWORD arg = 0;

	SPI_selectSlavenowait();

	/* Send command packet */
	transmitByte(0x40 | cmd);				/* Start + command index */
	transmitByte((BYTE)(arg >> 24));		/* Argument[31..24] */
	transmitByte((BYTE)(arg >> 16));		/* Argument[23..16] */
	transmitByte((BYTE)(arg >> 8));			/* Argument[15..8] */
	transmitByte((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	transmitByte(n);

	/* Receive command resp */
	n = 10;								/* Wait for response (10 bytes max) */
	do {
		res = transmitByte(0xFF);
	} while ((res & 0x80) && --n);

	return res;							/* Return received response */
}


static BYTE sendCommandToSD (		/* Return value: R1 resp (bit7==1:Failed to send) */
	BYTE cmd,		/* Command index */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;


	if (cmd & 0x80) {	/* Send a CMD55 prior to ACMD<n> */
		cmd &= 0x7F;
		res = sendCommandToSD(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready except to stop multiple block read */
	if (cmd != CMD12) {
		SPI_deselectSlave();
		if (!SPI_selectSlave()) return 0xFF;
	}

	/* Send command packet */
	transmitByte(0x40 | cmd);				/* Start + command index */
	transmitByte((BYTE)(arg >> 24));		/* Argument[31..24] */
	transmitByte((BYTE)(arg >> 16));		/* Argument[23..16] */
	transmitByte((BYTE)(arg >> 8));			/* Argument[15..8] */
	transmitByte((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	transmitByte(n);

	/* Receive command resp */
	if (cmd == CMD12) transmitByte(0xFF);	/* Diacard following one byte when CMD12 */
	n = 10;								/* Wait for response (10 bytes max) */
	do {
		res = transmitByte(0xFF);
	} while ((res & 0x80) && --n);

	return res;							/* Return received response */
}


/*--------------------------------------------------------------------------

   Public FatFs Functions (wrapped in user_diskio.c)

---------------------------------------------------------------------------*/

//The following functions are defined as inline because they aren't the functions that
//are passed to FatFs - they are wrapped by autogenerated (non-inline) cubemx template
//code.
//If you do not wish to use cubemx, remove the "inline" from these functions here
//and in the associated .h


/*-----------------------------------------------------------------------*/
/* Initialize disk drive                                                 */
/*-----------------------------------------------------------------------*/

inline DSTATUS USER_SPI_initialize (
	BYTE drv		/* Physical drive number (0) */
)
{
	BYTE n, cmd, ty, ocr[4], ret;

	if (drv != 0) return STA_NOINIT;		/* Supports only drive 0 */
	//assume SPI already init init_spi();	/* Initialize SPI */

	if (Stat & STA_NODISK) return Stat;	/* Is card existing in the soket? */

	FCLK_SLOW();
	for (n = 10; n; n--) transmitByte(0xFF);	/* Send 80 dummy clocks */

	sendCommandToSD_init();

	ty = 0;
	ret = sendCommandToSD(CMD0, 0);
	if (ret == 1) {			/* Put the card SPI/Idle state */
		SPI_Timer_On(1000);					/* Initialization timeout = 1 sec */
		if (sendCommandToSD(CMD8, 0x1AA) == 1) {	/* SDv2? */
			for (n = 0; n < 4; n++) ocr[n] = transmitByte(0xFF);	/* Get 32 bit return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* Is the card supports vcc of 2.7-3.6V? */
				while (SPI_Timer_Status() && sendCommandToSD(ACMD41, 1UL << 30)) ;	/* Wait for end of initialization with ACMD41(HCS) */
				if (SPI_Timer_Status() && sendCommandToSD(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = transmitByte(0xFF);
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* Card id SDv2 */
				}
			}
		} else {	/* Not SDv2 card */
			if (sendCommandToSD(ACMD41, 0) <= 1) 	{	/* SDv1 or MMC? */
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 (ACMD41(0)) */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 (CMD1(0)) */
			}
			while (SPI_Timer_Status() && sendCommandToSD(cmd, 0)) ;		/* Wait for end of initialization */
			if (!SPI_Timer_Status() || sendCommandToSD(CMD16, 512) != 0)	/* Set block length: 512 */
				ty = 0;
		}
	}
	CardType = ty;	/* Card type */
	SPI_deselectSlave();

	if (ty) {			/* OK */
		FCLK_FAST();			/* Set fast clock */
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT flag */
	} else {			/* Failed */
		Stat = STA_NOINIT;
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get disk status                                                       */
/*-----------------------------------------------------------------------*/

inline DSTATUS USER_SPI_status (
	BYTE drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only drive 0 */

	return Stat;	/* Return disk status */
}



/*-----------------------------------------------------------------------*/
/* Read sector(s)                                                        */
/*-----------------------------------------------------------------------*/

inline DRESULT USER_SPI_read (
	BYTE drv,		/* Physical drive number (0) */
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	if (drv || !count) return RES_PARERR;		/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA ot BA conversion (byte addressing cards) */

	if (count == 1) {	/* Single sector read */
		if ((sendCommandToSD(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& recieveDatablock(buff, 512)) {
			count = 0;
		}
	}
	else {				/* Multiple sector read */
		if (sendCommandToSD(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!recieveDatablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			sendCommandToSD(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	SPI_deselectSlave();

	return count ? RES_ERROR : RES_OK;	/* Return result */
}



/*-----------------------------------------------------------------------*/
/* Write sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
inline DRESULT USER_SPI_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Ponter to the data to write */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	if (drv || !count) return RES_PARERR;		/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check drive status */
	if (Stat & STA_PROTECT) return RES_WRPRT;	/* Check write protect */

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA ==> BA conversion (byte addressing cards) */

	if (count == 1) {	/* Single sector write */
		if ((sendCommandToSD(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& transmitDatablock(buff, 0xFE)) {
			count = 0;
		}
	}
	else {				/* Multiple sector write */
		if (CardType & CT_SDC) sendCommandToSD(ACMD23, count);	/* Predefine number of sectors */
		if (sendCommandToSD(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!transmitDatablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!transmitDatablock(0, 0xFD)) count = 1;	/* STOP_TRAN token */
		}
	}
	SPI_deselectSlave();

	return count ? RES_ERROR : RES_OK;	/* Return result */
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous drive controls other than data read/write               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
inline DRESULT USER_SPI_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE cmd,		/* Control command code */
	void *buff		/* Pointer to the conrtol data */
)
{
	DRESULT res;
	BYTE n, csd[16];
	DWORD *dp, st, ed, csize;


	if (drv) return RES_PARERR;					/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		if (SPI_selectSlave()) res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
		if ((sendCommandToSD(CMD9, 0) == 0) && recieveDatablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(DWORD*)buff = csize << 10;
			} else {					/* SDC ver 1.XX or MMC ver 3 */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		if (CardType & CT_SD2) {	/* SDC ver 2.00 */
			if (sendCommandToSD(ACMD13, 0) == 0) {	/* Read SD status */
				transmitByte(0xFF);
				if (recieveDatablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) transmitByte(0xFF);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDC ver 1.XX or MMC */
			if ((sendCommandToSD(CMD9, 0) == 0) && recieveDatablock(csd, 16)) {	/* Read CSD */
				if (CardType & CT_SD1) {	/* SDC ver 1.XX */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMC */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case CTRL_TRIM :	/* Erase a block of sectors (used when _USE_ERASE == 1) */
		if (!(CardType & CT_SDC)) break;				/* Check if the card is SDC */
		if (USER_SPI_ioctl(drv, MMC_GET_CSD, csd)) break;	/* Get CSD */
		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card */
		dp = buff; st = dp[0]; ed = dp[1];				/* Load sector block */
		if (!(CardType & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		if (sendCommandToSD(CMD32, st) == 0 && sendCommandToSD(CMD33, ed) == 0 && sendCommandToSD(CMD38, 0) == 0 && waitForSDReadyState(30000)) {	/* Erase sector block */
			res = RES_OK;	/* FatFs does not check result of this command */
		}
		break;

	default:
		res = RES_PARERR;
	}

	SPI_deselectSlave();

	return res;
}
#endif

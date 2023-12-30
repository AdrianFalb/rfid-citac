/*
 * fatfs_wraper_functions.c
 *
 *  Created on: Dec 22, 2023
 *      Author: u
 */
#include "fatfs_wraper_functions.h"


uint8_t openFileForWriting(FIL* fil, char* path)
{
	FRESULT res;

	res = f_open(fil, path, FA_OPEN_ALWAYS | FA_WRITE);

	if(res != FR_OK){
		f_close(fil);
		return 0;
	}
	return 1;
}

uint8_t openFileForReading(FIL* fil, char* path)
{
	FRESULT res;

	res = f_open(fil, path, FA_OPEN_ALWAYS | FA_READ);

	if(res != FR_OK){
		f_close(fil);
		return 0;
	}
	return 1;
}

uint8_t openFileForAppend(FIL* fil, char* path)
{
	FRESULT res;


	res = f_open(fil, path, FA_OPEN_ALWAYS | FA_WRITE);
	if(res == FR_OK){
		res = f_lseek(fil, f_size(fil));

		if(res != FR_OK){
			f_close(fil);
			return 0;
		}
	}
	else{
		f_close(fil);
		return 0;
	}

	return 1;
}

uint32_t calculateTotalCardSpace(FATFS* pfs)
{
	return (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
}

uint32_t calculateFreeCardSpace(FATFS* pfs, DWORD* fre_clust)
{
	return (uint32_t)(*fre_clust * pfs->csize * 0.5);
}

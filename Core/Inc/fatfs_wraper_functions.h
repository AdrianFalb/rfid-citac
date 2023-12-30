/*
 * fatfs_wraper_functions.h
 *
 *  Created on: Dec 22, 2023
 *      Author: u
 */

#ifndef INC_FATFS_WRAPER_FUNCTIONS_H_
#define INC_FATFS_WRAPER_FUNCTIONS_H_

#include "fatfs.h"
#include "spi.h"
#include <string.h>
#include "ff.h"
#include "stm32f3xx_hal.h"
#include <stdint.h>

uint8_t openFileForWriting(FIL* fil, char* path);
uint8_t openFileForReading(FIL* fil, char* path);
uint8_t openFileForAppend(FIL* fil, char* path);

uint32_t calculateTotalCardSpace(FATFS* pfs);
uint32_t calculateFreeCardSpace(FATFS* pfs, DWORD* fre_clust);

#endif /* INC_FATFS_WRAPER_FUNCTIONS_H_ */

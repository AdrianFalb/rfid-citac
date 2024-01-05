/*
 * fatfs_wraper_functions.c
 *
 *  Created on: Dec 22, 2023
 *      Author: u
 */
#include "fatfs_wraper_functions.h"

/**
 * @brief Function creates a directory with specified path.\n
 * @details Function is used to create a directory using FATFS predefined function f_mkdir.\n
 * 			Function takes a char pointer as an input parameter.
 * 			It returns 1, if a directory was successfully created or already exists.\n
 * 			Else returns 0.
 * @param[in] path -> char pointer that points to an array containing the path to dir.
 */
uint8_t createDirectory(char* path)
{
	FRESULT res;

	res = f_mkdir(path);

	if(res != FR_OK && res != FR_EXIST)
		return 0;

	return 1;
}

/**
 * @brief Function is used to open a file in the file system with a specified name in write mode.\n
 * @details Function is used to open a file in the file system in write mod.\n
 * 			File is specified by the path input parameter that contains the file path in the file system.\n
 * 			If no file with specified path exists, then it is created and used to write.\n
 * @param[in] path -> char pointer that points to an array containing the path to file.
 */
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

/**
 * @brief Function is used to open a file in the file system with a specified name in read mode.\n
 * @details Function is used to open a file in the file system in write mod.\n
 * 			File is specified by the path input parameter that contains the file path in the file system.\n
 * 			If no file with specified path exists, then it is created and used to read.\n
 * @param[in] path -> char pointer that points to an array containing the path to file.
 */
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


/**
 * @brief Function is used to open a file in the file system with a specified name in pseudo append mode.\n
 * @details Function is used to open a file in the file system in write mod.\n
 * 			After the file is opened, this function checks for the end of the file using f_lseek() function.\n
 * 			If f_lseek finds the end of the file, it also sets the file pointer to the files end.\n
 * 			File is specified by the path input parameter that contains the file path in the file system.\n
 * 			If no file with specified path exists, then a new file is created.\n
 * @param[in] path -> char pointer that points to an array containing the path to file.
 */
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

/**
 * @brief Function is used to calculate the total file system space.\n
 * @param[in] pfs -> pointer to the FATFS file system object structure.
 */
uint32_t calculateTotalCardSpace(FATFS* pfs)
{
	return (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
}

/**
 * @brief Function is used to calculate free space of a file system .\n
 * @param[in] pfs -> pointer to the FATFS file system object structure.
 * @param[in] fre_clust -> pointer to the FATFS file system object structure.
 */
uint32_t calculateFreeCardSpace(FATFS* pfs, DWORD* fre_clust)
{
	return (uint32_t)(*fre_clust * pfs->csize * 0.5);
}

/**
 * @brief Function is used to concatenate directory with file name to create a path to a .TXT file.\n
 * @details Function creates a path to a file, that  will be used to create or open an existing file in the file system.\n
 * 			The resulting path is stored in a buffer, which is passed as a pointer to the function.\n
 * @param[in] buff -> pointer to a buffer that is used to store the finished path
 * @param[in] dir -> pointer that holds the directory name, in which the file should be created/opened
 * @param[in] date -> pointer that holds the current date that is used to create the file name along with the directory name
 */
void createPathToFile(char* buff, char* dir, char* date)
{
	strcpy(buff, "/");
	strcat(buff,dir);
	strcat(buff,"/");
	strcat(dir,"_");
	strcat(dir,date);
	strcat(buff,dir);
	strcat(buff,".TXT");
}

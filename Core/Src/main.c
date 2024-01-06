/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "mfrc522.h"
#include "stdio.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_rtc.h"
#include "stm32f3xx_ll_rtc.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stm32f3xx_hal_conf.h"
#include "stm32f3xx_it.h"

#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char version_buffer[128];
char message_buffer[128];
char buf_hex[12];

uint8_t uid_card_found = 0;

RTC_TimeTypeDef curTime;
RTC_DateTypeDef curDate;
UART_HandleTypeDef huart1;
char bld[40];
char tm[40];
char buf[25];
char *months[] = {"???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char *delim = " :";

#define BUFFER_SIZE 128
#define UART_BUFFER_SIZE 128

char buff[BUFFER_SIZE];
char uart_buf[UART_BUFFER_SIZE];

char bufftest[BUFFER_SIZE];

char path[BUFFER_SIZE];

uint8_t testMinutes = 0;
uint8_t compareMinutes = 0;

FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
uint32_t uart_buf_len;

uint8_t r;
uint8_t buttonState = 0;
uint8_t failedCard = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  resetBuffer(buff, BUFFER_SIZE);
  resetBuffer(uart_buf, UART_BUFFER_SIZE);
  resetBuffer(path, BUFFER_SIZE);


  uint8_t not_vypis = 0;
  HAL_Delay(1000);



  ILI9163_RegisterCallback(SPI_TransmitData);
  HAL_Delay(50);

  lcdInitialise(192);
  lcdClearDisplay(decodeRgbValue(0, 0, 0));
  lcdPutS("Stlacte tlacidlo...", lcdTextX(2), lcdTextY(8), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));

  HAL_Delay(1000);


  HAL_RTC_GetTime(&hrtc, &curTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &curDate, RTC_FORMAT_BIN);// Replace rtclock.breakTime(rtclock.now(), &curTime);
  RTC_DateTypeDef sDate;
  RTC_TimeTypeDef sTime;
  sDate.Year = 0x23; // Set the year (e.g., 2023 - 2000)
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sTime.Hours = 0x12;
  sTime.Minutes = 0x00;
  sTime.Seconds = 0x00;

  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

  setBuildTime(&sDate, &sTime);

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  // Initialize MFRC522 and read the version
  uint8_t status;
  char card_buffer[4];

  uint8_t testCardFlag = 0;
  int diffMinutes;


  MFRC522_PCD_Init();
  HAL_Delay(1000);


  /* USER CODE END 2 */
  // Enter sleep mode
   HAL_SuspendTick();
   HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  showClock(1);

	  if (buttonState > 0)
	  {
		  if (not_vypis == 0)
		  {
			  HAL_Delay(100);
			  lcdClearDisplay(decodeRgbValue(0, 0, 0));
			  lcdPutS("Prilozte kartu...", lcdTextX(2), lcdTextY(8), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
			  HAL_Delay(100);
			  not_vypis = 1;
		  }

		  // Read card
		  if (uid_card_found == 0)
		  {
			  status = STATUS_ERROR;
			  status = MFRC522_PICC_RequestA(PICC_CMD_REQA, card_buffer);

			  if(status != STATUS_OK && !testCardFlag){
				  showClock(1);
				  compareMinutes = curTime.Minutes;
				  testCardFlag = 1;
			  }

			  else if(testCardFlag){
				  diffMinutes = testMinutes - compareMinutes;
				  if( diffMinutes < 0){
				  		diffMinutes = diffMinutes + 60;
				  }
				  if(diffMinutes >= 3){
					  buttonState = 0;
					  uid_card_found = 0;
					  not_vypis = 0;
					  testCardFlag = 0;
					  lcdClearDisplay(decodeRgbValue(0, 0, 0));
					  lcdPutS("Stlacte tlacidlo...", lcdTextX(2), lcdTextY(8), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
		  			  // Enter sleep mode
					  HAL_SuspendTick();
					  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
				  }
			  }

			  if (status == STATUS_OK)
		  		  {
		  			  memset(message_buffer, 0, sizeof(message_buffer));
		  			  snprintf(message_buffer, sizeof(message_buffer), "\n\r%X,%X,%X", card_buffer[0], card_buffer[1], card_buffer[2]);

		  			  HAL_UART_Transmit(&huart2, (uint8_t *)message_buffer, sizeof(message_buffer), 250);

		  			  status = MFRC522_PICC_Anticollision(card_buffer);
		  			  if (status == STATUS_OK)
		  			  {
		  				  memset(message_buffer, 0, sizeof(message_buffer));
		  				  snprintf(message_buffer, sizeof(message_buffer), "\n\rUID: %X %X %X %X", card_buffer[0], card_buffer[1], card_buffer[2], card_buffer[3]);

		  				  uid_card_found = 1;

		  				  HAL_UART_Transmit(&huart2, (uint8_t *)message_buffer, sizeof(message_buffer), 250);

		  			  }
		  		  }
		  }

		  	  if (uid_card_found == 1)
		  	  {
		  		  snprintf(buf_hex, sizeof(buf_hex), "%X_%X_%X_%X", card_buffer[0], card_buffer[1], card_buffer[2], card_buffer[3]);

  				  if(f_mount(&fs, "", 1) != FR_OK)
  					  Error_Handler();

		  		  r = createDirectory(buf_hex);
		  		  if(r == 0) {
		  			  Error_Handler();
		  		  }

		  		 char buf_hex_copy[12];
		  		 strcpy(buf_hex_copy,buf_hex);

				  if(strlen(bld) > 0)
					  createPathToFile(path, buf_hex, bld);

				  resetBuffer(buf_hex, sizeof(buf_hex));
				  strcpy(buf_hex, buf_hex_copy);

  				  HAL_Delay(1000);

				  r = openFileForAppend(&fil, path);
				  if(r == 0){
					  Error_Handler();
				  }

				  f_printf(&fil,"%s,%s,%s,%d;\n", buf_hex, bld, tm, buttonState);

				  // Output to LCD display
				  HAL_Delay(100);
				  lcdClearDisplay(decodeRgbValue(0, 0, 0));

				  switch (buttonState)
				  {
				  	  case 1:
						strcpy(buff,"Prichod: ");
						strcat(buff, tm);
						break;
				  	  case 2:
				  		strcpy(buff,"Odchod: ");
				  		strcat(buff, tm);
				  		break;
				  	  default:
				  		strcpy(buff,"Chyba");
		  		 		break;
				  	}

				  lcdPutS(buf_hex, lcdTextX(2), lcdTextY(1), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
		 		  lcdPutS(buff, lcdTextX(2), lcdTextY(4), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));

		 		  // Close file
  				  fres = f_close(&fil);
 				  if (fres != FR_OK)
  					  Error_Handler();

 				  // Unmount SDCARD
  				  fres = f_mount(NULL, "", 1);
  				  if (fres != FR_OK)
  				      Error_Handler();

  				  // Clear display
  				  HAL_Delay(5000);
  				  lcdClearDisplay(decodeRgbValue(0, 0, 0));
  				  lcdPutS("Stlacte tlacidlo...", lcdTextX(2), lcdTextY(6), decodeRgbValue(255, 255, 255), decodeRgbValue(0, 0, 0));
  				  uid_card_found = 0;
  				  buttonState = 0;
  				  not_vypis = 0;

  				  resetBuffer(buff, BUFFER_SIZE);
  				  resetBuffer(bld, sizeof(bld));
  				  resetBuffer(tm, sizeof(tm));
  				  resetBuffer(buf_hex, sizeof(buf_hex));
  				 // Enter sleep mode
				   HAL_SuspendTick();
				   HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		  	  }
	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// Wake up from sleep mode
	HAL_ResumeTick();

	if(buttonState > 0)
		return;

	if (GPIO_Pin == PRICHOD_Pin)
	{
		buttonState = 1;
	}
	else if (GPIO_Pin == ODCHOD_Pin)
	{
		buttonState = 2;
	}
	else
	{
		buttonState = 0;
		__NOP();
	}
}

void resetBuffer(char* buffer, uint32_t buff_size)
{
	for(int i = 0; i < buff_size; i++)
		buffer[i] = '\0';
}


void setBuildTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
    // Timestamp format: "Mar 3 2019 12:34:56"
    snprintf(bld, 40, "%s %s\n", __DATE__, __TIME__);
    char *token = strtok(bld, delim);
    while (token)
    {
        int m = str2month((const char *)token);
        if (m > 0)
        {
            date->Month = m;
            token = strtok(NULL, delim);
            date->Date = atoi(token);
            token = strtok(NULL, delim);
            date->Year = atoi(token) - 2000;
            token = strtok(NULL, delim);
            time->Hours = atoi(token);
            token = strtok(NULL, delim);
            time->Minutes = atoi(token);
            token = strtok(NULL, delim);
            time->Seconds = atoi(token);
        }
        token = strtok(NULL, delim);
    }
    snprintf(bld, 40, "%02d_%02d_%02d", date->Year + 2000, date->Month, date->Date);
    snprintf(tm, 40, "%02d:%02d:%02d", time->Hours, time->Minutes, time->Seconds);

}

int str2month(const char *str) {
    for (int i = 0; i < 12; ++i) {
        if (strncmp(str, months[i], 3) == 0) {
            return i + 1;  // Months are 1-based in the RTC_DateTypeDef structure
        }
    }
    return -1;  // Invalid month
}

void showClock(int seconds)
{
  char timeString[40];

  snprintf(bld, 40, "%s %s\n", __DATE__, __TIME__);

  HAL_RTC_GetTime(&hrtc, &curTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &curDate, RTC_FORMAT_BIN);

  // Format the time and date information
  snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d, %02d-%02d-%02d\r\n",
           curTime.Hours, curTime.Minutes, curTime.Seconds,
           curDate.Date, curDate.Month, curDate.Year + 2000);

  snprintf(bld, 40, "%02d_%02d_%02d", curDate.Year + 2000, curDate.Month, curDate.Date);
  snprintf(tm, 40, "%02d:%02d:%02d", curTime.Hours, curTime.Minutes, curTime.Seconds);
  testMinutes = curTime.Minutes;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

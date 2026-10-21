/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kb.h"
#include "sdk_uart.h"
#include "pca9538.h"
#include "oled.h"
#include "fonts.h"

#include "tim.h"
#include "buzzer.h"

#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIMER_INPUT 0
#define TIMER_RUNNING 1
#define TIMER_FINISHED 2

#define DEBOUNCE_DELAY 30
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t timerState = TIMER_INPUT;
uint32_t timerValue = 0;
uint32_t lastTick = 0;
uint8_t digitCount= 0;

char lastKey = '\0';
char stableKey = '\0';
uint32_t debounceTick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//void KB_Test( void );
//void OLED_KB( uint8_t OLED_Keys[]);
void oled_Reset( void );

void Check_Keyboard(void);
void Update_Timer(void);

void Show_Input_Screen(void);
void Show_Running_Screen(void);
void Show_Finished_Screen(void);
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
  MX_I2C1_Init();
  MX_USART6_UART_Init();

  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  oled_Init();
  Buzzer_Init();
  Show_Input_Screen();
  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  Check_Keyboard();
	  Update_Timer();

//	  KB_Test();
//	  HAL_Delay(500);

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

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//void KB_Test( void ) {
//	UART_Transmit( (uint8_t*)"KB test start\n" );
//	uint8_t R = 0, C = 0, L = 0, Row[4] = {ROW4, ROW3, ROW2, ROW1}, Key, OldKey, OLED_Keys[12] = {0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
//	oled_Reset();
//	oled_WriteString("BURMALDA", Font_7x10, White);
//	OLED_KB(OLED_Keys);
//	oled_UpdateScreen();
//	for ( int i = 0; i < 4; i++ ) {
//		while( !( R && C && L ) ) {
//			OldKey = Key;
//			Key = Check_Row( Row[i] );
//			if ( Key == 0x01 && Key != OldKey) {
//				UART_Transmit( (uint8_t*)"Right pressed\n" );
//				R = 1;
//				OLED_Keys[2+3*i] = 0x31;
//				OLED_KB(OLED_Keys);
//			} else if ( Key == 0x02 && Key != OldKey) {
//				UART_Transmit( (uint8_t*)"Center pressed\n" );
//				C = 1;
//				OLED_Keys[1+3*i] = 0x31;
//				OLED_KB(OLED_Keys);
//			} else if ( Key == 0x04 && Key != OldKey) {
//				UART_Transmit( (uint8_t*)"Left pressed\n" );
//				L = 1;
//				OLED_Keys[3*i] = 0x31;
//				OLED_KB(OLED_Keys);
//			}
//		}
//		UART_Transmit( (uint8_t*)"Row complete\n" );
//		R = C = L = 0;
//		HAL_Delay(25);
//	}
//	UART_Transmit( (uint8_t*)"KB test complete\n");
//}
//void OLED_KB( uint8_t OLED_Keys[12]) {
//	for (int i = 3; i >= 0; i--) {
//		oled_SetCursor(56, 5+(4-i)*10);
//		for (int j = 0; j < 3; j++) {
//			oled_WriteChar(OLED_Keys[j+3*i], Font_7x10, White);
//		}
//	}
//	oled_UpdateScreen();
//}
void oled_Reset( void ) {
	oled_Fill(Black);
	oled_SetCursor(0, 0);
	oled_UpdateScreen();
}


void Show_Input_Screen(void) {
    char timerText[20];

    snprintf(timerText, sizeof(timerText),
             "TIMER: %lu s", (unsigned long)timerValue);

    oled_Reset();

    oled_WriteString(timerText, Font_7x10, White);

    oled_SetCursor(35, 20);
    oled_WriteString("1   2   3", Font_7x10, White);

    oled_SetCursor(35, 30);
    oled_WriteString("4   5   6", Font_7x10, White);

    oled_SetCursor(35, 40);
    oled_WriteString("7   8   9", Font_7x10, White);

    oled_SetCursor(35, 50);
    oled_WriteString("*   0   #", Font_7x10, White);

    oled_UpdateScreen();
}


void Show_Running_Screen(void) {
    char timerText[20];

    snprintf(timerText, sizeof(timerText),
             "%lu", (unsigned long)timerValue);

    oled_Reset();

    oled_SetCursor(45, 10);
    oled_WriteString("TIMER", Font_7x10, White);

    oled_SetCursor(50, 30);
    oled_WriteString(timerText, Font_7x10, White);

    oled_SetCursor(25, 50);
    oled_WriteString("* - CANCEL", Font_7x10, White);

    oled_UpdateScreen();
}


void Show_Finished_Screen(void) {
    oled_Reset();

    oled_SetCursor(25, 10);
    oled_WriteString("TIME IS UP!", Font_7x10, White);

    oled_SetCursor(60, 30);
    oled_WriteString("0", Font_7x10, White);

    oled_SetCursor(20, 50);
    oled_WriteString("# - RESTART", Font_7x10, White);

    oled_UpdateScreen();
}


void Check_Keyboard(void) {
    char key = Get_Key();

    if (key != lastKey) {
        lastKey = key;
        debounceTick = HAL_GetTick();
    }

    if (HAL_GetTick() - debounceTick >= DEBOUNCE_DELAY) {

        if (key != stableKey) {
            stableKey = key;

            if (stableKey != '\0') {

                if (timerState == TIMER_INPUT) {

                    if (stableKey >= '0' && stableKey <= '9') {
                        if (digitCount < 4) {
                            timerValue = timerValue * 10 + (stableKey - '0');
                            digitCount++;

                            Show_Input_Screen();
                        }
                    }
                    else if (stableKey == '*' && digitCount > 0) {
                        timerValue = 0;
                        digitCount = 0;

                        Show_Input_Screen();
                    }
                    else if (stableKey == '#' && timerValue > 0) {
                        timerState = TIMER_RUNNING;
                        lastTick = HAL_GetTick();

                        Show_Running_Screen();
                    }
                }

                else if (timerState == TIMER_RUNNING) {

                    if (stableKey == '*') {
                        timerValue = 0;
                        digitCount = 0;
                        timerState = TIMER_INPUT;

                        Show_Input_Screen();
                    }
                }

                else if (timerState == TIMER_FINISHED) {

                    if (stableKey == '#') {
                    	Buzzer_Set_Volume(BUZZER_VOLUME_MUTE);
                        timerValue = 0;
                        digitCount = 0;
                        timerState = TIMER_INPUT;

                        Show_Input_Screen();
                    }
                }
            }
        }
    }
}


void Update_Timer(void) {
    if (timerState == TIMER_RUNNING) {

        if (HAL_GetTick() - lastTick >= 1000) {
            timerValue--;
            lastTick += 1000;

            if (timerValue == 0) {
                timerState = TIMER_FINISHED;
                Buzzer_Set_Freq(1000);
                Show_Finished_Screen();

                for (int i = 0; i < 3; i++) {
                    Buzzer_Set_Volume(BUZZER_VOLUME_MAX);
                    HAL_Delay(200);
                    Buzzer_Set_Volume(BUZZER_VOLUME_MUTE);
                    HAL_Delay(200);
                }
            }
            else {
            	Show_Running_Screen();
            }
        }
    }
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

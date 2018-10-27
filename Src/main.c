/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"

/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "stdlib.h"
#include "string.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

/* USER CODE BEGIN PV */
/* --------------------------- Private variables -----------------------------*/

AUTOMATE_STATES current_state = menu_navigation;
CURSOR_STATES current_cursor = exposition;

// Exposition parameters
uint16_t set_exp_minutes = 0, set_exp_sec = 3;
uint16_t tmp_exp_minutes, tmp_exp_sec;
// Number of shots
uint16_t set_num_shots = 3, tmp_num_shots;
// Interval paramters
uint16_t set_int_minutes = 0, set_int_sec = 5;
uint16_t tmp_int_minutes, tmp_int_sec;



uint8_t update_screen_flag = 0, gate_flag = 0, exiting_run = 0;

uint32_t seconds_counter = 0;

char *Menu_Items[]={"Exp:               ",
                    "Num:               ",
                    "Int:               ",
                    "Start              "};

char buffer[5];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
//static void MX_IWDG_Init(void);
static void MX_TIM14_Init(void);
static void MX_TIM16_Init(void);
static void MX_TIM17_Init(void);

/* USER CODE BEGIN PFP */
/* ---------------------------- Private function prototypes ------------------*/

void reverse(char s[]);
void itoa(int n, char s[]);

void Menu(void);
void Convert_time(uint16_t* minutes, uint16_t* seconds);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_TIM3_Init();
  //MX_IWDG_Init();
  MX_TIM14_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
  HAL_TIM_Base_Start_IT(&htim3);


  // Seconds Counter startup duty
  HAL_TIM_Base_Start_IT(&htim14);
  HAL_TIM_Base_Stop_IT(&htim14);
  HAL_TIM_Base_Start_IT(&htim16);

  HAL_TIM_Base_Start_IT(&htim17);
  
  HAL_GPIO_WritePin(GPIOA, LED_RED_PIN, GPIO_PIN_RESET);
  ssd1306_Init();
  current_cursor = exposition;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    //HAL_IWDG_Refresh(&hiwdg);

    if(__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_SYSCLKSOURCE_STATUS_HSE) HAL_GPIO_WritePin(GPIOA, LED_GREEN_PIN, GPIO_PIN_SET);
    else HAL_GPIO_WritePin(GPIOA, LED_GREEN_PIN, GPIO_PIN_RESET);

    if (update_screen_flag)
    {
      ssd1306_Fill(Black);
      ssd1306_UpdateScreen();
      update_screen_flag = 0;
    }
    
    if (seconds_counter > STOP_TIME_SEC) //if more than 10 s need to stop
    {
      // Entering STOP Mode Procedure
      seconds_counter = 0;

      GPIO_InitTypeDef GPIO_InitStruct;
      

      ssd1306_WriteCommand(0xAE);       // OLED Off

      GPIO_InitStruct.Pin = ENC_BUTTON_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING_FALLING;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

      HAL_GPIO_WritePin(GPIOA, LED_GREEN_PIN|LED_RED_PIN|GATE_PIN|FOCUS_PIN, GPIO_PIN_RESET);

      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);

      // Exit from STOP Mode procedure
      SystemClock_Config();
      ssd1306_WriteCommand(0xAF);      // OLED On

      GPIO_InitStruct.Pin = ENC_BUTTON_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      current_state = menu_navigation;
    }

    Menu();

    if (current_state == running_timer || current_state == running_interval)
    {
      if(gate_flag)
      {
        // Reset Gate & Focus
        HAL_GPIO_WritePin(GPIOA, FOCUS_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GATE_PIN, GPIO_PIN_RESET);
        HAL_TIM_Base_Stop_IT(&htim14);
        HAL_Delay(500);
        __HAL_TIM_SET_COUNTER(&htim14, 0);
        HAL_TIM_Base_Start_IT(&htim14);
        gate_flag = 0;
        // If not last shot, go to Interval state
        if(tmp_num_shots != 0)
        {
          tmp_int_minutes = set_int_minutes;
          tmp_int_sec = set_int_sec;
          tmp_exp_minutes = set_exp_minutes;
          tmp_exp_sec = set_exp_sec;
          current_state = running_interval;
          HAL_GPIO_WritePin(GPIOA, LED_RED_PIN,GPIO_PIN_RESET); // 1s LED Off
        }
        else
        {
          HAL_TIM_Base_Stop_IT(&htim14);
          __HAL_TIM_SET_COUNTER(&htim14, 0);
          HAL_GPIO_WritePin(GPIOA, LED_RED_PIN,GPIO_PIN_RESET); // 1s LED Off
          tmp_exp_minutes = set_exp_minutes;
          tmp_exp_sec = set_exp_sec;
          current_state = menu_navigation;
          update_screen_flag = 1;
        }
      }
    }
      
    if (exiting_run)
    {
      HAL_TIM_Base_Stop_IT(&htim16);
      HAL_Delay(1000);
      exiting_run = 0;
      HAL_TIM_Base_Start_IT(&htim16);
      HAL_GPIO_WritePin(GPIOA, FOCUS_PIN, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOA, GATE_PIN, GPIO_PIN_RESET);
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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Enables the Clock Security System
    */
  HAL_RCC_EnableCSS();

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IWDG init function */
/*
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
*/
/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_Encoder_InitTypeDef sConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 3;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 15;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 15;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM14 init function */
static void MX_TIM14_Init(void)
{

  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 7999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 1000;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM16 init function */
static void MX_TIM16_Init(void)
{

  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 799;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 2700;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM17 init function */
static void MX_TIM17_Init(void)
{

  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 7999;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 1000;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

// Reverse string
void reverse(char s[])
{
  int i, j;
  char c;

  for (i = 0, j = strlen(s)-1; i<j; i++, j--)
  {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

// Convert number n into char[] s
void itoa(int n, char s[])
{
  int i, sign;

  if ((sign = n) < 0)  /* записываем знак */
    n = -n;          /* делаем n положительным числом */
  i = 0;
  do {       /* генерируем цифры в обратном порядке */
    s[i++] = n % 10 + '0';   /* берем следующую цифру */
  } while ((n /= 10) > 0);     /* удаляем */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}

// Generate and update menu
void Menu(void)
{
  if (current_state != running_timer && current_state != running_interval)
  {
    for (int i=0; i<NUMBER_OF_MENU_ITEMS; i++)
    {
      ssd1306_SetCursor(0, i*FONT_HEIGHT);
      if(current_cursor == i) ssd1306_WriteString("->", Font_7x10, White);
      ssd1306_WriteString(Menu_Items[i], Font_7x10, White);
      switch (i)
      {
        case exposition:
          ssd1306_SetCursor(FONT_WIDTH*INDENT, i*FONT_HEIGHT);
          itoa(set_exp_minutes, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("`  ", Font_7x10, White);
          itoa(set_exp_sec, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("``", Font_7x10, White);
          break;
        case number:
          ssd1306_SetCursor(FONT_WIDTH*INDENT, i*FONT_HEIGHT);
          itoa(set_num_shots, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          break;
        case interval:
          ssd1306_SetCursor(FONT_WIDTH*INDENT, i*FONT_HEIGHT);
          itoa(set_int_minutes, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("`  ", Font_7x10, White);
          itoa(set_int_sec, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("``", Font_7x10, White);
          break;
        case start:
          break;
      }
    }
  }
  else
  {
    for (int i=0; i<NUMBER_OF_MENU_ITEMS-1; i++)
    {
      ssd1306_SetCursor(0, i*FONT_HEIGHT);
      ssd1306_WriteString(Menu_Items[i], Font_7x10, White);
      switch (i)
      {
        case exposition:
          ssd1306_SetCursor(FONT_WIDTH*INDENT, i*FONT_HEIGHT);
          itoa(tmp_exp_minutes, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("`  ", Font_7x10, White);
          itoa(tmp_exp_sec, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("``", Font_7x10, White);
          break;
        case number:
          ssd1306_SetCursor(FONT_WIDTH*INDENT, i*FONT_HEIGHT);
          itoa(tmp_num_shots, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          break;
        case interval:
          ssd1306_SetCursor(FONT_WIDTH*INDENT, i*FONT_HEIGHT);
          itoa(tmp_int_minutes, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("`  ", Font_7x10, White);
          itoa(tmp_int_sec, buffer);
          ssd1306_WriteString(buffer, Font_7x10, White);
          ssd1306_WriteString("``", Font_7x10, White);
          break;
        case start:
          break;
      }
    }
    ssd1306_SetCursor(0, NUMBER_OF_MENU_ITEMS*FONT_HEIGHT);
    ssd1306_WriteString("     Running      ", Font_7x10, White);
  }
  ssd1306_UpdateScreen();
}


void Convert_time(uint16_t* minutes, uint16_t* seconds)
{
  if (*seconds >= 60)
  {
    uint16_t tmp_min;
    tmp_min = (*seconds)/60;
    *minutes = *minutes+tmp_min;
    *seconds = *seconds-tmp_min*60;
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

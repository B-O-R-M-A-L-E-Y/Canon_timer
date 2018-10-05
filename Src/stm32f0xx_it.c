/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
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
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"

/* USER CODE BEGIN 0 */
#include "ssd1306.h"

extern AUTOMATE_STATES current_state;
extern CURSOR_STATES current_cursor;

// Exposition parameters
extern uint16_t set_exp_minutes, set_exp_sec;
extern uint16_t tmp_exp_minutes, tmp_exp_sec;
// Number of shots
extern uint16_t set_num_shots, tmp_num_shots;
// Interval paramters
extern uint16_t set_int_minutes, set_int_sec;
extern uint16_t tmp_int_minutes, tmp_int_sec;

extern uint8_t update_screen_flag, gate_flag;

extern uint32_t seconds_counter;

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim14;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

/******************************************************************************/
/*            Cortex-M0 Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  HAL_RCC_NMI_IRQHandler();
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
  /* USER CODE BEGIN HardFault_IRQn 1 */

  /* USER CODE END HardFault_IRQn 1 */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles TIM3 global interrupt.
*/
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */
  seconds_counter = 0;
  uint8_t rule = 0;
  rule = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3);
  switch(current_state)
  {
    case menu_navigation:
      if(rule) current_cursor == exposition ? current_cursor = start : current_cursor--;
      else current_cursor == start ? current_cursor = exposition : current_cursor++;
      update_screen_flag = 1;
      break;
    case config_exposition_min:
      rule ? set_exp_minutes-- : set_exp_minutes++;
      break;
    case config_exposition_sec:
      rule ? set_exp_sec-- : set_exp_sec++;
      break;
    case config_num_shots:
      rule ? set_num_shots-- : set_num_shots++;
      break;
    case config_interval_min:
      rule ? set_int_minutes-- : set_int_minutes++;
      break;
    case config_interval_sec:
      rule ? set_int_sec-- : set_int_sec++;
      break;
    case running_timer:
      break;
    case running_interval:
      break;
  }
  /* USER CODE END TIM3_IRQn 1 */
}

/**
* @brief This function handles TIM14 global interrupt.
*/
void TIM14_IRQHandler(void)
{
  /* USER CODE BEGIN TIM14_IRQn 0 */

  /* USER CODE END TIM14_IRQn 0 */
  HAL_TIM_IRQHandler(&htim14);
  /* USER CODE BEGIN TIM14_IRQn 1 */
  HAL_GPIO_TogglePin(GPIOA, LED_RED_PIN);
  if (current_state == running_timer)
  {
    if (tmp_exp_sec == 0)
    {
      if (tmp_exp_minutes == 0)
      {
        if (tmp_num_shots == 0)
        {
          gate_flag = 1;
          return;
        }
        tmp_num_shots--;
        // Check if loops in interrupt
        HAL_TIM_Base_Stop_IT(&htim14);
        gate_flag = 1;
        return;
      }
      tmp_exp_minutes--;
      tmp_exp_sec = 59;
      return;
    }
    tmp_exp_sec--;
  }
  if (current_state == running_interval)
  {
    if (tmp_int_sec == 0)
    {
      if (tmp_int_minutes == 0)
      {
        tmp_exp_minutes = set_exp_minutes;
        tmp_exp_sec = set_exp_sec;
        tmp_int_minutes = set_int_minutes;
        tmp_int_sec = set_int_sec;
        current_state = running_timer; // CONTINUE HERE
        HAL_GPIO_WritePin(GPIOA, GATE_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, FOCUS_PIN, GPIO_PIN_SET);
        return;
      }
      tmp_int_minutes--;
      tmp_int_sec = 59;
      return;
    }
    tmp_int_sec--;
  }
  /* USER CODE END TIM14_IRQn 1 */
}

/**
* @brief This function handles TIM16 global interrupt.
*/
void TIM16_IRQHandler(void)
{
  /* USER CODE BEGIN TIM16_IRQn 0 */

  /* USER CODE END TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim16);
  /* USER CODE BEGIN TIM16_IRQn 1 */

  if (HAL_GPIO_ReadPin(GPIOA, ENC_BUTTON_PIN))
  {
    seconds_counter = 0;
    switch (current_state)
    {
      case menu_navigation:
        switch(current_cursor)
        {
          case exposition:
            current_state = config_exposition_min;
            break;
          case number:
            current_state = config_num_shots;
            break;
          case interval:
            current_state = config_interval_min;
            break;
          case start:
            current_state = running_timer;
            update_screen_flag = 1;
            tmp_exp_minutes = set_exp_minutes;
            tmp_exp_sec = set_exp_sec;
            tmp_num_shots = set_num_shots;
            tmp_int_minutes = set_int_minutes;
            tmp_int_sec = set_int_sec;
            __HAL_TIM_SET_COUNTER(&htim14, 0);
            HAL_TIM_Base_Start_IT(&htim14);
            HAL_GPIO_WritePin(GPIOA, GATE_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, FOCUS_PIN, GPIO_PIN_SET);
            break;
        }
        break;
      case config_exposition_min:
        current_state = config_exposition_sec;
        break;
      case config_exposition_sec:
        Convert_time(&set_exp_minutes, &set_exp_sec);
        current_state = menu_navigation;
        break;
      case config_num_shots:
        current_state = menu_navigation;
        break;
      case config_interval_min:
        current_state = config_interval_sec;
        break;
      case config_interval_sec:
        Convert_time(&set_int_minutes, &set_int_sec);
        current_state = menu_navigation;
        break;
      case running_timer:
        break;
      case running_interval:
        break;
    }
  }
  /* USER CODE END TIM16_IRQn 1 */
}

/**
* @brief This function handles TIM17 global interrupt.
*/
void TIM17_IRQHandler(void)
{
  /* USER CODE BEGIN TIM17_IRQn 0 */

  /* USER CODE END TIM17_IRQn 0 */
  HAL_TIM_IRQHandler(&htim17);
  /* USER CODE BEGIN TIM17_IRQn 1 */
  if (current_state != running_timer && current_state != running_interval)
  {
    seconds_counter++;
  }
  /* USER CODE END TIM17_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

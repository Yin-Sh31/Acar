/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "control.h"
#define MAX_speed 1000
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
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void drive(int r, int l);
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
  float err = 0.0, integral = 0.0, prev_err = 0.0;
  int count = 0, sleep = 0, bac = 0;
  int sensors[4] = {0};
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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  { // 视觉
    if (sleep < 1)
    {
      sensors[0] = HAL_GPIO_ReadPin(left_GPIO_Port, left_Pin);
      sensors[1] = HAL_GPIO_ReadPin(mleft_GPIO_Port, mleft_Pin);
      sensors[2] = HAL_GPIO_ReadPin(mright_GPIO_Port, mright_Pin);
      sensors[3] = HAL_GPIO_ReadPin(right_GPIO_Port, right_Pin);
    }
    else
      sleep--;

    if (sensors[0] == 1 && sensors[1] == 1 && sensors[2] == 1 && sensors[3] == 1)
    {
      if (count < 3)
      {
        err = 0;
        count++;
        sleep = sl / 4;
      }
      else if (count == 3)
      {
        err = aleft;
        count++;
        sleep = sl;
      }
      // else
      // {
      //   for (int i = 100; i > 0; i--) // 刹车
      //   {
      //     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, MAX_speed);
      //     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, MAX_speed);
      //     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, MAX_speed);
      //     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, MAX_speed);
      //   }
      //   return 0;
      // }
    }
    else if (sensors[0] == 1 && sensors[1] == 1 && sensors[2] == 0 && sensors[3] == 0)
      err = aleft;
    else if (sensors[0] == 0 && sensors[1] == 0 && sensors[2] == 1 && sensors[3] == 1)
      err = aright;
    else if (sensors[0] == 1 && sensors[1] == 0 && sensors[2] == 0 && sensors[3] == 0)
      err = left;
    else if (sensors[0] == 0 && sensors[1] == 1 && sensors[2] == 0 && sensors[3] == 0)
      err = mleft;
    else if (sensors[0] == 0 && sensors[1] == 0 && sensors[2] == 1 && sensors[3] == 0)
      err = mringt;
    else if (sensors[0] == 0 && sensors[1] == 0 && sensors[2] == 0 && sensors[3] == 1)
      err = ringt;
    else if ((sensors[0] == 0 && sensors[1] == 0 && sensors[2] == 0 && sensors[3] == 0) && !(err < mleft || err > mringt))
      err = 0;
    //    else
    //      bac = 1;

    // pid
    integral += err;
    if (integral > 100)
      integral = 100;
    else if (integral < -100)
      integral = -100;

    int pid = kp * err + ki * integral + kd * (err - prev_err);
    prev_err = err;

    // 驱动
    int left_speed;
    int right_speed;
    if (bac == 1)
    {
      left_speed = -be_speed;
      right_speed = -be_speed;
      bac = 0;
    }
    else
    {
      left_speed = be_speed - pid;
      right_speed = be_speed + pid - cc;
    }
    if (left_speed > MAX_speed)
      left_speed = MAX_speed;
    else if (left_speed < -MAX_speed)
      left_speed = -MAX_speed;
    if (right_speed > MAX_speed)
      right_speed = MAX_speed;
    else if (right_speed < -MAX_speed)
      right_speed = -MAX_speed;

    if (left_speed >= 0)
    {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, left_speed);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    }
    else
    {
      left_speed = -left_speed;
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, left_speed);
    }
    if (left_speed >= 0)
    {
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, right_speed);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
    }
    else
    {
      right_speed = -right_speed;
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, right_speed);
    }
    //    HAL_Delay(cir);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000 - 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Reset1_Pin | Reset2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : left_Pin mleft_Pin mright_Pin right_Pin */
  GPIO_InitStruct.Pin = left_Pin | mleft_Pin | mright_Pin | right_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Reset1_Pin Reset2_Pin */
  GPIO_InitStruct.Pin = Reset1_Pin | Reset2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void drive(int r, int l);
{
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

#ifdef USE_FULL_ASSERT
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

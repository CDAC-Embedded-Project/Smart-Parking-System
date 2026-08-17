/*/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include<string.h>
#include<stdio.h>
#include <stdint.h>
#include "lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SERVO_0_DEG    500    // 0.5 ms pulse
#define SERVO_90_DEG   1500   // 1.5 ms pulse
#define GATE_OPEN_TIME 3000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char uartMsg[100];

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

uint8_t Slot1_Status = 0;
uint8_t Slot2_Status = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

uint16_t angle_to_pulse(uint8_t angle_deg);
void Servo1_SetAngle(uint8_t angle_deg);
void Servo2_SetAngle(uint8_t angle_deg);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    if(HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxHeader,RxData) != HAL_OK)
    {
    	sprintf(uartMsg,"ERROR: CAN Receive Failed\r\n");
        HAL_UART_Transmit(&huart2,(uint8_t*)uartMsg,strlen(uartMsg),HAL_MAX_DELAY);
        return;
    }

    if(RxHeader.IDE == CAN_ID_STD &&
       RxHeader.StdId == 0x123)
    {
    	if(RxHeader.DLC == 2)
    	{
    	    Slot1_Status = RxData[0];
    	    Slot2_Status = RxData[1];
    	}
    	else
    	{
    	    sprintf(uartMsg,"ERROR: Invalid DLC = %d\r\n",RxHeader.DLC);
    	    HAL_UART_Transmit(&huart2,(uint8_t*)uartMsg,strlen(uartMsg),HAL_MAX_DELAY);
    	    return;
    	}

        sprintf(uartMsg,"Received: S1=%d S2=%d\r\n",Slot1_Status,Slot2_Status);

        HAL_UART_Transmit(&huart2,(uint8_t*)uartMsg,strlen(uartMsg),HAL_MAX_DELAY);

        /* LED for Slot 1 */
        if(Slot1_Status == 1)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

        /* LED for Slot 2 */
        if(Slot2_Status == 1)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    }
    else
    {
        sprintf(uartMsg,"Unknown CAN ID: 0x%03lX\r\n",RxHeader.StdId);
        HAL_UART_Transmit(&huart2,(uint8_t*)uartMsg,strlen(uartMsg),HAL_MAX_DELAY);
    }
}
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
  MX_CAN1_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_TIM4_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, SERVO_0_DEG);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, SERVO_0_DEG);
  HAL_Delay(500);

  LCD_Init();
  LCD_UpdateParkingDisplay();

//  Servo1_SetAngle(90);
//    Servo2_SetAngle(90);
//    HAL_Delay(1000);
  /* USER CODE END 2 */



  /* USER CODE BEGIN 2 */
  HAL_UART_Transmit(&huart2, (uint8_t*)"HELLO\n\r", 7, HAL_MAX_DELAY);
  if (HAL_CAN_Start(&hcan1) == HAL_OK)
  {
      if(HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
      {
          sprintf(uartMsg,"ERROR: CAN Notification Failed\r\n");
          HAL_UART_Transmit(&huart2,(uint8_t*)uartMsg,strlen(uartMsg),HAL_MAX_DELAY);

          Error_Handler();
      }

      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);

      sprintf(uartMsg,"CAN Started Successfully\r\n");
      HAL_UART_Transmit(&huart2,(uint8_t*)uartMsg,strlen(uartMsg),HAL_MAX_DELAY);
  }
  else
  {
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);

      sprintf(uartMsg,"ERROR: CAN Start Failed\r\n");
      HAL_UART_Transmit(&huart2,(uint8_t*)uartMsg,strlen(uartMsg),HAL_MAX_DELAY);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
//    /* USER CODE END WHILE */
//
//    /* USER CODE BEGIN 3 */
//  /* USER CODE END 3 */
//  }

  while (1)
  	  {
  	      uint8_t entryNow = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET);
  	      uint8_t exitNow  = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET);

  	      // -------- ENTRY GATE --------
//  	      if (entryNow)
//  	      {
//  	          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, SERVO_90_DEG);
//  	          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);   // Green LED ON
//
//  	          if (!entryPrevState && (parkedCount < TOTAL_SLOTS))   // rising edge + slot available
//  	          {
//  	              parkedCount++;
//  	              lcdNeedsUpdate = 1;
//  	          }
//  	      }
//  	      else
//  	      {
//  	          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, SERVO_0_DEG);
//  	          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);     // Green LED OFF
//  	      }
//  	      entryPrevState = entryNow;

  	    if(entryNow)
  	    {
  	        if(parkedCount < TOTAL_SLOTS)        // Parking available
  	        {
  	            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, SERVO_90_DEG);

  	            if(!entryPrevState)
  	            {
  	                parkedCount++;
  	                lcdNeedsUpdate = 1;
  	            }
  	        }
  	        else                                 // Parking Full
  	        {
  	            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, SERVO_0_DEG);
  	            lcdNeedsUpdate = 1;              // LCD shows "Parking Full"
  	        }
  	    }
  	    else
  	    {
  	        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, SERVO_0_DEG);
  	    }

  	    entryPrevState = entryNow;

  	      // -------- EXIT GATE --------
  	      if (exitNow)
  	      {
  	          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, SERVO_90_DEG);
  	          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);   // Orange LED ON

  	          if (!exitPrevState && (parkedCount > 0))   // rising edge + car actually parked
  	          {
  	              parkedCount--;
  	              lcdNeedsUpdate = 1;
  	          }
  	      }
  	      else
  	      {
  	          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, SERVO_0_DEG);
  	          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);     // Orange LED OFF
  	      }
  	      exitPrevState = exitNow;

  	      // -------- LCD update (only when count changes, avoids I2C spam) --------
  	      if (lcdNeedsUpdate)
  	      {
  	    	  HAL_Delay(200);
  	          LCD_UpdateParkingDisplay();
  	          lcdNeedsUpdate = 0;
  	      }

  	      HAL_Delay(20);
  	  }

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
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

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
  CAN_FilterTypeDef canFilter;

  canFilter.FilterActivation = ENABLE;
  canFilter.FilterBank = 0;
  canFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  canFilter.FilterIdHigh = 0x0000;
  canFilter.FilterIdLow = 0x0000;
  canFilter.FilterMaskIdHigh = 0x0000;
  canFilter.FilterMaskIdLow = 0x0000;
  canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
  canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
  canFilter.SlaveStartFilterBank = 14;

  if(HAL_CAN_ConfigFilter(&hcan1, &canFilter) != HAL_OK)
  {
      Error_Handler();
  }
  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 83;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 19999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
uint16_t angle_to_pulse(uint8_t angle)
{
    return 500 + ((uint32_t)angle * 2000) / 180;
}

	void Servo1_SetAngle(uint8_t angle_deg)
	{
	    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, angle_to_pulse(angle_deg));
	}

	void Servo2_SetAngle(uint8_t angle_deg)
	{
	    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, angle_to_pulse(angle_deg));
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
#endif /* USE_FULL_ASSERT */*/

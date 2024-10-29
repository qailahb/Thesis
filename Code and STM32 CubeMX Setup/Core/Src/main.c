/* TBMPA Code */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this
 * software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_uart.h"

/* Define pins for input sensors and output electromagnets */
#define SENSOR1_PIN GPIO_PIN_7
#define SENSOR2_PIN GPIO_PIN_11
#define MAGNET1_PIN GPIO_PIN_9
#define MAGNET2_PIN GPIO_PIN_10

/* Global Variables */
TIM_HandleTypeDef htim2;  	// General purpose timer
TIM_HandleTypeDef htim1;  	// Magnet  control timer
UART_HandleTypeDef huart2;  // UART comms

/* Function Prototypes */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_TIM2_Init(void);
void MX_TIM1_Init(void);
void MX_USART2_UART_Init(void);
void EXTI0_1_IRQHandler(void);
void EXTI4_15_IRQHandler(void);
void schedule_magnet_activation(uint32_t delay_ms);
void send_debug_message_with_timestamp(char *message);
void Error_Handler(void);

/* Main Program */
int main(void)
{
    HAL_Init();  			// Initialises HAL Library
    SystemClock_Config();  	// System clock
    MX_GPIO_Init();  		// Initialises GPIO
    MX_TIM2_Init();  		// Initialises Timer 2 (general purposes)
    MX_TIM1_Init();  		// Initialises Timer 1 (magnet control)
    MX_USART2_UART_Init();  // Initialises USART2 for comms

    while (1)
    {
        // Keeps STM32 running
    	 // All logic is interrupt driven, thus can add low-priority tasks here
    }
}

/* System Clock Configuration  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Ensures consistent timing across system
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;		// High-speed internal clock source
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

/* GPIO Initialisation Function */
void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();  				// Enables GPIO Clock

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SENSOR1_PIN | SENSOR2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  // Interrupt with rising edge detection
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MAGNET1_PIN | MAGNET2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Output pins in push-pull mode
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* EXTI interrupt */
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2,0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

/* TIM2 Initialization for General Purpose  */
void MX_TIM2_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    // General system time base
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 48000 - 1;  // 48 MHz clock
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }

    HAL_TIM_Base_Start(&htim2);  // Starts Timer 2
}

/* TIM1 Initialisation for Magnet Control */
void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    // Magnet control time
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 48000 - 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 65535;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
     if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    HAL_TIM_Base_Start_IT(&htim1);  // Start Timer 1 with interrupt
}

/* USART2 Initialization for Debug Messages */
void MX_USART2_UART_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();  // Enables USART2 clock

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

    // Enable the GPIO pins clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin =  GPIO_PIN_2 | GPIO_PIN_3;  // Sets PA2 as TX, PA3 as RX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* Send Debug Message with Timestamp */
void send_debug_message_with_timestamp(char *message)
{
    char buffer[100];  // Hold
    uint32_t timestamp = __HAL_TIM_GET_COUNTER(&htim2)/1000;  // Current time (ms)

    // Add timestamp
    snprintf(buffer, sizeof(buffer),  "[%lu ms] %s", timestamp, message);

    //  Transmit
    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/* Magnet Activation Scheduling */
// Fuction that takes a delay in ms and activates magnet after
void schedule_magnet_activation(uint32_t delay_ms)
{
    send_debug_message_with_timestamp("Scheduling magnet activation.\r\n");
    __HAL_TIM_SET_COUNTER(&htim1, 0);  				// General counter reset
    __HAL_TIM_SET_AUTORELOAD(&htim1, delay_ms);  	// Delay set
    HAL_TIM_Base_Start_IT(&htim1);  				// Start timer with interrupt
}

/* Timer Interrupt Callback for Magnet Activation */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        HAL_GPIO_WritePin(GPIOA, MAGNET1_PIN, GPIO_PIN_RESET);  		// Deactivate magnet after a bit
        send_debug_message_with_timestamp("Magnet deactivated.\r\n");
        HAL_TIM_Base_Stop_IT(&htim1);  // Stop timer
    }
}

/* EXTIO Interrupt Handlers */
// Handle sensor interrupts when object detected
void EXTI0_1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(SENSOR1_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(SENSOR1_PIN);
        schedule_magnet_activation(10);  // NB: Set time for ... seconds after sensor detects
        send_debug_message_with_timestamp("Ball bearing detected at SENSOR1.\r\n");
    }
}

void EXTI4_15_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(SENSOR2_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(SENSOR2_PIN);
        schedule_magnet_activation(10);  // NB: Set time for ... seconds after sensor detected
        send_debug_message_with_timestamp("Ball bearing detected at SENSOR2.\r\n");
    }
}

/* Error Handler */

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
        send_debug_message_with_timestamp("Error!\r\n");
    }
}

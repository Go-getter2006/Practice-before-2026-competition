/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern float yaw;                  // JY901偏航角（全局）
extern uint8_t sensor_state;       // 灰度传感器状态
extern float left_target, right_target; // 左右轮目标速度
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define KEY1_Pin GPIO_PIN_0
#define KEY1_GPIO_Port GPIOC
#define KEY2_Pin GPIO_PIN_1
#define KEY2_GPIO_Port GPIOC
#define KEY3_Pin GPIO_PIN_2
#define KEY3_GPIO_Port GPIOC
#define STBY_Pin GPIO_PIN_5
#define STBY_GPIO_Port GPIOC
#define OUT_Pin GPIO_PIN_2
#define OUT_GPIO_Port GPIOB
#define BIN1_Pin GPIO_PIN_12
#define BIN1_GPIO_Port GPIOB
#define BIN2_Pin GPIO_PIN_13
#define BIN2_GPIO_Port GPIOB
#define AIN1_Pin GPIO_PIN_11
#define AIN1_GPIO_Port GPIOA
#define AIN2_Pin GPIO_PIN_12
#define AIN2_GPIO_Port GPIOA
#define AD2_Pin GPIO_PIN_15
#define AD2_GPIO_Port GPIOA
#define AD1_Pin GPIO_PIN_4
#define AD1_GPIO_Port GPIOB
#define AD0_Pin GPIO_PIN_5
#define AD0_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

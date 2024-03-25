/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LINK1_Pin GPIO_PIN_0
#define LINK1_GPIO_Port GPIOB
#define LINK1_EXTI_IRQn EXTI0_IRQn
#define I2C_LED1_Pin GPIO_PIN_1
#define I2C_LED1_GPIO_Port GPIOB
#define LINK2_Pin GPIO_PIN_2
#define LINK2_GPIO_Port GPIOB
#define LINK2_EXTI_IRQn EXTI2_IRQn
#define I2C_LED2_Pin GPIO_PIN_10
#define I2C_LED2_GPIO_Port GPIOB
#define LINK3_Pin GPIO_PIN_12
#define LINK3_GPIO_Port GPIOB
#define LINK3_EXTI_IRQn EXTI15_10_IRQn
#define I2C_LED3_Pin GPIO_PIN_13
#define I2C_LED3_GPIO_Port GPIOB
#define LINK4_Pin GPIO_PIN_14
#define LINK4_GPIO_Port GPIOB
#define LINK4_EXTI_IRQn EXTI15_10_IRQn
#define I2C_LED4_Pin GPIO_PIN_15
#define I2C_LED4_GPIO_Port GPIOB
#define LINK5_Pin GPIO_PIN_6
#define LINK5_GPIO_Port GPIOC
#define LINK5_EXTI_IRQn EXTI9_5_IRQn
#define I2C_LED5_Pin GPIO_PIN_7
#define I2C_LED5_GPIO_Port GPIOC
#define DP_EN_Pin GPIO_PIN_8
#define DP_EN_GPIO_Port GPIOC
#define LED_HB_Pin GPIO_PIN_12
#define LED_HB_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

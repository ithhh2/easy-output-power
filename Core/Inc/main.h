/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_gpio.h"

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
#define CFG1_Pin GPIO_PIN_0
#define CFG1_GPIO_Port GPIOF
#define CFG2_Pin GPIO_PIN_1
#define CFG2_GPIO_Port GPIOF
#define CFG3_Pin GPIO_PIN_1
#define CFG3_GPIO_Port GPIOA
#define CON_IO1_Pin GPIO_PIN_2
#define CON_IO1_GPIO_Port GPIOA
#define CON_IO2_Pin GPIO_PIN_3
#define CON_IO2_GPIO_Port GPIOA
#define CON_IO3_Pin GPIO_PIN_4
#define CON_IO3_GPIO_Port GPIOA
#define SW3_Pin GPIO_PIN_6
#define SW3_GPIO_Port GPIOA
#define SW3_EXTI_IRQn EXTI4_15_IRQn
#define SW2_Pin GPIO_PIN_7
#define SW2_GPIO_Port GPIOA
#define SW2_EXTI_IRQn EXTI4_15_IRQn
#define SW1_Pin GPIO_PIN_1
#define SW1_GPIO_Port GPIOB
#define SW1_EXTI_IRQn EXTI0_1_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

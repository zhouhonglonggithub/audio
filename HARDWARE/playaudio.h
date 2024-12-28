/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    playaudio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __PLAYAUDIO__
#define __PLAYAUDIO__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

void VS1003_PlayMusic_2(const char* filepPath);

#ifdef __cplusplus
}
#endif
#endif /*__ __PLAYAUDIO__ */



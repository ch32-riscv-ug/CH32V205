/********************************** (C) COPYRIGHT *******************************
 * File Name          : App.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2025/11/25
 * Description        : Touch Key Routines
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef APP_H_
#define APP_H_

#include "ch32v205.h"

// #define DEBUG 0

#if(DEBUG)
  #define PRINT(format, ...)    printf(format, ##__VA_ARGS__)
#else
  #define PRINT(X...)
#endif

/************************TOUCH_KEY_DEFINE****************************/
#define TOUCH_KEY_ELEMENTS            	    (TKY_MAX_QUEUE_NUM)
#define TOUCH_KEY_CHS                       0,1,2,3,4,5,6,7

extern volatile uint8_t timerFlag;

void TKY_Init(void);
void TKY_dataProcess(void);

#endif

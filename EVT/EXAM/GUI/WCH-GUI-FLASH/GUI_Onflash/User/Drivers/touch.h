/********************************** (C) COPYRIGHT  *******************************
* File Name          : touch.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2025/08/01
* Description        : This file provides some prototypes and defines .
 *********************************************************************************
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef __TOUCH_H
#define __TOUCH_H

#include "soft_iic.h"

#define TOUCH_MAX 5

typedef struct
{
    uint8_t  gesture; /* 0=none, 1=down, 2=up, 3=left, 4=right,
                           5=click, 0x0B=double-click, 0x0C=long-press */
    uint8_t  fingers; /* 0 or 1 */
    uint16_t x;       /* 0..359 */
    uint16_t y;       /* 0..359 */
} cst816d_touch_t;

void    Touch_Init(void);
void    Touch_Scan(void);

#endif

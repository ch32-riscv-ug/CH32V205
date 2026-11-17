/*********************************************************************
 * File Name          : qspi.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2026/08/12
 * Description        : QSPI driver for the 4-line (SIOX) LCD panel.
 *********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __QSPI_LCD_H
#define __QSPI_LCD_H

#include "debug.h"
#include "config.h"
#include "ch32v205_qspi.h"

#define QSPI_LCD_CS_RCC         RCC_PB2Periph_GPIOB
#define QSPI_LCD_CS_PORT        GPIOB
#define QSPI_LCD_CS_PIN         (GPIO_Pin_1)

#define QSPI_LCD_CS_SET()       (QSPI_LCD_CS_PORT->BSHR = QSPI_LCD_CS_PIN)
#define QSPI_LCD_CS_CLR()       (QSPI_LCD_CS_PORT->BCR = QSPI_LCD_CS_PIN)

#define QSPI_LCD_PRESCALER      (10u)
#define QSPI_LCD_FSIZE          (23u)

#define QSPI_LCD_INS_SINGLE     (0x02u)
#define QSPI_LCD_INS_QUAD       (0x32u)

#define QSPI_LCD_CMD_RAMWR      (0x2Cu)

void QSPI_LCD_Init(void);
void QSPI_LCD_Cmd(uint8_t cmd);
void QSPI_LCD_CmdData(uint8_t cmd, const uint8_t *data, uint16_t len);
void QSPI_LCD_CmdPixels(uint8_t cmd, const uint16_t *pixels, uint32_t count);

#endif

/*********************************************************************
 * File Name          : config.h
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/8/15
 * Description        : file for Config.
 *********************************************************************************
* Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __CONFIG_H__
#define __CONFIG_H__

/* SPI FLASH and LCD_SCREEN GPIO Definition */
#define FLASH_CS_PORT     GPIOA
#define FLASH_CS_RCC      RCC_PB2Periph_GPIOA
#define FLASH_CS_PIN      (GPIO_Pin_4)

#endif

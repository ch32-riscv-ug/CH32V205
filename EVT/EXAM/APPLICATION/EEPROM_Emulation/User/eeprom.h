/********************************** (C) COPYRIGHT *******************************
* File Name          : eeprom.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2026/10/27
* Description        : eeprom functions definetion.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef  __EEPROM_H
#define  __EEPROM_H

#include "stdio.h"

#define EEPROM_PAGE_SIZE               (4096)
#define FLASH_PAGE_SIZE                (256)

#define EEPROM_START_ADDRESS           ((uint32_t)(0x08028000))  
#define EEPROM_PART0_BASE_ADDRESS      (EEPROM_START_ADDRESS)
#define EEPROM_PART0_END_ADDRESS       (EEPROM_START_ADDRESS+EEPROM_PAGE_SIZE-1)

#define EEPROM_PART1_BASE_ADDRESS      (EEPROM_START_ADDRESS + EEPROM_PAGE_SIZE)
#define EEPROM_PART1_END_ADDRESS       (EEPROM_PART1_BASE_ADDRESS + EEPROM_PAGE_SIZE -1)

#define PAGE0      (0u)
#define PAGE1      (1u)

#define NO_VALID_PAGE         ((uint16_t)0xBBBB)

#define PAGE_STATE_EMPTY      ((uint16_t)0xFFFF)
#define PAGE_STATE_RECEIVE    ((uint16_t)0xEEEE)
#define PAGE_STATE_VALID      ((uint16_t)0x0000)

#define READ_VALID  (0u)
#define WRITE_VALID (1u)

#define PAGE_FULL             ((uint16_t)0xAAAA)
#define NB_VAR                (3u)

/* The chip only supports fast program/erase which works by a whole FLASH_PAGE_SIZE(256 bytes) page,
   so the records are packed by pages: the first page of each EEPROM page stores the page state,
   the remaining pages store the records. One record occupies EEPROM_RECORD_SIZE(4) bytes, the low
   half word is the variable data, the high half word is the virtual address. The virtual address
   must not be 0xFFFF. */
#define EEPROM_RECORD_SIZE             (4u)

uint16_t EEPROM_Init(void);
uint16_t EEPROM_Read(uint16_t VirtAddress, uint16_t* Data);
uint16_t EEPROM_Write(uint16_t VirtAddress, uint16_t Data);

#endif
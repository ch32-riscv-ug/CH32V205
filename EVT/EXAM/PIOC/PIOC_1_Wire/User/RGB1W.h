/********************************** (C) COPYRIGHT *******************************
 * File Name          : RGB1W.h
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2025/05/20
 * Description        : 1-wire example 1W-RGB, 1W-DS1820
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "ch32v205.h"
#include "ch32v205_pioc.h"

#define		RGB1W_SFR_ADDR	((uint8_t *)&(PIOC->D8_DATA_REG0))	// RGB1W data buffer start address for SFR mode
#define		RGB1W_SFR_SIZE	32						// RGB1W data buffer size for SFR mode
#define		RGB1W_RAM_ADDR	((uint8_t *)(PIOC_SRAM_BASE+0x600))	// RGB1W data buffer start address for RAM mode
#define		RGB1W_RAM_SIZE	((uint8_t *)(PIOC_SRAM_BASE+0x1000)-RGB1W_RAM_ADDR)	// RGB1W data buffer size for RAM mode

#define		RGB1W_CMD_RAM	0x70	// notice PIOC to send data from RAM

#define		RGB1W_COMMAND	(PIOC->D8_CTRL_WR)	// command input
// 1~RGB1W_SFR_SIZE: RGB SFR mode, 0x01-0x20: RGB with short data in SFR, low 7bits is total byte
// 0x41: start temperature convert
// 0x42: get temperature data
// 0x43: start temperature convert then get data

#define		CMD_T_START		0x41	// start temperature convert
#define		CMD_T_GET		0x42	// get temperature data
#define		CMD_T_STA_GET	0x43	// start temperature convert then get data

#define		RGB1W_ERR_OK	0		// error code for success
#define		RGB1W_ERR_CMD	1		// error code for command error
#define		RGB1W_ERR_PARA	2		// error code for parameter error
#define		RGB1W_ERR_OUTH	4		// error code for pin high at the end
#define		RGB1W_ERR_PINH	6		// error code for pin high at the start

extern	__IO	uint8_t		stat;

extern	const unsigned char PIOC_1W_CODE[] __attribute__((aligned (4)));

void PIOC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void RGB1W_Init ( void );

void RGB1W_SendSFR( uint16_t total_bytes, uint8_t *p_source_addr ,uint8_t mod);  //SFR mode for 1~32 bytes data

void RGB1W_SendRAM( uint16_t total_bytes, uint8_t *p_source_addr ,uint8_t mod);  //RAM mode for 1~2560 bytes data

uint8_t RGB1W_SendSFR_Wait( uint16_t total_bytes, uint8_t *p_source_addr ,uint8_t mod);  //SFR mode for 1~32 bytes data

uint8_t RGB1W_SendRAM_Wait( uint16_t total_bytes, uint8_t *p_source_addr ,uint8_t mod);  //RAM mode for 1~2560 bytes data

void RGB1W_Halt( void );  //halt/sleep PIOC

/********************************** (C) COPYRIGHT  *******************************
 * File Name          : flash.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2020/12/16
 * Description        : CH32V205 fast program
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "flash.h"
#include "string.h"

u32 Verify_buf[32];

/*********************************************************************
 * @fn      CH32_IAP_Program
 *
 * @brief   adr - the date address
 *          buf - the date buffer
 *
 * @return  none
 */
void CH32_IAP_Program(u32 adr, u32* buf) {

    FLASH_ROM_WRITE(adr,buf,256);
}




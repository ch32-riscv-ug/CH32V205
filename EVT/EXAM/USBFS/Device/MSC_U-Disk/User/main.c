/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2024/01/19
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/* @Note
 * UDisk Example:
 * This program provides examples of UDisk.Supports external SPI Flash and internal
 * Flash, selected by STORAGE_MEDIUM at SW_UDISK.h.
 *  */

#include "debug.h"
#include "ch32v205_usbfs_device.h"
#include "Internal_Flash.h"
#include "SPI_FLASH.h"
#include "SW_UDISK.h"
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(921600);
    
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
	Delay_Ms( 10 );
#if (STORAGE_MEDIUM == MEDIUM_SPI_FLASH)
    printf("USBD UDisk Demo\r\nStorage Medium: SPI FLASH \r\n");
    /* SPI flash init */
    FLASH_Port_Init( );
    /* FLASH ID check */
    FLASH_IC_Check( );
#elif (STORAGE_MEDIUM == MEDIUM_INTERAL_FLASH)
    printf("USBD UDisk Demo\r\nStorage Medium: INTERNAL FLASH \r\n");
    Flash_Sector_Count = IFLASH_UDISK_SIZE  / DEF_UDISK_SECTOR_SIZE;
    Flash_Sector_Size = DEF_UDISK_SECTOR_SIZE;
#endif

    /* Enable Udisk */
    Udisk_Capability = Flash_Sector_Count;
    Udisk_Status |= DEF_UDISK_EN_FLAG;

    /* Usb Init */
    USBFS_RCC_Init( );
    USBFS_Device_Init( ENABLE );
    NVIC_EnableIRQ(USBFS_IRQn);

	while(1)
	{
	    ;
	}
}

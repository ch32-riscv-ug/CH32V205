/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2025/05/20
 * Description        : 
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
  @Note
  (1),Example routine to emulate a simulate USB-CDC Device, USE USART2(PA2/PA3),
  Please note: This code uses the default serial port 1 for debugging,
  if you need to modify the debugging serial port, please do not use USART2
  (2),Merged HID devices, using endpoint 4 data downlink to fetch reverse upload;
*/

#include "debug.h"
#include "ch32v205_usbfs_device.h"

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
    RCC_Configuration( );

    /* Tim2 init */
    TIM2_Init( );

    /* Usart1 init */
    UART2_Init( 1, DEF_UARTx_BAUDRATE, DEF_UARTx_STOPBIT, DEF_UARTx_PARITY );

    /* USB20 device init */
    USBFS_RCC_Init( );
    USBFS_Device_Init( ENABLE );
    
    while(1)
    {
        UART2_DataRx_Deal( );
        UART2_DataTx_Deal( );
    }
}

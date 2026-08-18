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
 *@Note
  Example routine to emulate a custom USB device (CH372 device).
  This routine demonstrates the use of a USBFS Device to emulate a custom device, the CH372,
  with endpoints 1/3/5 downlinking data and uploading via endpoints 2/4/6 respectively
  Endpoint 1/2 uploads and downlinks via ring buffer with no data reversal, endpoints 3/4, and endpoints 5/6 copy and upload.
  The device can be operated using Bushund or other upper computer software.
  Note: This routine needs to be demonstrated in conjunction with the host software.

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
    printf("CH372Device Running On USBFS Controller\n");

	/* USBFSD device init */
	USBFS_RCC_Init( );
	USBFS_Device_Init( ENABLE );

    while(1)
    {
        /* Determine if enumeration is complete, perform data transfer if completed */
        if(USBFS_DevEnumStatus)
        {
            /* Data Transfer */
            if(RingBuffer_Comm.RemainPack)
            {
                if((USBFSD->UEP2_TX_CTRL & USBFS_UEP_T_RES_MASK) == USBFS_UEP_T_RES_NAK)
                {
                    memcpy(USBFSD_UEP_BUF(DEF_UEP2),&Data_Buffer[(RingBuffer_Comm.DealPtr) * DEF_USBD_FS_PACK_SIZE], RingBuffer_Comm.PackLen[RingBuffer_Comm.DealPtr]);
                    USBFSD->UEP2_TX_LEN = RingBuffer_Comm.PackLen[RingBuffer_Comm.DealPtr];
                    USBFSD->UEP2_TX_CTRL = (USBFSD->UEP2_TX_CTRL & ~USBFS_UEP_T_RES_MASK) | USBFS_UEP_T_RES_ACK;
                    
                    NVIC_DisableIRQ(USBFS_IRQn);
                    RingBuffer_Comm.RemainPack--;
                    RingBuffer_Comm.DealPtr++;
                    if(RingBuffer_Comm.DealPtr == DEF_Ring_Buffer_Max_Blks)
                    {
                        RingBuffer_Comm.DealPtr = 0;
                    }
                    NVIC_EnableIRQ(USBFS_IRQn);
                }
            }

            /* Monitor whether the remaining space is available for further downloads */
            if(RingBuffer_Comm.RemainPack < (DEF_Ring_Buffer_Max_Blks - DEF_RING_BUFFER_RESTART))
            {
                if(RingBuffer_Comm.StopFlag)
                {
                    RingBuffer_Comm.StopFlag = 0;
                    USBFSD->UEP1_RX_CTRL = (USBFSD->UEP1_RX_CTRL & ~USBFS_UEP_R_RES_MASK) | USBFS_UEP_R_RES_ACK;
                }
            }
        }
	}
}

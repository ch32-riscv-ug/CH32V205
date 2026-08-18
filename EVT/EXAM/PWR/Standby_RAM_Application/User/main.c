/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2025/12/15
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *when LV is not enabled in standby mode, RAM 4k and 28K low-power data holding routines:
 *This routine demonstrates writing data at the specified location of 4K RAM and 28K RAM,
 *and then WFI enters STANDBY sleep mode, through the PA0 (wakeup) pin Input high level
 *to exit standby mode, print RAM data after waking up, and test whether RAM is maintained
 *data.
 *
 */

#include "debug.h"

/* Global define */
#define _KEEP_4K_RAM    __attribute__((section(".keep_4k_ram"))) //space for keep 4K RAM.
#define _KEEP_28K_RAM   __attribute__((section(".keep_28k_ram"))) //space for keep 28K RAM.

_KEEP_4K_RAM    u32 DataBuf0[256];
_KEEP_28K_RAM   u32 DataBuf1[256];
/*********************************************************************
 * @fn      TestDataWrite
 *
 * @brief   Write data to certain address of RAM.
 *
 * @return  none
 */
void TestDataWrite(void)
{
    uint32_t i=0;
    /* Write data in 4K RAM */
    for( i=0; i<256; i++ )
    {
       DataBuf0[i] = 0x1234ABCD;
    }
    printf("Write data in 4K RAM Successfully!\r\n");

    /* Write data in 28K RAM */
    for( i=0; i<256; i++ )
    {
       DataBuf1[i] = 0x55555555;
    }
    printf("Write data in 28K RAM Successfully!\r\n");

}
/*********************************************************************
 * @fn      TestDataRead
 *
 * @brief   Print the data of certain address of RAM.
 *
 * @return  none
 */
void TestDataRead(void)
{
    uint32_t i=0;
    printf("Read data in 4K RAM:\r\n");
    for( i=0; i<256; i++ )
    {
        printf("DataBuf0#%d = 0x%08x\r\n",i,DataBuf0[i]);
    }

    printf("Read data in 28K RAM:\r\n");
    for( i=0; i<256; i++ )
    {
        printf("DataBuf1#%d = 0x%08x\r\n",i,DataBuf1[i]);
    }
}

/*********************************************************************
 * @fn      All_GPIO_IPD_Init
 *
 * @brief   To reduce power consumption, unused I/O is configured in pull-down input mode.
 *
 * @return  none
 */
void All_GPIO_IPD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA | RCC_PB2Periph_GPIOB | RCC_PB2Periph_GPIOC |
                          RCC_PB2Periph_GPIOD | RCC_PB2Periph_GPIOE | RCC_PB2Periph_AFIO, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init( GPIOA, &GPIO_InitStructure );
    GPIO_Init( GPIOB, &GPIO_InitStructure );
    GPIO_Init( GPIOC, &GPIO_InitStructure );
    GPIO_Init( GPIOD, &GPIO_InitStructure );
    GPIO_Init( GPIOE, &GPIO_InitStructure );
}

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
    Delay_Ms(1000);
    All_GPIO_IPD_Init();
    USART_Printf_Init(115200);
    Delay_Ms(500);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("Standby Mode Test\r\n");
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);
    
    Delay_Ms(1000);

    while(1)
    {
        TestDataRead();
        printf("RAM Mode Test\r\n");
        TestDataWrite();
        PWR_WakeUpPinCmd(ENABLE);
        PWR_EnterSTANDBYMode_RAM();
        printf("\r\n ########## \r\n");
    }
}

/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2025/05/20
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


/*********************************************************************
 * @fn      TestDataWrite
 *
 * @brief   Write data to certain address of 4K RAM and 28K RAM.
 *
 * @return  none
 */
void TestDataWrite(void)
{
    uint32_t myAddr=0x20000000+4*1024;
    uint32_t myAddr2=0x20000000+3*1024;
    uint32_t i=0;
    for(i=0;i<5120;i++)//Write 28K RAM
    {
        *(uint32_t volatile *)(myAddr+(i<<2)) = 0x33333333;
    }
    for(i=0;i<5120;i++)//Check 28K RAM
    {
        if((*(uint32_t volatile *)(myAddr+(i<<2))) != 0x33333333)
        {
            printf("Write 28K RAM Error!\r\n");
            break;
        }
    }
    printf("Write 28K RAM Successfully!\r\n");
    for(i=0;i<200;i++)//Write 4K RAM
    {
        *(uint32_t volatile *)(myAddr2+(i<<2)) = 0x55555555;
    }
    for(i=0;i<200;i++)//Check 4K RAM
    {
        if((*(uint32_t volatile *)(myAddr2+(i<<2))) != 0x55555555)
        {
            printf("Write 4K RAM Error!\r\n");
            break;
        }
    }
    printf("Write 4K RAM Successfully!\r\n");

}


/*********************************************************************
 * @fn      TestDataRead
 *
 * @brief   Print the data of certain address of 4K RAM and 28K RAM.
 *
 * @return  none
 */
void TestDataRead(void)
{
    uint32_t myAddr=0x20000000+4*1024;
    uint32_t myAddr2=0x20000000+3*1024;
    uint32_t i=0;
    printf("4K RAM:\r\n");
    for(i=0;i<200;i++)
    {
        printf("0x%08x=0x%08x\r\n",myAddr2+(i<<2),*(uint32_t volatile *)(myAddr2+(i<<2)));
    }
    printf("28K RAM:\r\n");
    for(i=0;i<5120;i++)
    {
        printf("0x%08x=0x%08x\r\n",myAddr+(i<<2),*(uint32_t volatile *)(myAddr+(i<<2)));
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
        printf("RAM LV Mode Test\r\n");
        TestDataWrite();
        PWR_WakeUpPinCmd(ENABLE);
        PWR_EnterSTANDBYMode_RAM();
        printf("\r\n ########## \r\n");
    }
}

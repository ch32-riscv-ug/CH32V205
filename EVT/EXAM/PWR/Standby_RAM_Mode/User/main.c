/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2025/10/14
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

#define RAM_BASE            0x20000000U
#define RAM4K_BASE          (RAM_BASE)                
#define RAM28K_BASE         (RAM_BASE + 4U * 1024U)    
#define RAM28K_SIZE         (28U * 1024U)


#define RAM28K_TEST_ADDR    (RAM_BASE + 4U * 1024U) 
#define RAM28K_TEST_WORDS   1048U            
#define RAM4K_TEST_ADDR     (RAM_BASE + 3U * 1024U)
#define RAM4K_TEST_WORDS    200U 

#define RAM_TEST_OK      0U
#define RAM_TEST_FAIL    1U

#define RAM_TEST_PATTERN    0x5555FFFFU
/*********************************************************************
 * @fn      TestDataWrite
 *
 * @brief   Write data to certain address of 4K RAM and 28K RAM.
 *
 * @return  none
 */
uint32_t TestDataWrite(void)
{
    uint32_t i=0;
    uint32_t myAddr = RAM28K_TEST_ADDR;
    uint32_t myAddr2 = RAM4K_TEST_ADDR;
    
    for(i=0; i<RAM28K_TEST_WORDS; i++)//Write 28KRAM
    {
        *(uint32_t volatile *)(myAddr+(i<<2)) = RAM_TEST_PATTERN;
    }
    for(i=0; i<RAM28K_TEST_WORDS; i++)//Check 28KRAM
    {
        if((*(uint32_t volatile *)(myAddr+(i<<2))) != RAM_TEST_PATTERN)
        {
            printf("28K RAM write error at 0x%08x\r\n", (unsigned)(myAddr + (i << 2)));
            return RAM_TEST_FAIL;
        }
    }
    printf("Write 28K RAM Successfully!\r\n");

    for(i=0; i<RAM4K_TEST_WORDS; i++)//Write 4KRAM
    {
        *(uint32_t volatile *)(myAddr2+(i<<2)) = RAM_TEST_PATTERN;
    }
    for(i=0; i<RAM4K_TEST_WORDS; i++)//Check 4KRAM
    {
        if((*(uint32_t volatile *)(myAddr2+(i<<2))) != RAM_TEST_PATTERN)
        {
            printf("4K RAM write error at 0x%08x\r\n", (unsigned)(myAddr2 + (i << 2)));
            return RAM_TEST_FAIL;
        }
    }
    printf("Write 4K RAM Successfully!\r\n");
    return RAM_TEST_OK;
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
    uint32_t i=0;

    printf("4K RAM:\r\n");
    for(i=0; i<RAM4K_TEST_WORDS; i++)
    {
        printf("0x%08x=0x%08x\r\n", (unsigned)(RAM4K_TEST_ADDR + (i << 2)),
               (unsigned)*(uint32_t volatile *)(RAM4K_TEST_ADDR + (i << 2)));
    }

    printf("28K RAM:\r\n");
    for(i=0; i<RAM28K_TEST_WORDS; i++)
    {
        printf("0x%08x=0x%08x\r\n", (unsigned)(RAM28K_TEST_ADDR + (i << 2)),
               (unsigned)*(uint32_t volatile *)(RAM28K_TEST_ADDR + (i << 2)));
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
    printf("SystemClk:%u\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("Standby Mode Test\r\n");
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);
    
    Delay_Ms(1000);

    while(1)
    {
        TestDataRead();
        printf("RAM LV Mode Test\r\n");
        if(TestDataWrite() != RAM_TEST_OK)
        {
            printf("RAM write/verify failed\r\n");
        }
        PWR_WakeUpPinCmd(ENABLE);
        PWR_EnterSTANDBYMode_RAM();
    }
}

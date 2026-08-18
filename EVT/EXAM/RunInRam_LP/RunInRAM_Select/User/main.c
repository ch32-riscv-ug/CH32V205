/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.1
 * Date               : 2025/12/10
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *RunInRAM_Select:
 * This routine demonstrates that some programs run in RAM, which is faster than running in FLASH.
 * If you want the function run in RAM,you need to add prefix __attribute__((section(".highcode")))
 */

#include "debug.h"

/* Global define */

/* Global Variable */

const u8 tp[10] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};

/*********************************************************************
 * @fn      FLASH_LP_Enter
 *
 * @brief   Enter FLASH Low power mode
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void FLASH_LP_Enter(void)
{
    uint32_t tmpreg = 0;
    /* FLASH low power mode  */
    RCC->PB1PCENR |= RCC_PB1Periph_PWR;
    tmpreg = PWR->CTLR;
    tmpreg &= ~(7 << 9);
    tmpreg |= (1 << 11) | (1 << 9);
    PWR->CTLR = tmpreg;
}

/*********************************************************************
 * @fn      FLASH_LP_Exit
 *
 * @brief   Exit FLASH Low power mode
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void FLASH_LP_Exit(void)
{
    RCC->PB1PCENR |= RCC_PB1Periph_PWR;
    PWR->CTLR &= ~(1 << 9);
}

/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0
 *
 * @return  none
 */
__attribute__((section(".highcode")))
void GPIO_Toggle_INIT(void)
{
    FLASH_LP_Enter();

    //PA0
    RCC->PB2PCENR |= RCC_PB2Periph_GPIOA;
    GPIOA->CFGLR &= ~0x0000000F;
    GPIOA->CFGLR |= 3;

    FLASH_LP_Exit();
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
    vu8 i = 0;

    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("EXTEN->CTLR0-%08x\r\n", EXTEN->CTLR0);

    GPIO_Toggle_INIT();

    for(i=0; i<10; i++){
        printf("-%02x\r\n", tp[i]);
    }

    while(1)
    {
        Delay_Ms(1000);
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, (i == 0) ? (i = Bit_SET) : (i = Bit_RESET));
    }
}

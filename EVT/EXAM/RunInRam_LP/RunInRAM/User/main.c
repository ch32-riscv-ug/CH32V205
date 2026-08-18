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
 *RunInRAM:
 *This routine runs in RAM
 *
 */
#include "debug.h"

/* Global define */

/* Global Variable */

const u8 tp[10] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};
vu32 temp = 0;
/*********************************************************************
 * @fn      GPIO_Toggle_INIT
 *
 * @brief   Initializes GPIOA.0
 *
 * @return  none
 */
void GPIO_Toggle_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      LDO_Deal
 *
 * @brief   Deal LDO mode(reduced power).
 *
 * @return  none
 */
void LDO_Deal(void)
{
    /* LDOTRIM[1:0] == 01b */
    uint32_t tmp = 0;
    tmp = EXTEN->CTLR0;
    tmp &= ~EXTEN_LDO_TRIM;
    tmp |= EXTEN_LDO_TRIM1;
    EXTEN->CTLR0 = tmp;
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
    SystemInit();
    vu8 i = 0;
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("Run in RAM\r\n");
    Delay_Ms(1000);

    /* Adjusting LDO */
    LDO_Deal();

    /* FLASH low power mode*/
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);
    PWR_FLASH_LP_Cmd(ENABLE);

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

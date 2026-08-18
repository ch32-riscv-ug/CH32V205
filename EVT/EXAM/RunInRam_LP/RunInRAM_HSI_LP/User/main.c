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
 *RunInRAM_HSI_LP:
 *This routine shows the method of reducing power consumption.
 *1.The program in RAM.FLASH is in Low_power Mode,and not used PLL.
 *2.HSI as the system frequency,and HSI is in Low_power Mode(HSI is 1MHz when use UART,the Baud supports less than 62.5Kbps)
 *3.adjust LDO TRIM to reduce power consumption.(when run in FLASH and use PLL, this mode is not supported)
 * note - All three points above are selected by default, and users can choose according to need.
 *
 */

#include "debug.h"

/* Global define */

/* Global Variable */
const u8 tp[10] = {0x01,0x02,0x01,0x02,0x01,0x02,0x01,0x02,0x01,0x02};

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
    //reduced power
    /* LDOTRIM[1:0] == 01'b */
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
    USART_Printf_Init(56000);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("Run in RAM and HSI Low power mode\r\n");
    Delay_Ms(1000);
    /* Adjusting LDO reduced power */
    LDO_Deal();
    /* FLASH low power mode reduced power */
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);
    PWR_FLASH_LP_Cmd(ENABLE);

    GPIO_Toggle_INIT();

    while(1)
    {
        Delay_Ms(1000);
        GPIO_WriteBit(GPIOA, GPIO_Pin_0, (i == 0) ? (i = Bit_SET) : (i = Bit_RESET));
    }
}

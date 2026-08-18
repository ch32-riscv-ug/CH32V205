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
 *OPA1 is used as a voltage follower output, the external voltage
 *is output to OPA1
 *OPA1_CHP0--PB5
 *OPA1_CHN0--PB11
 *OPA1_OUT--PB1
 *
 *In this example, PB11 and PB1 are short-circuited, and the external voltage is input from PB11.
 *If the negative feedback is connected to a resistor, it can also form an operational amplifier.
 */

#include "debug.h"

/* Global Variable */

/*********************************************************************
 * @fn      OPA1_Init
 *
 * @brief   Initializes OPA1.
 *
 * @return  none
 */
void OPA1_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    OPA_InitTypeDef  OPA_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_11|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    OPA_InitStructure.Mode = OUT_IO_OUT0;
    OPA_InitStructure.PSEL = CHP0;
    OPA_InitStructure.NSEL = CHN0;
    OPA_Init(OPA1,&OPA_InitStructure);
    OPA_Cmd( OPA1,ENABLE );

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
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    OPA_Unlock();
    OPA1_Init();

    while(1);

}


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

/**
 * @note
 * The code initializes the power voltage detector (PVD) and enters low-power stop mode, waking up when
 * the PVD interrupt is triggered.
 */

#include "debug.h"

/* Global define */

/* Global Variable */

void PVD_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void PVD_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line16) != RESET)
    {
        SystemInit();
        printf("Run at EXTI\r\n");
        EXTI_ClearITPendingBit(EXTI_Line16);
    }
}

/*********************************************************************
 * @fn       PVD_Init
 *
 * @brief    The function initializes the power voltage detector (PVD) by configuring the necessary registers and
 *          enabling the PVD interrupt.
 *
 * @return   none
 */
void PVD_Init(void)
{
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO, ENABLE);

    EXTI_InitTypeDef EXIT_InitStructure = {0};

    EXIT_InitStructure.EXTI_Line = EXTI_Line16;
    EXIT_InitStructure.EXTI_LineCmd = ENABLE;
    EXIT_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXIT_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&EXIT_InitStructure);

    NVIC_EnableIRQ(PVD_IRQn);
    NVIC_SetPriority(PVD_IRQn, 2);

    PWR_PVDLevelConfig(PWR_PVDLevel_7);

    PWR_PVDCmd(ENABLE);
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
    Delay_Ms(100);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    printf("PVD wakeup TEST\r\n");
    /*Leave 1s for user to debug the chip*/
    Delay_Ms(1000);
    /* The code is initializing the power voltage detector (PVD) and 
    then entering the low-power stop mode. */
    PVD_Init();
    printf("Fall asleep...\r\n");
    Delay_Ms(100);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    printf("Wake up!!\r\n");

    while (1)
    {
        Delay_Ms(250);
        printf("run in main\r\n");
    }
}

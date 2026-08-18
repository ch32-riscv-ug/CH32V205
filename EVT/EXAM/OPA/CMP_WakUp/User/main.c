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
 *Comparator wakeup routines:
 *CMP1_P0--PA2
 *CMP1_N0--PB1
 *CMP1_OUT--PB12
 *In this example, PA2 and PB1 are postive and negative input, PB12 is the output of the comparator.
 *When the voltage of PA2 is greater than the voltage of PB1, the output of PB12 is high level,
 *otherwise the output of PB12 is low level.when output is high stop wake will be performed.
 * 
 */
#include "debug.h"

/*********************************************************************
 * @fn      CMP_Init
 *
 * @brief   Initializes CMP.
 *
 * @return  none
 */
void CMP_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    CMP_InitTypeDef  CMP_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA|RCC_PB2Periph_GPIOB|RCC_PB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    CMP_InitStructure.CMP_Out_Mode = OUT_IO0;
    CMP_InitStructure.NSEL = CMP_CHN0;
    CMP_InitStructure.PSEL = CMP_CHP0;
    CMP_InitStructure.HYS = CMP_HYS_Mode2;
    OPA_CMP_Init(CMP1,&CMP_InitStructure);
    OPA_CMP_Cmd(CMP1,ENABLE);
    OPA_CMP_SystemReset_Cmd(CMP1,DISABLE);
    OPA_CMP_WakeUp_ModeConfig(CMP_WakeUp_CMP1);
}

/*********************************************************************
 * @fn      CMP_OUT
 *
 * @brief   CMP out program.
 *
 * @return  none
 */
void CMP_OUT( void )
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_PB2PeriphClockCmd( RCC_PB2Periph_GPIOB|RCC_PB2Periph_AFIO, ENABLE );
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF1);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*********************************************************************
 * @fn      EXTI_Event_Init
 *
 * @brief   Initializes EXTI.
 *
 * @return  none
 */
void EXTI_Event_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA | RCC_PB2Periph_AFIO, ENABLE);

    EXTI_InitStructure.EXTI_Line = EXTI_Line22;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    NVIC_EnableIRQ(CMPWakeUp_IRQn);
    NVIC_SetPriority(CMPWakeUp_IRQn,2);

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
    Delay_Init();
    SystemCoreClockUpdate();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR, ENABLE);
    PWR_PVDCmd(ENABLE);
    OPA_Unlock();
    OPA_CMP_Unlock();
    CMP_Init();
    CMP_OUT();
    EXTI_Event_Init();
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

    while(1)
    {
        Delay_Ms(1000);
        printf("Run in main\r\n");
    }
 
}


void CMPWakeUp_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      CMPWakeUp_IRQHandler
 *
 * @brief   This function handles CMPWakeUp exception.
 *
 * @return  none
 */
void CMPWakeUp_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line22)!=RESET)
  {
	SystemInit();
    printf("EXTI Wake_up\r\n");
    EXTI_ClearITPendingBit(EXTI_Line22);     /* Clear Flag */
  }
}

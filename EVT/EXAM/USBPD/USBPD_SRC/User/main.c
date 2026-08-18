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
 *
 * PD SRC Sample code
 *
 * This sample code may have compatibility issues and is for learning purposes only.
 *
 * Be sure to remove the pull-down resistors on both CC wires when using this Sample code!
 *
 * The inability to control the VBUS voltage on the board may lead to some compatibility problems,
 * mainly manifested in the inability of some devices to complete the PD communication process.
 *
 *Please keep HCLK_Frequency not beyond 150MHz.
 */

#include "debug.h"
#include "PD_Process.h"

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#if(Wake_up_mode==USBPDWake_up)
void USBPDWakeUp_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#elif(Wake_up_mode==GPIOWake_up)
void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#endif
volatile UINT8  Tim_Ms_Cnt = 0x00;

/*********************************************************************
 * @fn      TIM1_Init
 *
 * @brief   Initialize TIM1
 *
 * @return  none
 */
void TIM1_Init( u16 arr, u16 psc )
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_TIM1, ENABLE );
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
    TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);
    TIM_ClearITPendingBit( TIM1, TIM_IT_Update );
    NVIC_SetPriority(TIM1_UP_IRQn,0);
    NVIC_EnableIRQ(TIM1_UP_IRQn);

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM_Cmd( TIM1, ENABLE );
}

/*********************************************************************
 * @fn      EXTI_INIT
 *
 * @brief   Initialize Wake up EXTI
 *
 * @return  none
 */
void EXTI_INIT(void)
{
#if(Wake_up_mode==USBPDWake_up)
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    EXTI_InitStructure.EXTI_Line = EXTI_Line19;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
#elif(Wake_up_mode==GPIOWake_up)
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    /* GPIOA.0 ----> EXTI_Line0 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* GPIOA.1 ----> EXTI_Line1 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
#endif

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
    USART_Printf_Init(921600);
    printf( "SystemClk:%d\r\n", SystemCoreClock );
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    printf( "PD SRC TEST\r\n" );
    PD_Init( );
    EXTI_INIT();
    TIM1_Init( 999, 40-1);
    while(1)
    {
        /* Get the calculated timing interval value */
        TIM_ITConfig( TIM1, TIM_IT_Update , DISABLE );
        Tmr_Ms_Dlt = Tim_Ms_Cnt - Tmr_Ms_Cnt_Last;
        Tmr_Ms_Cnt_Last = Tim_Ms_Cnt;
        TIM_ITConfig( TIM1, TIM_IT_Update , ENABLE );
        PD_Ctl.Det_Timer += Tmr_Ms_Dlt;
        if( PD_Ctl.Det_Timer > 4 )
        {
            PD_Ctl.Det_Timer = 0;
            PD_Det_Proc( );
        }
        PD_Main_Proc( );
    }
}

/*********************************************************************
 * @fn      TIM1_UP_IRQHandler
 *
 * @brief   This function handles TIM1 interrupt.
 *
 * @return  none
 */
void TIM1_UP_IRQHandler(void)
{
    if( TIM_GetITStatus( TIM1, TIM_IT_Update ) != RESET )
    {
        Tim_Ms_Cnt++;
        TIM_ClearITPendingBit( TIM1, TIM_IT_Update );
    }
}

/*********************************************************************
 * @fn      EXTI0_IRQHandler
 *
 * @brief   This function handles EXTI0 exception.
 *
 * @return  none
 */
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
  {
      SystemInit();
      printf(" GPIO Wake_up\r\n");
      EXTI_ClearITPendingBit(EXTI_Line0);     /* Clear Flag */
      NVIC_DisableIRQ(EXTI0_IRQn);
  }

}

/*********************************************************************
 * @fn      EXTI1_IRQHandler
 *
 * @brief   This function handles EXTI1 exception.
 *
 * @return  none
 */
void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1)!=RESET)
  {
      SystemInit();
      printf(" GPIO Wake_up\r\n");
      EXTI_ClearITPendingBit(EXTI_Line1);     /* Clear Flag */
      NVIC_DisableIRQ(EXTI1_IRQn);
    }

}

/*********************************************************************
 * @fn      USBPDWakeUp_IRQHandler
 *
 * @brief   This function handles USBPD WakeUp exception.
 *
 * @return  none
 */
void USBPDWakeUp_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line19)!=RESET)
  {
      SystemInit();
      EXTI_ClearITPendingBit(EXTI_Line19);     /* Clear Flag */
      NVIC_DisableIRQ(USBPDWakeUp_IRQn);
      USBPD->CONFIG&=~IE_PD_IO;
      printf("USBPDWakeUp\r\n");
      USBPD->PORT_CC1&=~(CC_PU_Mask);
      USBPD->PORT_CC2&=~(CC_PU_Mask);
      USBPD->PORT_CC1|=CC_PU_330;
      USBPD->PORT_CC2|=CC_PU_330;
  }
}

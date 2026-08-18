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
 *Comparator routines:
 *CMP1_P0--PA2
 *CMP1_N0--PB1
 *CMP1_OUT--PB12  TIM4_CH4
 *In this example, PA2 and PB1 are postive and negative input, PB12 is the output of the comparator.
 *When the voltage of PA2 is greater than the voltage of PB1, the output of PB12 is high level,
 *otherwise the output of PB12 is low level.or TIM4_CH4 can capture the CMP OUT.
 */
#include "debug.h"

#define OUT_IO      0
#define OUT_TIM     1

// #define CMP_Mode_Out   OUT_IO
#define CMP_Mode_Out   OUT_TIM

/* CMP Reset Enable Definition */
#define CMP_RST_ENABLE   0
#define CMP_RST_DISABLE  1

/* CMP Reset Enable Selection */
#define CMP_RST   CMP_RST_DISABLE
// #define CMP_RST   CMP_RST_ENABLE

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

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB|RCC_PB2Periph_GPIOA|RCC_PB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#if  (CMP_Mode_Out == OUT_IO)
    CMP_InitStructure.CMP_Out_Mode = OUT_IO0;
#else
     CMP_InitStructure.CMP_Out_Mode = OUT_IO_TIM_Mode3;
#endif
    CMP_InitStructure.NSEL = CMP_CHN0;
    CMP_InitStructure.PSEL = CMP_CHP0;
    CMP_InitStructure.HYS = CMP_HYS_Mode1;
    OPA_CMP_Init(CMP1,&CMP_InitStructure);
    OPA_CMP_Cmd(CMP1,ENABLE);
#if  (CMP_RST == WDT_RST_ENABLE)
    OPA_CMP_SystemReset_Cmd(CMP1,ENABLE);
#else
    OPA_CMP_SystemReset_Cmd(CMP1,DISABLE);
#endif
 
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
 * @fn      Input_Capture_Init
 *
 * @brief   Initializes TIM4 input capture.
 *
 * @param   arr - the period value.
 *          psc - the prescaler value.
 *          ccp - the pulse value.
 *
 * @return  none
 */
void Input_Capture_Init( u16 arr, u16 psc )
{
	TIM_ICInitTypeDef TIM_ICInitStructure={0};
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC_PB1PeriphClockCmd(RCC_PB1Periph_TIM4, ENABLE);

	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter =  0x00;
	TIM_TimeBaseInit( TIM4, &TIM_TimeBaseInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x00;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;

	TIM_ICInit( TIM4, &TIM_ICInitStructure );

    NVIC_EnableIRQ(TIM4_IRQn);
	TIM_ITConfig( TIM4, TIM_IT_CC4, ENABLE );

	TIM_SelectInputTrigger( TIM4, TIM_TS_TI1FP1 );
	TIM_SelectSlaveMode( TIM4, TIM_SlaveMode_Reset );
	TIM_SelectMasterSlaveMode( TIM4, TIM_MasterSlaveMode_Enable );
	TIM_Cmd( TIM4, ENABLE );
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
    OPA_Unlock();
    OPA_CMP_Unlock();
    CMP_Init();

#if  (CMP_Mode_Out == OUT_IO)
       CMP_OUT();
#else
       Input_Capture_Init(0XFFFF,48);
#endif
    while(1);
 
}

void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      TIM4_IRQHandler
 *
 * @brief   This function handles TIM4  Capture Compare Interrupt exception.
 *
 * @return  none
 */
void TIM4_IRQHandler(void)
{
	if( TIM_GetITStatus( TIM4, TIM_IT_CC4 ) != RESET )
	{
		printf( "TIM4_Val:%d\r\n", TIM_GetCapture4( TIM4 ) );
	}

	TIM_ClearITPendingBit( TIM4, TIM_IT_CC4  );
}


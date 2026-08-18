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
 *OPA1_CHN0--PGA_16
 *OPA1_OUT--CMP1_P0
 *P0-PB5 P1-PB0  P2-PB12
 *
 *CMP1_N0--CMP_DAC
 *CMP1_OUT--TIM1-CH4
 *The number of positive terminals polling is 3. The polling time is 1000ms, OPA post 
 *channel can be queried when CMP output is high voltage.
 */
#include "debug.h"

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

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_12|GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    OPA_InitStructure.Mode = OUT_IO_OUT1;
    OPA_InitStructure.POLL_NUM = CHP_POLL_NUM_3;
    OPA_InitStructure.PSEL_POLL = CHP_OPA_POLL_ON;
    OPA_InitStructure.POLL_CH1 = OPA_POLL_CH1_PB5;
    OPA_InitStructure.POLL_CH2 = OPA_POLL_CH2_PB0;
    OPA_InitStructure.POLL_CH3 = OPA_POLL_CH3_PB12;
    OPA_InitStructure.POLL_Mode = OPA_POLL_Mode_NoSingle;
    OPA_InitStructure.OUT_IE=OUT_IE_ON;
    OPA_InitStructure.SampleSource=OPA_SampleSource_CMP1;
    OPA_InitStructure.FB = FB_ON;
    OPA_InitStructure.PGA_SEL=CHN_PGA_16xIN;
    
    OPA_InitStructure.SETUP_Time = 0x1f;
    OPA_InitStructure.OPA_HS = HS_ON;
    OPA_InitStructure.POLL_SEL = OPA_POLL_SEL_TIM3_CH4;

    OPA_Init(OPA1,&OPA_InitStructure);

    OPA_Cmd( OPA1,ENABLE );

    NVIC_EnableIRQ(OPA_IRQn);
    NVIC_SetPriority(OPA_IRQn,0x80);

}

/*********************************************************************
 * @fn      CMP_Init
 *
 * @brief   Initializes CMP.
 *
 * @return  none
 */
void CMP_Init( void )
{

    CMP_InitTypeDef  CMP_InitStructure = {0};

    CMP_InitStructure.CMP_Out_Mode = OUT_IO_TIM_Mode1;
    CMP_InitStructure.NSEL = CMP_CHN2;
    CMP_InitStructure.PSEL = CMP_CHP2;
    CMP_InitStructure.HYS = CMP_HYS_Mode2;
    OPA_CMP_Init(CMP1,&CMP_InitStructure);
    OPA_CMP_DAC_Cmd(ENABLE);
    OPA_CMP_DAC_DataConfig(3);
    OPA_CMP_Cmd(CMP1,ENABLE);
}

/*********************************************************************
 * @fn      TIM3_PWM_In
 *
 * @brief   Initializes the trigger input source for Timer 3 to an update event.
 *
 * @param   arr - the period value.
 *          psc - the prescaler value.
 *          ccp - the pulse value.
 *
 * @return  none
 */
void TIM3_PWM_In(u16 arr, u16 psc, u16 ccp)
{

    TIM_OCInitTypeDef       TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_PB1PeriphClockCmd( RCC_PB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccp;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);

    TIM_CtrlPWMOutputs(TIM3, ENABLE);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
    TIM_Cmd(TIM3, ENABLE);
}

/*********************************************************************
 * @fn      ADC_Function_Init
 *
 * @brief   Initializes ADC collection.
 *
 * @return  none
 */
void ADC_Function_Init(void)
{
    ADC_InitTypeDef  ADC_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig( RCC_PCLK2_Div8 );

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigInjecConv_Ext_IT15_TKEY_OPA;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
  
    ADC_ExternalTrigInjecConv_Ext_IT15_TKEY_OPA_Config(ADC1, ADC_ExternalTrigInjecConv_Ext_IT15_OPA);
    ADC_OPAPollTrigInjecConvCmd(ADC1,ENABLE);
    ADC_InjectedSequencerLengthConfig(ADC1, 3);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_CyclesMode0);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_CyclesMode0);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_CyclesMode0);
    ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE);

}

/*********************************************************************
 * @fn      Input_Capture_Init
 *
 * @brief   Initializes TIM1 input capture.
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

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_TIM1, ENABLE);

	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter =  0x00;
	TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x00;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;

	TIM_ICInit( TIM1, &TIM_ICInitStructure );

    NVIC_EnableIRQ(TIM1_CC_IRQn);
	TIM_ITConfig( TIM1, TIM_IT_CC4, ENABLE );
    NVIC_SetPriority(TIM1_CC_IRQn,0x00);
	TIM_SelectInputTrigger( TIM1, TIM_TS_TI1FP1 );
	TIM_SelectSlaveMode( TIM1, TIM_SlaveMode_Reset );
	TIM_SelectMasterSlaveMode( TIM1, TIM_MasterSlaveMode_Enable );
	TIM_Cmd( TIM1, ENABLE );
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
    OPA1_Init();
    ADC_Function_Init();
    TIM3_PWM_In(1000-1,SystemCoreClock/1000,500);
    Input_Capture_Init(0xffff,SystemCoreClock/1000);
    while(1);

}


void OPA_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void OPA_IRQHandler(void)
{

   if(OPA1_GetFlagStatus( OPA_FLAG_OUT_POLL_CH_1))
    {
        printf("OPA1_CH1\r\n");
    }
   if(OPA1_GetFlagStatus( OPA_FLAG_OUT_POLL_CH_2))
    {
        printf("OPA1_CH2\r\n"); 
    }
    if(OPA1_GetFlagStatus( OPA_FLAG_OUT_POLL_CH_3))
    {
        printf("OPA1_CH3\r\n");
    }
    OPA1_ClearFlag( OPA_FLAG_OUT_POLL_CH_3);
    OPA1_ClearFlag( OPA_FLAG_OUT_POLL_CH_2);
    OPA1_ClearFlag( OPA_FLAG_OUT_POLL_CH_1);
}

void TIM1_CC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_CC_IRQHandler(void)
{
	if( TIM_GetITStatus( TIM1, TIM_IT_CC4 ) != RESET )
	{
		printf( "TIM1_Val:%d\r\n", TIM_GetCapture4( TIM1 ) );
	}
	TIM_ClearITPendingBit( TIM1, TIM_IT_CC4  );
}


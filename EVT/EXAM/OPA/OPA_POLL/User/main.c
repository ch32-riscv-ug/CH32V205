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
 *OPA positive input polling function:
 *
 *OPA1_CHN0--PB11
 *OPA1_OUT--PA3
 *P0-PB5 P1-PB0  P2-PB12
 *The number of positive terminals polling is 3. The polling time is 1000ms, and the corresponding 
 *channel can be queried when the output is high voltage. OPA can be selected as the brake signal,
 *and the brake is effective when the output is high.
 */

#include "debug.h"

/* Global Variable */
#define  Break_ENABLE  0
#define  Break_DISABLE   1

// #define  OPA_Break  Break_DISABLE
#define  OPA_Break  Break_ENABLE
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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init( GPIOB, &GPIO_InitStructure );

    OPA_InitStructure.Mode = OUT_IO_OUT1;
    OPA_InitStructure.POLL_NUM = CHP_POLL_NUM_3;
    OPA_InitStructure.PSEL_POLL = CHP_OPA_POLL_ON;
    OPA_InitStructure.POLL_CH1 = OPA_POLL_CH1_PB5;
    OPA_InitStructure.POLL_CH2 = OPA_POLL_CH2_PB0;
    OPA_InitStructure.POLL_CH3 = OPA_POLL_CH3_PB12;
    OPA_InitStructure.POLL_Mode = OPA_POLL_Mode_NoSingle;
    OPA_InitStructure.CNT_IE=CNT_IE_ON;
    
    OPA_InitStructure.NSEL = CHN0;
    OPA_InitStructure.SETUP_Time = 0x1f;
    OPA_InitStructure.OPA_HS = HS_ON;
    OPA_InitStructure.POLL_SEL = OPA_POLL_SEL_TIM3_CH4;
    OPA_InitStructure.OPA_HS = HS_ON;
    OPA_Init(OPA1,&OPA_InitStructure);

    OPA_Cmd( OPA1,ENABLE );
    #if(OPA_Break == Break_ENABLE)
    OPA_CMP_TIM1_BKINConfig( TIM1_Brake_Source_OPA1 );
    #endif
    NVIC_EnableIRQ(OPA_IRQn);
    NVIC_SetPriority(OPA_IRQn,0);

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
    RCC_ADCCLKConfig( RCC_PCLK2_Div4 );

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
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_CyclesMode1);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_CyclesMode1);
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_CyclesMode1);
    ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);
    NVIC_EnableIRQ(ADC_IRQn);
    NVIC_SetPriority(ADC_IRQn,0xe0);

    ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);

    ADC_Cmd(ADC1, ENABLE);
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
    TIM_Cmd(TIM3, ENABLE);
}

/*********************************************************************
 * @fn      TIM1_Dead_Time_Init
 *
 * @brief   Initializes TIM1 complementary output and dead time.
 *
 * @param   arr - the period value.
 *          psc - the prescaler value.
 *          ccp - the pulse value.
 *
 * @return  none
 */
void TIM1_Dead_Time_Init( u16 arr, u16 psc, u16 ccp )
{
	GPIO_InitTypeDef GPIO_InitStructure={0};
	TIM_OCInitTypeDef TIM_OCInitStructure={0};
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure={0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_TIM1|RCC_PB2Periph_AFIO|RCC_PB2Periph_GPIOB|RCC_PB2Periph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF0);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF0);

	/* TIM1_CH1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	/* TIM1_CH1N */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
	GPIO_Init( GPIOA, &GPIO_InitStructure );

	TIM_TimeBaseInitStructure.TIM_Period = arr;
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = ccp;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	TIM_OC1Init( TIM1, &TIM_OCInitStructure );

    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
	TIM_BDTRInitStructure.TIM_DeadTime = 0xFF;
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
	TIM_BDTRConfig( TIM1, &TIM_BDTRInitStructure );
  
	TIM_CtrlPWMOutputs(TIM1, ENABLE );
	TIM_OC1PreloadConfig( TIM1, TIM_OCPreload_Disable );
	TIM_ARRPreloadConfig( TIM1, ENABLE );

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
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
    OPA_Unlock();
    OPA1_Init();
    ADC_Function_Init();
    TIM3_PWM_In(16000-1,1000,8000);
    #if(OPA_Break == Break_ENABLE)
    TIM1_Dead_Time_Init( 16000, 2000-1, 8000 );
    #endif

    while(1);

}

void ADC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      ADC_IRQHandler
 *
 * @brief   ADC Interrupt Service Function.
 *
 * @return  none
 */
void ADC_IRQHandler(void)
{
    u16 ADC_val1,ADC_val2,ADC_val3;
    if(ADC_GetITStatus(ADC1, ADC_IT_JEOC))
    {
        ADC_val1=ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
        ADC_val2=ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2);
        ADC_val3=ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_3);
        printf("JADC- %04d   - %04d  - %04d \r\n", ADC_val1,ADC_val2,ADC_val3);
    }

    ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
}

void OPA_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      OPA_IRQHandler
 *
 * @brief   OPA Interrupt Service Function.
 *
 * @return  none
 */
void OPA_IRQHandler(void)
{
  
   if(OPA1_GetFlagStatus( OPA_FLAG_OUT_POLLIntervalEnd))
    {
         printf("END\r\n");
    }

    OPA1_ClearFlag( OPA_FLAG_OUT_POLLIntervalEnd);
}


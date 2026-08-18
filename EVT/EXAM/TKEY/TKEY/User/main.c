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
 *Touchkey detection routine:
 *This example demonstrates channel 3 (PA3), which is a Touchkey application.
 *
 */

#include "debug.h"

/*********************************************************************
 * @fn      Touch_Key_Init
 *
 * @brief   Initializes Touch Key collection.
 *
 * @return  none
 */
void Touch_Key_Init(void)
{
    ADC_InitTypeDef  ADC_InitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TKEY_ChargeTransferModeInitTypeDef   TKEY_InitStructure = {0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TKEY_OPA;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC1->CTLR2|=(1<<25)|(1<<20); //´¥·¢TKY×ª»»

    ADC_Cmd(ADC1, ENABLE);
    
    TKEY_InitStructure.TKEY_SwitchTime = TKEY_SwitchTime_4HCLK;
    TKEY_InitStructure.TKEY_ChargeTransferCycles = 0x3ff;
    TKEY_InitStructure.TKEY_T2_T4 = TKEY_T2_32HCLK_T4_192HCLK;
    TKEY_ChargeTransferMode_Init(TKEY1,&TKEY_InitStructure);  

    TKEY_ChargeTransferChannelConfig(TKEY1, TKEY_Channel_5,TKEY_Channel_3);    

    TKey_MulShieldCmd(TKEY1, ENABLE);
    TKEY1->DRV_CFG|=(0xFFD7<<16);
    TKEY_WorkModeConfig(TKEY1,TKEY_WorkMode_ChargeTransfer);
    TKEY_CtrlChannelModeConfig(TKEY1,TKEY_CtrlChannelMode_TKEY);

    TKEY_Cmd(TKEY1, ENABLE);
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

    Touch_Key_Init();
    while(1)
    {
        TKEY1->CTLR|=(1<<8); 
        while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        printf("%d\r\n",ADC_GetConversionValue(ADC1));
        Delay_Ms(200);
    }

}



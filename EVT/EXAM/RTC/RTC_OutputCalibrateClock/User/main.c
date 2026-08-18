/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2025/08/12
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/**
 * @note
 * RTC select LSE as clock, PC13 Output LSE/64 Frequency.
 * The routine enables the second interrupt and the alarm interrupt.
 *
 */
#include "debug.h"

#define RTC_Event   1
// #define MODE        RTC_Event  //RTC_Alarm Event wakeup standby Mode

/*********************************************************************
 * @fn      RTC_OutputInit
 *
 * @brief   Initializes RTC OutputInit.
 *
 * @return  none
 */
void RTC_OutputInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOC,ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_Init(GPIOC,&GPIO_InitStructure);

    BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);

}

void RTC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      RTC_IRQHandler
 *
 * @brief   This function handles RTC Handler.
 *
 * @return  none
 */
void RTC_IRQHandler(void)
{
    USART_Printf_Init(115200);    
    if (RTC_GetFlagStatus(RTC_FLAG_SEC)==SET)
    {
       printf("RTC SEC\r\n");
    } 
    if (RTC_GetFlagStatus(RTC_FLAG_ALR)==SET)
    {
       printf("RTC ALR\r\n"); 
       RTC_SetCounter(200);
       RTC_WaitForLastTask();
    } 
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_ALR);
    RTC_WaitForLastTask();
}

/*********************************************************************
 * @fn      RTC_EXTI_Config
 *
 * @brief   Initializes RTC EXTI Init.
 *
 * @return  none
 */
static void RTC_EXTI_Config(void)
{
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO , ENABLE);  
    EXTI_InitTypeDef EXTI_InitStructure = {0};

    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
 
    NVIC_DisableIRQ(RTC_IRQn);   
    PWR_EnterSTANDBYMode();         //Enter standby 

}

/*********************************************************************
 * @fn      RTC_NVIC_Config
 *
 * @brief   Initializes RTC Int.
 *
 * @return  none
 */
static void RTC_NVIC_Config(void)
{
    NVIC_SetPriority(RTC_IRQn, 0);
    NVIC_EnableIRQ(RTC_IRQn);
}

/*********************************************************************
 * @fn      RTC_Init
 *
 * @brief   Initializes RTC Init.
 *
 * @return  1 - Init Fail
 *          0 - Init Success
 */
u8 RTC_Init(void)
{
    u8 temp = 0;
    RCC_PB1PeriphClockCmd(RCC_PB1Periph_PWR | RCC_PB1Periph_BKP , ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    /* Is it the first configuration */
    BKP_DeInit();
    RCC_LSEConfig(RCC_LSE_ON);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp < 250)
    {
        temp++;
        Delay_Ms(20);
    }
    if (temp >= 250)
        return 1;   
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForLastTask();
    RTC_WaitForSynchro(); 
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    RTC_WaitForLastTask(); 

    RTC_EnterConfigMode();
    RTC_SetPrescaler(32766);
    RTC_WaitForLastTask();
    RTC_SetAlarm(205);
    RTC_WaitForLastTask();
    RTC_SetCounter(200);
    RTC_WaitForLastTask();  
    RTC_ExitConfigMode();
    RTC_NVIC_Config();
    return 0;
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
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("RTC Output Calibrate Clock\r\n");

    RTC_Init();
    RTC_OutputInit();

#if(MODE == RTC_Event)
      RTC_EXTI_Config(); //RTC_Alarm Event wakeup standby Mode
#endif
 
    while (1)
    {      
    }
}
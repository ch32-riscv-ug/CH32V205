/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch32v205_it.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2025/05/20
 * Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v205_it.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
    while(1)
    {
    }
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
    // printf("RCC->PB1PRSTR=%x\r\n",RCC->PB1PRSTR);
    // printf("RCC->PB2PRSTR=%x\r\n",RCC->PB2PRSTR);
    // printf("RCC->HBRSTR=%x\r\n",RCC->HBRSTR);
    // printf("RCC->RSTSCKR=%x\r\n",RCC->RSTSCKR);

    printf("__get_MCAUSE=%x\r\n",__get_MCAUSE());
     printf("__get_MEPC=%x\r\n",__get_MEPC());
    // NVIC_SystemReset();
    // while (1)
    // {
    // }
}



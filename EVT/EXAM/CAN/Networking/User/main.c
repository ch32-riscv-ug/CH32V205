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
 *CAN normal mode, standard frame and expanded frame data transceiver:
 *This routine requires an external CAN transceiver to demonstrate Standard_Frame and Extended_Frame.
 *Standard_Frame: including 1 32bit filter mask bit pattern, 2 16bit filter mask bit patterns,
 *1 32bit identifier list mode, 2 16bit identifier list modes;
 *Extended_Frame: 1 32bit filter mask bit pattern.
 *
 *Current bitrate is 833333.33Hz. If you want to change the bit rate to config the bitrate to 250k,
 *change the PB1 clock to 48M.If you want to configure it yourself, please refer to our manual.
 */

#include "debug.h"

/* CAN Mode Definition */
#define TX_MODE 0
#define RX_MODE 1

/* Frame Format Definition */
#define Standard_Frame 0
#define Extended_Frame 1

/* CAN Communication Mode Selection */
// #define CAN_MODE TX_MODE
#define CAN_MODE RX_MODE

/* Frame Formate Selection */
#define Frame_Format Standard_Frame
// #define Frame_Format   Extended_Frame

/* Receive can data in interrupt */
#define USE_INTERRUPT_TO_REC

// #define USE_SOFT_FILTER

#define CANSOFTFILTER_MAX_GROUP_NUM 14 // The maximum recommended configuration is 14.
// Configure only what you need to prevent excessive RAM usage or an increase in the software's filtering time.

#define CANSOFTFILER_PREDEF_CTRLBYTE_MASK32 ((CAN_FilterScale_32bit << 5) | (CAN_FilterMode_IdMask << 1))
#define CANSOFTFILER_PREDEF_CTRLBYTE_ID32 ((CAN_FilterScale_32bit << 5) | (CAN_FilterMode_IdList << 1))

#define GET_PIN_BIT(x) ((uint16_t)0x1 << x)

/*
This is the structure of the software filtering table. It can be configured through the CAN_SoftFilterInit function,
or you can directly set the configuration values. The configured values can be modified directly during runtime.
However, when using the interrupt mode for reception, you need to be aware that if the modification is interrupted,
it may affect the filtering results during this period.
*/
struct CANFilterStruct_t
{
    union {
        union {
            struct
            {
                uint32_t : 1;
                uint32_t RTR : 1;
                uint32_t IDE : 1;
                uint32_t ExID : 29;
            } Access_Ex;
            struct
            {
                uint32_t : 1;
                uint32_t RTR : 1;
                uint32_t IDE : 1;
                uint32_t : 18;
                uint32_t StID : 11;
            } Access_St;
        };
        union {
            struct
            {
                uint16_t FR_16_L;
                uint16_t FR_16_H;
            };
            uint32_t FR_32;
        };
    } FR[2];
    union {
        struct
        {
            uint16_t en : 1;
            uint16_t mode : 4;
            uint16_t scale : 3;
        };
        uint16_t ctrl_byte;
    };
} CANFilterStruct[CANSOFTFILTER_MAX_GROUP_NUM];

/**
 * @struct CANPinTable_t
 * @note   CAN pin table structure
 */
typedef struct
{
    CAN_TypeDef *CANx;
    GPIO_TypeDef *PORT;
    uint8_t Pin;
    uint8_t AF;
} CANPinTable_t;

/**
 * @brief  CAN Tx pin table
 * @note   CAN1: PA12, PB9, PD1
 */
const CANPinTable_t CANTXPinTable[] = {
    {CAN1, GPIOA, 12, GPIO_AF6},{CAN1, GPIOB, 9, GPIO_AF6},
    {CAN1, GPIOD, 1, GPIO_AF6}
};
/**
 * @brief  CAN Rx pin table
 * @note   CAN1: PA11, PB8, PD0
 */
const CANPinTable_t CANRXPinTable[] = {
    {CAN1, GPIOA, 11, GPIO_AF6},{CAN1, GPIOB, 8, GPIO_AF6},
    {CAN1, GPIOD, 0, GPIO_AF6}
};

volatile uint8_t TxBuff[8], RxBuff[8];

volatile uint8_t RxInterruptFlag = 0;

/*********************************************************************
 * @fn      CAN_Mode_Init
 *
 * @brief   CAN mode initialization
 *
 * @param   CANx: CAN1 or CAN2
 *          RxTab: CAN Rx pin table
 *          TxTab: CAN Tx pin table
 *          CAN_InitSturcture: CAN initialization structure
 *
 * @return  ErrorStatus: READY or NoREADY
 */
ErrorStatus CAN_Mode_Init(CAN_TypeDef *CANx, const CANPinTable_t *RxTab, const CANPinTable_t *TxTab,
                          CAN_InitTypeDef *CAN_InitSturcture)
{
    GPIO_InitTypeDef GPIO_InitSturcture = {0};

    if (CANx != RxTab->CANx || CANx != TxTab->CANx)
    {
        return NoREADY;
    }

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO | (RCC_PB2Periph_GPIOA << (((void *)(RxTab->PORT) - (void *)GPIOA) / sizeof(GPIO_TypeDef))) |
                              (RCC_PB2Periph_GPIOA << ((TxTab->PORT - GPIOA) / sizeof(GPIO_TypeDef))),
                          ENABLE);

    RCC_PB1PeriphClockCmd(RCC_PB1Periph_CAN1, ENABLE);

    GPIO_PinAFConfig(RxTab->PORT, RxTab->Pin, RxTab->AF);
    GPIO_PinAFConfig(TxTab->PORT, TxTab->Pin, TxTab->AF);

    GPIO_InitSturcture.GPIO_Pin = GET_PIN_BIT(TxTab->Pin);
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_High;
    GPIO_Init(TxTab->PORT, &GPIO_InitSturcture);

    GPIO_InitSturcture.GPIO_Pin = GET_PIN_BIT(RxTab->Pin);
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(RxTab->PORT, &GPIO_InitSturcture);

    CAN_Init(CANx, CAN_InitSturcture);

    return READY;
}

/*********************************************************************
 * @fn      CAN_SoftFilterInit
 *
 * @brief   CAN soft filter initialization
 *
 * @param   CAN_FilterInitStruct: CAN filter initialization structure
 *
 * @return  None
 *
 */
void CAN_SoftFilterInit(CAN_FilterInitTypeDef *CAN_FilterInitStruct)
{
    if (CAN_FilterInitStruct->CAN_FilterNumber > CANSOFTFILTER_MAX_GROUP_NUM)
    {
        return;
    }
    if (CAN_FilterInitStruct->CAN_FilterActivation)
    {
        CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].en = 1;
    }
    else
    {
        CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].en = 0;
    }

    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[0].FR_16_H = CAN_FilterInitStruct->CAN_FilterIdHigh;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[0].FR_16_L = CAN_FilterInitStruct->CAN_FilterIdLow;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[1].FR_16_H = CAN_FilterInitStruct->CAN_FilterMaskIdHigh;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].FR[1].FR_16_L = CAN_FilterInitStruct->CAN_FilterMaskIdLow;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].mode = CAN_FilterInitStruct->CAN_FilterMode;
    CANFilterStruct[CAN_FilterInitStruct->CAN_FilterNumber].scale = CAN_FilterInitStruct->CAN_FilterScale;
}
/*********************************************************************
 * @fn     	CAN_Send_Msg
 *
 * @brief   	CAN Transmit function.
 *
 * @param   	CANx - where x can be 1...3 to select the CAN peripheral.
 *		   	IdType - The target ID type for sending the message.
 *				- CAN_Id_Standard
 *				- CAN_Id_Extended
 *		   	Identifier - The target ID for sending the message.
 *		   	msg - Transmit data buffer.
 *          	len - Data length.
 *
 * @return  	0 - Send successful.
 *          	1 - Send failed.
 */
uint8_t CAN_Send_Msg(CAN_TypeDef *CANx, uint32_t IdType, uint32_t Identifier, uint8_t *msg, uint8_t len)
{
    u8 mbox;
    u16 i = 0;

    CanTxMsg CanTxStructure = {0};

    IdType == CAN_Id_Extended ? (CanTxStructure.ExtId = Identifier) : (CanTxStructure.StdId = Identifier);
    CanTxStructure.IDE = IdType;
    CanTxStructure.RTR = CAN_RTR_Data;
    CanTxStructure.DLC = len;

    for (i = 0; i < len; i++)
    {
        CanTxStructure.Data[i] = msg[i];
    }

    mbox = CAN_Transmit(CANx, &CanTxStructure);
    i = 0;

    while ((CAN_TransmitStatus(CANx, mbox) != CAN_TxStatus_Ok) && (i < 0xFFF))
    {
        i++;
    }

    if (i == 0xFFF)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*********************************************************************
 * @fn      	CAN_ReceiveViaSoftFilter
 *
 * @brief   	Receives a message via soft filter.
 *
 * @param   	CANx - where x can be 1 to select the CAN peripheral.
 *          	FIFONumber - Receive FIFO number.
 *        			- CAN_FIFO0.
 *          	RxMessage -  pointer to a structure receive message which contains
 *        CAN Id, CAN DLC, CAN datas and FMI number.
 *
 * @return  	none
 */
void CAN_ReceiveViaSoftFilter(CAN_TypeDef* CANx, uint8_t FIFONumber, CanRxMsg* RxMessage)
{
    for (int group = 0; group < sizeof(CANFilterStruct)/sizeof(*CANFilterStruct); group++) 
    {
        if (CANFilterStruct[group].en) 
        {
            uint32_t temp = CANx->sFIFOMailBox[0].RXMIR & (~0x1);
            switch ((uint8_t)CANFilterStruct[group].ctrl_byte & ~0x1) 
            {

                case CANSOFTFILER_PREDEF_CTRLBYTE_ID32:
                    if((CANFilterStruct[group].FR[0].FR_32 != temp) && (CANFilterStruct[group].FR[1].FR_32 != temp))
                    {
                        continue;
                    }
                    else
                    {
                        CAN_Receive(CANx, CAN_FIFO0, RxMessage);
                        return;
                    }
                    break;

                case CANSOFTFILER_PREDEF_CTRLBYTE_MASK32:
                    if((CANFilterStruct[group].FR[0].FR_32 & CANFilterStruct[group].FR[1].FR_32) ^ (temp & CANFilterStruct[group].FR[1].FR_32))
                    {
                        continue;
                    }
                    else 
                    {
                        CAN_Receive(CANx, CAN_FIFO0, RxMessage);
                        return;
                    }
                    break;

                default:
                    return;
                    break;
            }
        }
    }
    CAN_FIFORelease(CANx,CAN_FIFO0);
}

/*********************************************************************
 * @fn      CAN_Receive_Msg
 *
 * @brief   CAN Receive function.
 *
 * @param   CANx - where x can be 1...3 to select the CAN peripheral.
 *			RxBuff - Pointer to the memory used to store received data.
 *
 * @return  CanRxStructure.DLC - Receive data length.
 */
uint8_t CAN_Receive_Msg(CAN_TypeDef *CANx, uint8_t *RxBuff)
{
    u8 i;

    CanRxMsg CanRxStructure = {0};

    if (CAN_MessagePending(CANx, CAN_FIFO0) == 0)
    {
        return 0;
    }
#ifdef USE_SOFT_FILTER
    CAN_ReceiveViaSoftFilter(CANx, CAN_FIFO0, &CanRxStructure);
#else
    CAN_Receive(CANx, CAN_FIFO0, &CanRxStructure);
#endif
    for (i = 0; i < 8; i++)
    {
        RxBuff[i] = CanRxStructure.Data[i];
    }

    return CanRxStructure.DLC;
}

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      main
 *
 * @brief   program to test can in loopback mode
 *
 * @return  none
 */
int main(void)
{
    uint32_t CAN1_TarID = 0x317;
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    printf("CAN test in loopback mode\n");
    printf("Chip_id: %#010x\n",CHIPID);
    printf("Current SystemCoreClock: %d\n",SystemCoreClock);

    uint32_t Circle_CNT = 0;
    CAN_FilterInitTypeDef CAN_FilterInitSturcture = {0};
    CAN_FilterInitSturcture.CAN_FilterNumber = 0;
#if (Frame_Format == Standard_Frame)
    /* identifier mask mode, One 32-bit filter, StdId: 0x317 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x62E0;
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFFE0;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0x0006;
    /* identifier list mode, One 32-bit filter, StdId: 0x317,0x316 */
	// CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdList;
	// CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
	// CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x62E0;
	// CAN_FilterInitSturcture.CAN_FilterIdLow = 0;
	// CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0x62C0;
	// CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0;
#elif (Frame_Format == Extended_Frame)
    /* identifier mask mode, One 32-bit filter, ExtId: 0x12124567 */
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x9092;
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0x2B3C;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0xFFFF;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0xFFFE;
    // /* identifier list mode, One 32-bit filter, ExtId: 0x12124567,0x12124566 */
    // CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdList;
    // CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    // CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x9092;
    // CAN_FilterInitSturcture.CAN_FilterIdLow = 0x2B3C;
    // CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0x9092;
    // CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0x2B34;
#endif

    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;

    CAN_InitTypeDef CAN_InitSturcture = {};
    CAN_InitSturcture.CAN_TTCM = DISABLE;
    CAN_InitSturcture.CAN_ABOM = DISABLE;
    CAN_InitSturcture.CAN_AWUM = DISABLE;
    CAN_InitSturcture.CAN_NART = ENABLE;
    CAN_InitSturcture.CAN_RFLM = DISABLE;
    CAN_InitSturcture.CAN_TXFP = DISABLE;
    
    CAN_InitSturcture.CAN_Mode = CAN_Mode_LoopBack;

    CAN_InitSturcture.CAN_SJW = CAN_SJW_1tq;
    CAN_InitSturcture.CAN_BS1 = CAN_BS1_6tq;
    CAN_InitSturcture.CAN_BS2 = CAN_BS2_5tq;
    CAN_InitSturcture.CAN_Prescaler = 16;
    CAN_Mode_Init(CAN1, CANRXPinTable, CANTXPinTable, &CAN_InitSturcture);
#ifdef USE_INTERRUPT_TO_REC
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
#endif

#ifdef USE_SOFT_FILTER
    CAN_FilterInitTypeDef releaseallfilter = {0};
    releaseallfilter.CAN_FilterMode = CAN_FilterMode_IdMask;
    releaseallfilter.CAN_FilterScale = CAN_FilterScale_32bit;

    CAN_FilterInit(&releaseallfilter);
    CAN_SoftFilterInit(&CAN_FilterInitSturcture);
#else
    CAN_FilterInit(&CAN_FilterInitSturcture);
#endif
printf("%#x\n",CAN1->FWR);
    while (1)
    {
#if (CAN_MODE == TX_MODE)
        printf("------------Sending data-------------\n");

        printf("CAN1: ");
        for (uint8_t i = 0; i < sizeof(TxBuff) / sizeof(*TxBuff); i++)
        {
            TxBuff[i] = (i + Circle_CNT) % 256;
            printf("%#04x ", TxBuff[i]);
        }
        if (!CAN_Send_Msg(CAN1, CAN_Id_Standard, CAN1_TarID, (uint8_t *)TxBuff, 8))
        {
            printf("|\t-->\t%#x\033[32m\nSuccessful\033[0m\n", CAN1_TarID);
        }
        else
        {
            printf("|\t-->\t%#x\033[31m\nFail\033[0m\n", CAN1_TarID);
        }

        printf("-------------------------------------\n");
#else
#ifdef USE_INTERRUPT_TO_REC
        printf("************Receive data*************\n");
        printf("Received data: ");
        if (RxInterruptFlag)
        {

            for (uint8_t i = 0; i < RxInterruptFlag; i++)
            {
                printf("%#04x ", RxBuff[i]);
            }
            RxInterruptFlag = 0;
        }
        else
        {
            printf("\033[34mNone");
        }
        printf("\033[0m\n");
        printf("*************************************\n");

#else
        printf("************Receive data*************\n");
        uint8_t rxlen;

        printf("CAN1: ");
        if ((rxlen = CAN_Receive_Msg(CAN1, (uint8_t *)RxBuff)))
        {
            for (uint8_t i = 0; i < rxlen; i++)
            {
                printf("%#04x ", RxBuff[i]);
            }
            rxlen = 0;
        }
        else
        {
            printf("\033[34mNone");
        }
        printf("\033[0m\n");
        
        printf("*************************************\n");
#endif
#endif
        Circle_CNT++;
        Delay_Ms(500);
    }
}

#ifdef USE_INTERRUPT_TO_REC
__attribute__((interrupt("WCH-Interrupt-fast"))) void USB_LP_CAN1_RX0_IRQHandler()
{
    int i;
    uint8_t px, pbuf[8];
    if (CAN_GetITStatus(CAN1, CAN_IT_FMP0))
    {
        px = CAN_Receive_Msg(CAN1, pbuf);
        for (i = 0; i < px; i++)
        {
            RxBuff[i] = pbuf[i];
        }
        if (px)
        {
            RxInterruptFlag = px;
        }
        CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
    }
}
#endif
/*********************************************************************
 * File Name          : lcd.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/8/15
 * Description        : file for lcd screen.
 *********************************************************************************
* Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#include "lcd.h"
#include "debug.h"
#include "LCD_Font.h"
#include "debug.h"
#include "ch32v205_qspi.h"

#define ST77916_NOP        0x00
#define ST77916_SWRESET    0x01
#define ST77916_RDDID      0x04
#define ST77916_RDDST      0x09

#define ST77916_SLPIN      0x10
#define ST77916_SLPOUT     0x11
#define ST77916_PTLON      0x12
#define ST77916_NORON      0x13

#define ST77916_INVOFF     0x20
#define ST77916_INVON      0x21
#define ST77916_DISPOFF    0x28
#define ST77916_DISPON     0x29

#define ST77916_CASET      0x2A
#define ST77916_RASET      0x2B
#define ST77916_RAMWR      0x2C
#define ST77916_RAMRD      0x2E

#define ST77916_PTLAR      0x30
#define ST77916_COLMOD     0x3A
#define ST77916_MADCTL     0x36

#define ST77916_MADCTL_MY  0x80
#define ST77916_MADCTL_MX  0x40
#define ST77916_MADCTL_MV  0x20
#define ST77916_MADCTL_ML  0x10
#define ST77916_MADCTL_RGB 0x00
#define ST77916_MADCTL_BGR 0x08
#define ST77916_MADCTL_MH  0x04

#define WRITE_C8_D8        (1)
#define WRITE_C8_D16       (2)
#define WRITE_C8_Dn        (3)
#define WRITE_End          (0xff)

const uint8_t st77916_init_code[] = {
    WRITE_C8_D8, 0xF0, 0x28,         //
    WRITE_C8_D8, 0xF2, 0x28,         //
    WRITE_C8_D8, 0x73, 0xF0,         //
    WRITE_C8_D8, 0x7C, 0xD1,         //
    WRITE_C8_D8, 0x83, 0xE0,         //
    WRITE_C8_D8, 0x84, 0x61,         //
    WRITE_C8_D8, 0xF2, 0x82,         //
    WRITE_C8_D8, 0xF0, 0x00,         //
    WRITE_C8_D8, 0xF0, 0x01,         //
    WRITE_C8_D8, 0xF1, 0x01,         //
    WRITE_C8_D8, 0xB0, 0x5E,         //
    WRITE_C8_D8, 0xB1, 0x55,         //
    WRITE_C8_D8, 0xB2, 0x24,         //
    WRITE_C8_D8, 0xB3, 0x01,         //
    WRITE_C8_D8, 0xB4, 0x87,         //
    WRITE_C8_D8, 0xB5, 0x44,         //
    WRITE_C8_D8, 0xB6, 0x8B,         //
    WRITE_C8_D8, 0xB7, 0x40,         //
    WRITE_C8_D8, 0xB8, 0x86,         //
    WRITE_C8_D8, 0xB9, 0x15,         //
    WRITE_C8_D8, 0xBA, 0x00,         //
    WRITE_C8_D8, 0xBB, 0x08,         //
    WRITE_C8_D8, 0xBC, 0x08,         //
    WRITE_C8_D8, 0xBD, 0x00,         //
    WRITE_C8_D8, 0xBE, 0x00,         //
    WRITE_C8_D8, 0xBF, 0x07,         //
    WRITE_C8_D8, 0xC0, 0x80,         //
    WRITE_C8_D8, 0xC1, 0x10,         //
    WRITE_C8_D8, 0xC2, 0x37,         //
    WRITE_C8_D8, 0xC3, 0x80,         //
    WRITE_C8_D8, 0xC4, 0x10,         //
    WRITE_C8_D8, 0xC5, 0x37,         //
    WRITE_C8_D8, 0xC6, 0xA9,         //
    WRITE_C8_D8, 0xC7, 0x41,         //
    WRITE_C8_D8, 0xC8, 0x01,         //
    WRITE_C8_D8, 0xC9, 0xA9,         //
    WRITE_C8_D8, 0xCA, 0x41,         //
    WRITE_C8_D8, 0xCB, 0x01,         //
    WRITE_C8_D8, 0xCC, 0x7F,         //
    WRITE_C8_D8, 0xCD, 0x7F,         //
    WRITE_C8_D8, 0xCE, 0xFF,         //
    WRITE_C8_D8, 0xD0, 0x91,         //
    WRITE_C8_D8, 0xD1, 0x68,         //
    WRITE_C8_D8, 0xD2, 0x68,         //
    WRITE_C8_D16, 0xF5, 0x00, 0xA5,  //
    WRITE_C8_D8, 0xDD, 0x40,         //
    WRITE_C8_D8, 0xDE, 0x40,         //
    WRITE_C8_D8, 0xF1, 0x10,         //
    WRITE_C8_D8, 0xF0, 0x00,         //
    WRITE_C8_D8, 0xF0, 0x02,         //
    WRITE_C8_Dn, 0xE0,               //
    14,                              //
    0xF0, 0x10, 0x18, 0x0D,          //
    0x0C, 0x38, 0x3E, 0x44,          //
    0x51, 0x39, 0x15, 0x15,          //
    0x30, 0x34,                      //
    WRITE_C8_Dn, 0xE1,               //
    14,                              //
    0xF0, 0x0F, 0x17, 0x0D,          //
    0x0B, 0x07, 0x3E, 0x33,          //
    0x51, 0x39, 0x15, 0x15,          //
    0x30, 0x34,                      //
    WRITE_C8_D8, 0xF0, 0x10,         //
    WRITE_C8_D8, 0xF3, 0x10,         //
    WRITE_C8_D8, 0xE0, 0x08,         //
    WRITE_C8_D8, 0xE1, 0x00,         //
    WRITE_C8_D8, 0xE2, 0x00,         //
    WRITE_C8_D8, 0xE3, 0x00,         //
    WRITE_C8_D8, 0xE4, 0xE0,         //
    WRITE_C8_D8, 0xE5, 0x06,         //
    WRITE_C8_D8, 0xE6, 0x21,         //
    WRITE_C8_D8, 0xE7, 0x03,         //
    WRITE_C8_D8, 0xE8, 0x05,         //
    WRITE_C8_D8, 0xE9, 0x02,         //
    WRITE_C8_D8, 0xEA, 0xE9,         //
    WRITE_C8_D8, 0xEB, 0x00,         //
    WRITE_C8_D8, 0xEC, 0x00,         //
    WRITE_C8_D8, 0xED, 0x14,         //
    WRITE_C8_D8, 0xEE, 0xFF,         //
    WRITE_C8_D8, 0xEF, 0x00,         //
    WRITE_C8_D8, 0xF8, 0xFF,         //
    WRITE_C8_D8, 0xF9, 0x00,         //
    WRITE_C8_D8, 0xFA, 0x00,         //
    WRITE_C8_D8, 0xFB, 0x30,         //
    WRITE_C8_D8, 0xFC, 0x00,         //
    WRITE_C8_D8, 0xFD, 0x00,         //
    WRITE_C8_D8, 0xFE, 0x00,         //
    WRITE_C8_D8, 0xFF, 0x00,         //
    WRITE_C8_D8, 0x60, 0x40,         //
    WRITE_C8_D8, 0x61, 0x05,         //
    WRITE_C8_D8, 0x62, 0x00,         //
    WRITE_C8_D8, 0x63, 0x42,         //
    WRITE_C8_D8, 0x64, 0xDA,         //
    WRITE_C8_D8, 0x65, 0x00,         //
    WRITE_C8_D8, 0x66, 0x00,         //
    WRITE_C8_D8, 0x67, 0x00,         //
    WRITE_C8_D8, 0x68, 0x00,         //
    WRITE_C8_D8, 0x69, 0x00,         //
    WRITE_C8_D8, 0x6A, 0x00,         //
    WRITE_C8_D8, 0x6B, 0x00,         //
    WRITE_C8_D8, 0x70, 0x40,         //
    WRITE_C8_D8, 0x71, 0x04,         //
    WRITE_C8_D8, 0x72, 0x00,         //
    WRITE_C8_D8, 0x73, 0x42,         //
    WRITE_C8_D8, 0x74, 0xD9,         //
    WRITE_C8_D8, 0x75, 0x00,         //
    WRITE_C8_D8, 0x76, 0x00,         //
    WRITE_C8_D8, 0x77, 0x00,         //
    WRITE_C8_D8, 0x78, 0x00,         //
    WRITE_C8_D8, 0x79, 0x00,         //
    WRITE_C8_D8, 0x7A, 0x00,         //
    WRITE_C8_D8, 0x7B, 0x00,         //
    WRITE_C8_D8, 0x80, 0x48,         //
    WRITE_C8_D8, 0x81, 0x00,         //
    WRITE_C8_D8, 0x82, 0x07,         //
    WRITE_C8_D8, 0x83, 0x02,         //
    WRITE_C8_D8, 0x84, 0xD7,         //
    WRITE_C8_D8, 0x85, 0x04,         //
    WRITE_C8_D8, 0x86, 0x00,         //
    WRITE_C8_D8, 0x87, 0x00,         //
    WRITE_C8_D8, 0x88, 0x48,         //
    WRITE_C8_D8, 0x89, 0x00,         //
    WRITE_C8_D8, 0x8A, 0x09,         //
    WRITE_C8_D8, 0x8B, 0x02,         //
    WRITE_C8_D8, 0x8C, 0xD9,         //
    WRITE_C8_D8, 0x8D, 0x04,         //
    WRITE_C8_D8, 0x8E, 0x00,         //
    WRITE_C8_D8, 0x8F, 0x00,         //
    WRITE_C8_D8, 0x90, 0x48,         //
    WRITE_C8_D8, 0x91, 0x00,         //
    WRITE_C8_D8, 0x92, 0x0B,         //
    WRITE_C8_D8, 0x93, 0x02,         //
    WRITE_C8_D8, 0x94, 0xDB,         //
    WRITE_C8_D8, 0x95, 0x04,         //
    WRITE_C8_D8, 0x96, 0x00,         //
    WRITE_C8_D8, 0x97, 0x00,         //
    WRITE_C8_D8, 0x98, 0x48,         //
    WRITE_C8_D8, 0x99, 0x00,         //
    WRITE_C8_D8, 0x9A, 0x0D,         //
    WRITE_C8_D8, 0x9B, 0x02,         //
    WRITE_C8_D8, 0x9C, 0xDD,         //
    WRITE_C8_D8, 0x9D, 0x04,         //
    WRITE_C8_D8, 0x9E, 0x00,         //
    WRITE_C8_D8, 0x9F, 0x00,         //
    WRITE_C8_D8, 0xA0, 0x48,         //
    WRITE_C8_D8, 0xA1, 0x00,         //
    WRITE_C8_D8, 0xA2, 0x06,         //
    WRITE_C8_D8, 0xA3, 0x02,         //
    WRITE_C8_D8, 0xA4, 0xD6,         //
    WRITE_C8_D8, 0xA5, 0x04,         //
    WRITE_C8_D8, 0xA6, 0x00,         //
    WRITE_C8_D8, 0xA7, 0x00,         //
    WRITE_C8_D8, 0xA8, 0x48,         //
    WRITE_C8_D8, 0xA9, 0x00,         //
    WRITE_C8_D8, 0xAA, 0x08,         //
    WRITE_C8_D8, 0xAB, 0x02,         //
    WRITE_C8_D8, 0xAC, 0xD8,         //
    WRITE_C8_D8, 0xAD, 0x04,         //
    WRITE_C8_D8, 0xAE, 0x00,         //
    WRITE_C8_D8, 0xAF, 0x00,         //
    WRITE_C8_D8, 0xB0, 0x48,         //
    WRITE_C8_D8, 0xB1, 0x00,         //
    WRITE_C8_D8, 0xB2, 0x0A,         //
    WRITE_C8_D8, 0xB3, 0x02,         //
    WRITE_C8_D8, 0xB4, 0xDA,         //
    WRITE_C8_D8, 0xB5, 0x04,         //
    WRITE_C8_D8, 0xB6, 0x00,         //
    WRITE_C8_D8, 0xB7, 0x00,         //
    WRITE_C8_D8, 0xB8, 0x48,         //
    WRITE_C8_D8, 0xB9, 0x00,         //
    WRITE_C8_D8, 0xBA, 0x0C,         //
    WRITE_C8_D8, 0xBB, 0x02,         //
    WRITE_C8_D8, 0xBC, 0xDC,         //
    WRITE_C8_D8, 0xBD, 0x04,         //
    WRITE_C8_D8, 0xBE, 0x00,         //
    WRITE_C8_D8, 0xBF, 0x00,         //
    WRITE_C8_D8, 0xC0, 0x10,         //
    WRITE_C8_D8, 0xC1, 0x47,         //
    WRITE_C8_D8, 0xC2, 0x56,         //
    WRITE_C8_D8, 0xC3, 0x65,         //
    WRITE_C8_D8, 0xC4, 0x74,         //
    WRITE_C8_D8, 0xC5, 0x88,         //
    WRITE_C8_D8, 0xC6, 0x99,         //
    WRITE_C8_D8, 0xC7, 0x01,         //
    WRITE_C8_D8, 0xC8, 0xBB,         //
    WRITE_C8_D8, 0xC9, 0xAA,         //
    WRITE_C8_D8, 0xD0, 0x10,         //
    WRITE_C8_D8, 0xD1, 0x47,         //
    WRITE_C8_D8, 0xD2, 0x56,         //
    WRITE_C8_D8, 0xD3, 0x65,         //
    WRITE_C8_D8, 0xD4, 0x74,         //
    WRITE_C8_D8, 0xD5, 0x88,         //
    WRITE_C8_D8, 0xD6, 0x99,         //
    WRITE_C8_D8, 0xD7, 0x01,         //
    WRITE_C8_D8, 0xD8, 0xBB,         //
    WRITE_C8_D8, 0xD9, 0xAA,         //
    WRITE_C8_D8, 0xF3, 0x01,         //
    WRITE_C8_D8, 0xF0, 0x00,         //
    WRITE_C8_D8, 0x3A, 0x55,         //

    WRITE_End, 0xff

};

/*********************************************************************
 * @fn      QSPI_Send
 * 
 * @brief   Send data to QSPI
 * 
 * @param   pBuffer - pointer to the data buffer
 * @param   len - length of the data to be send
 * 
 * @return  none
 */
void QSPI_Send(uint8_t* pBuffer, uint32_t len)
{
    uint32_t i = 0;
    while (i < len)
    {
        if (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_FT))
        {
            QSPI_SendData8(QSPI1, pBuffer[i]);
            i++;
        }
    }
}

/*********************************************************************
 * @fn      QSPI_Receive
 * 
 * @brief   Receive data from QSPI
 * 
 * @param   pBuffer - pointer to the data buffer
 * @param   len - length of the data to be received
 * 
 * @return  none
 */
void QSPI_Receive(uint8_t* pBuffer, uint32_t len)
{
    uint32_t i = 0;
    while (i < len)
    {
        if (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_FT))
        {
            pBuffer[i] = QSPI_ReceiveData8(QSPI1);
            i++;
        }
    }
}

void qspi_cs_set()
{
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
}

void qspi_cs_clr()
{
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
}

/*********************************************************************
 * @fn      GPIO_Config
 * 
 * @brief   Configure the GPIO pins
 * 
 * @param   none
 */
static void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_AFIO, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB, ENABLE);

    // QSPI_SCK PB2(AF5)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource2, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // QSPI_SCSXN PB1(AF5)
    // GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF5);
    // GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    // GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    // GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // QSPI_SIOX0 PB8(AF5)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // QSPI_SIOX1 PB9(AF5)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // QSPI_SIOX2 PB10(AF5)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // QSPI_SIOX3 PB11(AF5)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_SetBits(GPIOC, GPIO_Pin_13);

    qspi_cs_set();
}

/*********************************************************************
 * @fn      QSPI1_Config
 * 
 * @brief   Configure the QSPI1
 * 
 * @param   none
 */
static void QSPI1_Config(void)
{
    QSPI_InitTypeDef QSPI_InitStructure = {0};

    RCC_HBPeriphClockCmd(RCC_HBPeriph_QSPI1, ENABLE);
    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA2, ENABLE);

    QSPI_InitStructure.QSPI_Prescaler = 10;
    QSPI_InitStructure.QSPI_CKMode    = QSPI_CKMode_Mode3;
    QSPI_InitStructure.QSPI_CSHTime   = QSPI_CSHTime_7Cycle;

    // size = 2 ** (FSize + 1) = 2 ** 23 = 8MB
    QSPI_InitStructure.QSPI_FSize = 0xff;

    QSPI_InitStructure.QSPI_FSelect = QSPI_FSelect_2;
    QSPI_InitStructure.QSPI_DFlash  = QSPI_DFlash_Disable;

    QSPI_Init(QSPI1, &QSPI_InitStructure);
    QSPI_SetFIFOThreshold(QSPI1, 12);
}

void QSPI_Tx_DMA(uint8_t* buffer, uint32_t len)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    DMA_DeInit(DMA2_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (QSPI1->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr    = (u32)buffer;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize         = len / 2;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel1, &DMA_InitStructure);

    DMA_ClearFlag(DMA2, DMA_FLAG_TC1);
}

void QSPI_ReadData(uint8_t cmd, uint8_t* data, uint32_t len)
{

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_clr();
    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = 0x0b;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 8;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, cmd << 8);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_Start(QSPI1);

    QSPI_Receive(data, len);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_set();
}

void QSPI_sendCmd(uint8_t cmd)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_clr();
    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_NoData;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = 0x02;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, cmd << 8);

    QSPI_Start(QSPI1);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_set();
}

void QSPI_sendCmd_raw(uint8_t cmd)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_NoData;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = 0x32;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, cmd << 8);

    QSPI_Start(QSPI1);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
}




// PP 0x02
void QSPI_WriteBuffer(uint8_t cmd, uint8_t* data, uint32_t len)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_clr();
    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Write;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = 0x02;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, cmd << 8);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_Start(QSPI1);

    QSPI_Send(data, len);

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_set();
}


void LCD_SetWindows(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    uint8_t x_data[] = {
        (x_start >> 8) & 0xFF,
        x_start & 0xFF,
        ((x_end) >> 8) & 0xFF,
        (x_end) & 0xFF,
    };
    uint8_t y_data[] = {
        (y_start >> 8) & 0xFF,
        y_start & 0xFF,
        ((y_end) >> 8) & 0xFF,
        (y_end) & 0xFF,
    };

    QSPI_WriteBuffer(ST77916_CASET, x_data, 4);
    QSPI_WriteBuffer(ST77916_RASET, y_data, 4);
}

void QSPI_WriteBuffer_raw(uint8_t* data, uint32_t len)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_NoInstruction;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_NoAddress;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_4Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Write;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = 0;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetDataLength(QSPI1, len);

    QSPI_DMACmd(QSPI1, ENABLE);
    QSPI_Tx_DMA(data, len);
    DMA_Cmd(DMA2_Channel1, ENABLE);

    QSPI_Start(QSPI1);
}

void QSPI_WriteColor_Quad(uint8_t cmd, uint32_t len, uint16_t color)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_clr();
    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_4Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Write;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = 0x32;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, cmd << 8);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_Start(QSPI1);

    uint32_t i = 0;
    while (i < len / 2)
    {
        if (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_FT))
        {
            QSPI_SendData16(QSPI1, color);
            i++;
        }
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_set();
}

void LCD_Write_C8(uint8_t cmd)
{
    return QSPI_sendCmd(cmd);
}

void LCD_Write_C8D8(uint8_t cmd, uint8_t data)
{
    return QSPI_WriteBuffer(cmd, &data, 1);
}

void LCD_Write_C8D16(uint8_t cmd, uint8_t data1, uint8_t data2)
{
    uint8_t data[] = {data1, data2};
    return QSPI_WriteBuffer(cmd, data, 2);
}

void LCD_Write_Cmd_Seq(uint8_t cmd, uint8_t* data, uint16_t len)
{
    if (len == 0)
    {
        return LCD_Write_C8(cmd);
    }
    else
    {
        return QSPI_WriteBuffer(cmd, data, len);
    }
}

void LCD_do_init(const uint8_t* _init_op, uint32_t int_len)
{
    uint8_t* p  = (uint8_t*)_init_op;
    uint8_t* ep = p + int_len;

    uint8_t cmd_type;

    while (p < ep)
    {
        cmd_type = *p;
        p++;
        switch (cmd_type)
        {
            case WRITE_C8_D8:
            {
                uint8_t cmd = *p;
                p++;
                uint8_t data = *p;
                p++;
                LCD_Write_C8D8(cmd, data);
                break;
            }
            case WRITE_C8_D16:
            {
                uint8_t cmd = *p;
                p++;
                uint8_t data1 = *p;
                p++;
                uint8_t data2 = *p;
                p++;
                LCD_Write_C8D16(cmd, data1, data2);
                break;
            }
            case WRITE_C8_Dn:
            {
                uint8_t cmd = *p;
                p++;
                uint8_t len = *p;
                p++;
                uint8_t* data = p;
                p += len;

                LCD_Write_Cmd_Seq(cmd, data, len);

                break;
            }
            case WRITE_End:
            {
                return;

                break;
            }
            default:
                // printf("err %x\n", cmd_type);
                break;
        }
    }
}

void LCD_DrawColor_Quad(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint16_t color)
{
    uint8_t x_data[] = {
        (x_start >> 8) & 0xFF,
        x_start & 0xFF,
        ((x_end - 1) >> 8) & 0xFF,
        (x_end - 1) & 0xFF,
    };
    uint8_t y_data[] = {
        (y_start >> 8) & 0xFF,
        y_start & 0xFF,
        ((y_end - 1) >> 8) & 0xFF,
        (y_end - 1) & 0xFF,
    };

    QSPI_WriteBuffer(ST77916_CASET, x_data, 4);
    QSPI_WriteBuffer(ST77916_RASET, y_data, 4);

    size_t len = (x_end - x_start) * (y_end - y_start) * 2;
    QSPI_WriteColor_Quad(ST77916_RAMWR, len, color);
}

void LCD_setRotation(uint8_t r)
{
    switch (r)
    {
        case 1:
            r = ST77916_MADCTL_MX | ST77916_MADCTL_MV | ST77916_MADCTL_RGB;
            break;
        case 2:
            r = ST77916_MADCTL_MX | ST77916_MADCTL_MY | ST77916_MADCTL_RGB;
            break;
        case 3:
            r = ST77916_MADCTL_MY | ST77916_MADCTL_MV | ST77916_MADCTL_RGB;
            break;
        default:  // case 0:
            r = ST77916_MADCTL_RGB;
            break;
    }

    LCD_Write_C8D8(ST77916_MADCTL, r);
}

void QSPI_LCD_Init()
{

    GPIO_Config();
    QSPI1_Config();

    QSPI_Cmd(QSPI1, ENABLE);
    QSPI_EnableQuad(QSPI1, ENABLE);

    Delay_Ms(120);
    LCD_Write_C8(ST77916_SWRESET);
    Delay_Ms(120);
    LCD_do_init(st77916_init_code, sizeof(st77916_init_code));

    LCD_Write_C8(ST77916_INVON);
    LCD_Write_C8(ST77916_SLPOUT);
    LCD_Write_C8(ST77916_DISPON);
    LCD_setRotation(1);
}

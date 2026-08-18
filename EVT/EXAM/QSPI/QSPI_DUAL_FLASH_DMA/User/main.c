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
 *QSPI routine:
 *This example demonstrates how to use dual QSPI flash to read data in DMA mode.
 *
 *   hardware connection:	
 * PB2  -- FLASH_SCK
 * PB6  -- FLASH1_CS
 * PD11 -- FLASH1_IO0
 * PD12 -- FLASH1_IO1
 * PE6  -- FLASH1_IO2
 * PD13 -- FLASH1_IO3
 * PC11 -- FLASH2_CS
 * PE7  -- FLASH2_IO0
 * PE8  -- FLASH2_IO1
 * PE9  -- FLASH2_IO2
 * PE10 -- FLASH2_IO3
 *
 */

#include "debug.h"
#include "string.h"
#include "ch32v205_qspi.h"

#define FLASH_CMD_EnableReset             0x66
#define FLASH_CMD_ResetDevice             0x99
#define FLASH_CMD_JedecID                 0x9F
#define FLASH_CMD_WriteEnable             0X06
#define FLASH_CMD_WriteDisable            0X04
#define FLASH_CMD_ManufactDeviceID        0x90
#define FLASH_CMD_ManufactDeviceID_DualIO 0x92
#define FLASH_CMD_ManufactDeviceID_QuadIO 0x94
#define FLASH_CMD_SectorErase             0x20
#define FLASH_CMD_Read                    0x03
#define FLASH_CMD_FastRead                0x0B
#define FLASH_CMD_FastReadQuadIO          0xEB
#define FLASH_CMD_FastReadDualIO          0xBB
#define FLASH_CMD_PageProgram             0x02
#define FLASH_CMD_PageProgramQuad         0x32
#define FLASH_CMD_ReadStatus_REG1         0X05
#define FLASH_CMD_ReadStatus_REG2         0X35

#define FLASH_SECTOR_SIZE                 (4096U)
#define FLASH_PAGE_SIZE                   (256U)

uint8_t readBuffer[512];
uint8_t verifyBuffer[512];
uint8_t writeBuffer[512];

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
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOC, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOD, ENABLE);
    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOE, ENABLE);

    // QSPI_SCK PB2(AF5)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource2, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // QSPI_SCSN PB6(AF5)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // QSPI_SIO0 PD11(AF5)
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // QSPI_SIO1 PD12(AF5)
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // QSPI_SIO2 PE6(AF5)
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // QSPI_SIO3 PD13(AF5)
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // QSPI_SCSXN PC11(AF5)
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // QSPI_SIOX0 PE7(AF5)
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // QSPI_SIOX1 PE8(AF5)
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // QSPI_SIOX2 PE9(AF5)
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // QSPI_SIOX3 PE10(AF5)
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF5);
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
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

    QSPI_InitStructure.QSPI_Prescaler = 8;
    QSPI_InitStructure.QSPI_CKMode    = QSPI_CKMode_Mode3;
    QSPI_InitStructure.QSPI_CSHTime   = QSPI_CSHTime_7Cycle;

    // size = 2 ** (FSize + 1) = 2 ** 23 = 8MB
    QSPI_InitStructure.QSPI_FSize = 22;

    QSPI_InitStructure.QSPI_FSelect = QSPI_FSelect_2;
    QSPI_InitStructure.QSPI_DFlash  = QSPI_DFlash_Enable;

    QSPI_Init(QSPI1, &QSPI_InitStructure);
    QSPI_SetFIFOThreshold(QSPI1, 5);
    QSPI_SetDataLength(QSPI1, 1);
}

void QSPI_Rx_DMA(uint8_t* buffer, uint32_t len)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    DMA_DeInit(DMA2_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (QSPI1->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr    = (u32)buffer;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = len / 4;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel1, &DMA_InitStructure);

    DMA_ClearFlag(DMA2, DMA_FLAG_TC1);
}

void QSPI_Tx_DMA(uint8_t* buffer, uint32_t len)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    DMA_DeInit(DMA2_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (QSPI1->DR);
    DMA_InitStructure.DMA_Memory0BaseAddr    = (u32)buffer;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize         = len / 4;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel1, &DMA_InitStructure);

    DMA_ClearFlag(DMA2, DMA_FLAG_TC1);
}

/*********************************************************************
 * @fn      QSPI_CmdWithAddress
 * 
 * @brief   Send a command with an address to the QSPI
 * 
 * @param   cmd - the command to be sent
 *          addr - the address to be sent
 * 
 * @return  none
 */
void QSPI_CmdWithAddress(uint8_t cmd, uint32_t addr)
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
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = cmd;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, addr);

    QSPI_Start(QSPI1);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
}

/*********************************************************************
 * @fn      QSPI_WriteCmd
 * 
 * @brief   Write a command to the QSPI
 * 
 * @param   cmd - the command to be written
 * 
 * @return  none
 */
void QSPI_WriteCmd(uint8_t cmd)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_NoAddress;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_NoData;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = cmd;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;

    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_Start(QSPI1);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
}

/*********************************************************************
 * @fn      QSPI_CmdWithData
 * 
 * @brief   Send a command with data to the QSPI
 * 
 * @param   cmd - the command to be sent
 *          data - the data to be sent
 * 
 * @return  none
 */
void QSPI_CmdWithData(uint8_t cmd, uint16_t data)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_NoAddress;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Write;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = cmd;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetDataLength(QSPI1, 2);

    QSPI_Start(QSPI1);

    QSPI_SendData16(QSPI1, data);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
}

/*********************************************************************
 * @fn      QSPI_ReadSR
 * 
 * @brief   Read the status register of the QSPI
 * 
 * @param   cmd - the command to be written
 * 
 * @return  the status register value
 */
uint16_t QSPI_ReadSR(uint8_t cmd)
{
    uint32_t res = 0;
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_NoAddress;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = cmd;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetDataLength(QSPI1, 2);

    QSPI_Start(QSPI1);
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_FT) == RESET)
    {
    }

    res = QSPI_ReceiveData16(QSPI1);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    return res;
}

/*********************************************************************
 * @fn      QSPI_WaitForBsy
 * 
 * @brief   Wait for the QSPI to be ready
 * 
 * @param   none
 * 
 * @return  none
 */
void QSPI_WaitForBsy(void)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_AutoPollingMode_Config(QSPI1, 0x0000, 0x0101, QSPI_PMM_AND);
    QSPI_AutoPollingModeStopCmd(QSPI1, ENABLE);

    QSPI_AutoPollingMode_SetInterval(QSPI1, 2500);

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Auto_Polling;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_NoAddress;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_ReadStatus_REG1;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetDataLength(QSPI1, 2);

    QSPI_Start(QSPI1);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_SM) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_SM);

    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
}

/*********************************************************************
 * @fn      QSPI_WriteEnable
 * 
 * @brief   Enable the write operation
 * 
 * @param   none
 * 
 * @return  none
 */
static inline void QSPI_WriteEnable(void)
{
    QSPI_WriteCmd(FLASH_CMD_WriteEnable);
}

/*********************************************************************
 * @fn      QSPI_WriteDisable
 * 
 * @brief   Disable the write operation
 * 
 * @param   none
 * 
 * @return  none
 */
static inline void QSPI_WriteDisable(void)
{
    QSPI_WriteCmd(FLASH_CMD_WriteDisable);
}

/*********************************************************************
 * @fn      QSPI_EraseSector
 * 
 * @brief   Erase a sector of the QSPI
 * 
 * @param   addr - the address of the sector to be erased
 * 
 * @return  none
 */
void QSPI_EraseSector(uint32_t addr)
{
    QSPI_WriteEnable();
    QSPI_CmdWithAddress(FLASH_CMD_SectorErase, addr);
    QSPI_WaitForBsy();
}

/*********************************************************************
 * @fn      QSPI_WritePage
 * 
 * @brief   Write a page of the QSPI
 * 
 * @param   addr - the address of the page to be written
 *          data - the data to be written
 *          len - the length of the data to be written
 * 
 * @return  none
 */
void QSPI_WritePage(uint32_t addr, uint8_t* data, uint16_t len)
{

    QSPI_AbortRequest(QSPI1);
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    {
    }
    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Write;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_PageProgram;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, addr);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_DMACmd(QSPI1, ENABLE);
    QSPI_Tx_DMA(data, len);
    DMA_Cmd(DMA2_Channel1, ENABLE);

    QSPI_Start(QSPI1);

    while (DMA_GetFlagStatus(DMA2, DMA_FLAG_TC1) == RESET)
    {
    }

    QSPI_DMACmd(QSPI1, DISABLE);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
}

/*********************************************************************
 * @fn      QSPI_WritePageQuad
 * 
 * @brief   Write a page of the QSPI in 4 lines mode
 * 
 * @param   addr - the address of the page to be written
 *          data - the data to be written
 *          len - the length of the data to be written
 * 
 * @return  none
 */
void QSPI_WritePageQuad(uint32_t addr, uint8_t* data, uint16_t len)
{

    QSPI_AbortRequest(QSPI1);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    {
    }
    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_4Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Write;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_PageProgramQuad;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, addr);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_EnableQuad(QSPI1, ENABLE);

    QSPI_DMACmd(QSPI1, ENABLE);
    QSPI_Tx_DMA(data, len);
    DMA_Cmd(DMA2_Channel1, ENABLE);

    QSPI_Start(QSPI1);

    while (DMA_GetFlagStatus(DMA2, DMA_FLAG_TC1) == RESET)
    {
    }

    QSPI_DMACmd(QSPI1, DISABLE);

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    QSPI_EnableQuad(QSPI1, DISABLE);
}

/*********************************************************************
 * @fn      QSPI_ProgramPage
 * 
 * @brief   Program a page of the QSPI
 * 
 * @param   addr - the address of the page to be programmed
 *          data - the data to be programmed
 *          len - the length of the data to be programmed
 * 
 * @return  none
 */
void QSPI_ProgramPage(uint32_t addr, uint8_t* data, uint16_t len)
{
    QSPI_WriteEnable();
    QSPI_WritePage(addr, data, len);
    QSPI_WaitForBsy();
}

/*********************************************************************
 * @fn      QSPI_ProgramPageQuad
 * 
 * @brief   Program a page of the QSPI in 4 lines mode
 * 
 * @param   addr - the address of the page to be programmed
 *          data - the data to be programmed
 *          len - the length of the data to be programmed
 * 
 * @return  none
 */
void QSPI_ProgramPageQuad(uint32_t addr, uint8_t* data, uint16_t len)
{
    QSPI_WriteEnable();
    QSPI_WritePageQuad(addr, data, len);
    QSPI_WaitForBsy();
}
/*********************************************************************
 * @fn      QSPI_GetManufacturerID
 * 
 * @brief   Read the manufacturer and device ID of the QSPI
 * 
 * @param   none
 * 
 * @return  the manufacturer and device ID of the QSPI
 */
uint32_t QSPI_GetManufacturerID()
{
    uint32_t res = 0;
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_ManufactDeviceID;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, 0);
    QSPI_SetDataLength(QSPI1, 4);

    QSPI_Start(QSPI1);

    uint32_t i = 0;

    while (i < 4)
    {
        if (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_FT))
        {
            res <<= 8;
            res |= QSPI_ReceiveData8(QSPI1);
            i++;
        }
    }

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    return res;
}

/*********************************************************************
 * @fn      QSPI_GetManufacturerID_DualIO
 * 
 * @brief   Read the manufacturer and device ID of the QSPI in 2 lines mode
 * 
 * @param   none
 * 
 * @return  the manufacturer and device ID of the QSPI
 */
uint32_t QSPI_GetManufacturerID_DualIO()
{
    uint32_t res = 0;
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_2Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_2Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_ManufactDeviceID_DualIO;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 4;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetDataLength(QSPI1, 4);
    QSPI_EnableQuad(QSPI1, ENABLE);
    QSPI_Start(QSPI1);

    uint32_t i = 0;
    while (i < 4)
    {
        if (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_FT))
        {
            res <<= 8;
            res |= QSPI_ReceiveData8(QSPI1);
            i++;
        }
    }

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    QSPI_EnableQuad(QSPI1, DISABLE);
    return res;
}

/*********************************************************************
 * @fn      QSPI_GetManufacturerID_QuadIO
 * 
 * @brief   Read the manufacturer and device ID of the QSPI in 4 lines mode
 * 
 * @param   none
 * 
 * @return  the manufacturer and device ID of the QSPI
 */
uint32_t QSPI_GetManufacturerID_QuadIO()
{
    uint32_t res = 0;
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_4Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_4Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_ManufactDeviceID_QuadIO;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 6;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetDataLength(QSPI1, 4);
    QSPI_EnableQuad(QSPI1, ENABLE);
    QSPI_Start(QSPI1);

    uint32_t i = 0;
    while (i < 4)
    {
        if (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_FT))
        {
            res <<= 8;
            res |= QSPI_ReceiveData8(QSPI1);
            i++;
        }
    }

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    QSPI_EnableQuad(QSPI1, DISABLE);
    return res;
}

/*********************************************************************
 * @fn      QSPI_GetData
 * 
 * @brief   Read data from the QSPI
 * 
 * @param   addr - the address of the data to be read
 *          data - the buffer to store the data
 *          len - the length of the data to be read
 * 
 * @return  none
 */
void QSPI_GetData(uint32_t addr, uint8_t* data, uint32_t len)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_FastRead;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 8;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, addr);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_Rx_DMA(data, len);
    QSPI_DMACmd(QSPI1, ENABLE);
    QSPI_Start(QSPI1);
    DMA_Cmd(DMA2_Channel1, ENABLE);

    while (DMA_GetFlagStatus(DMA2, DMA_FLAG_TC1) == RESET)
    {
    }

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    QSPI_DMACmd(QSPI1, DISABLE);
}

void QSPI_GetData_DualIO(uint32_t addr, uint8_t* data, uint32_t len)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_2Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_2Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_FastReadDualIO;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 4;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, addr);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_Rx_DMA(data, len);
    QSPI_DMACmd(QSPI1, ENABLE);
    QSPI_Start(QSPI1);
    DMA_Cmd(DMA2_Channel1, ENABLE);

    while (DMA_GetFlagStatus(DMA2, DMA_FLAG_TC1) == RESET)
    {
    }

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    QSPI_DMACmd(QSPI1, DISABLE);
}

/*********************************************************************
 * @fn      QSPI_GetData_QuadIO
 * 
 * @brief   Read data from the QSPI in 4 lines mode
 * 
 * @param   addr - the address of the data to be read
 *          data - the buffer to store the data
 *          len - the length of the data to be read
 * 
 * @return  none    
 */
void QSPI_GetData_QuadIO(uint32_t addr, uint8_t* data, uint32_t len)
{
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }

    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode       = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode      = QSPI_ComConfig_ADMode_4Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode       = QSPI_ComConfig_DMode_4Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABMode      = QSPI_ComConfig_ABMode_NoAlternateByte;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode       = QSPI_ComConfig_FMode_Indirect_Read;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_SIOOMode    = QSPI_ComConfig_SIOOMode_Disable;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ABSize      = QSPI_ComConfig_ABSize_8bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize      = QSPI_ComConfig_ADSize_24bit;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins         = FLASH_CMD_FastReadQuadIO;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 6;
    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);

    QSPI_SetAddress(QSPI1, addr);
    QSPI_SetDataLength(QSPI1, len);

    QSPI_EnableQuad(QSPI1, ENABLE);
    QSPI_Rx_DMA(data, len);
    QSPI_DMACmd(QSPI1, ENABLE);
    QSPI_Start(QSPI1);
    DMA_Cmd(DMA2_Channel1, ENABLE);

    while (DMA_GetFlagStatus(DMA2, DMA_FLAG_TC1) == RESET)
    {
    }

    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_TC) == RESET)
    {
    }
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_FT);
    QSPI_ClearFlag(QSPI1, QSPI_FLAG_TC);
    QSPI_DMACmd(QSPI1, DISABLE);
    QSPI_EnableQuad(QSPI1, DISABLE);
}

/*********************************************************************
 * @fn      Buffer_Cmp
 * 
 * @brief   Compare two buffers
 * 
 * @param   buf1 - the first buffer
 *          buf2 - the second buffer
 *          len - the length of the buffers
 * 
 * @return  the index of the first different byte, or -1 if the buffers are identical
 */
int32_t Buffer_Cmp(uint8_t* buf1, uint8_t* buf2, uint32_t len)
{
    int32_t res = -1;
    for (int i = 0; i < len; i++)
    {
        if (buf1[i] != buf2[i])
        {
            res = i;
            break;
        }
    }
    return res;
}

/*********************************************************************
 * @fn      Print_Buffer
 * 
 * @brief   Print a buffer
 * 
 * @param   buf - the buffer to print
 *          len - the length of the buffer
 * 
 * @return  none
 */
void Print_Buffer(u8* buf, u32 len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\nEND\n");
}

/*********************************************************************
 * @fn      write_data
 * 
 * @brief   Write  data to the FLASH
 * 
 * @param   addr - The address to be written.It must be a 4K-aligned address
 *          data - the data to be Wrote
 *          len - The length of the written data.It must be a multiple of 512
 * 
 * @return  none
 */
void write_data(uint32_t addr, uint8_t* data, size_t len)
{
    uint32_t current_addr = addr;
    uint8_t* current_data = data;
    size_t   remaining    = len;

    while (remaining > 0)
    {
        if (current_addr % FLASH_SECTOR_SIZE == 0)
        {
            QSPI_EraseSector(current_addr);
        }

        QSPI_ProgramPageQuad(current_addr, current_data, FLASH_PAGE_SIZE);
        current_addr += FLASH_PAGE_SIZE;
        current_data += FLASH_PAGE_SIZE;
        remaining -= FLASH_PAGE_SIZE;
    }
}

/*********************************************************************
 * @fn      main
 * 
 * @brief   Main function
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
    printf("QSPI TEST\n");

    GPIO_Config();
    QSPI1_Config();

    QSPI_Cmd(QSPI1, ENABLE);

    QSPI_WriteCmd(FLASH_CMD_EnableReset);
    QSPI_WriteCmd(FLASH_CMD_ResetDevice);
    Delay_Us(100);
    QSPI_WaitForBsy();

#if 0
    // Some chips do not have QUAD mode enabled by default.
    // Use the following command to enable QUAD mode
    QSPI_WriteEnable();
    QSPI_WriteCmd(0x50);
    QSPI_CmdWithData(0x31, 0x0202);
#endif

    printf("GetManufacturerID %x\n", QSPI_GetManufacturerID());
    printf("GetManufacturerID_DualIO %x\n", QSPI_GetManufacturerID_DualIO());
    printf("GetManufacturerID_QuadIO %x\n", QSPI_GetManufacturerID_QuadIO());
    printf("sr1 %x\n", QSPI_ReadSR(FLASH_CMD_ReadStatus_REG1));
    printf("sr2 %x\n", QSPI_ReadSR(FLASH_CMD_ReadStatus_REG2));

    const uint32_t rd_addr = 0x1000;
    const uint32_t rd_len  = 256;
    int32_t        res     = 0;

    printf("one line read\n");
    QSPI_GetData(rd_addr, verifyBuffer, rd_len);
    Print_Buffer(verifyBuffer, rd_len);
    Delay_Ms(1);

    printf("DualIO read\n");
    memset(readBuffer, 0, rd_len);
    QSPI_GetData_DualIO(rd_addr, readBuffer, rd_len);

    res = Buffer_Cmp(verifyBuffer, readBuffer, rd_len);

    if (res != -1)
    {
        printf("DualIO read error %d\n", res);
        Print_Buffer(readBuffer, rd_len);
    }
    else
    {
        printf("DualIO read success\n");
    }

    printf("QuadIO read\n");
    memset(readBuffer, 0, rd_len);
    QSPI_GetData_QuadIO(rd_addr, readBuffer, rd_len);
    res = Buffer_Cmp(verifyBuffer, readBuffer, rd_len);

    if (res != -1)
    {
        printf("QuadIO read error %d\n", res);
        Print_Buffer(readBuffer, rd_len);
    }
    else
    {
        printf("QuadIO read success\n");
    }

    printf("read test and verify end\n");

    while (1)
    {
    }
}

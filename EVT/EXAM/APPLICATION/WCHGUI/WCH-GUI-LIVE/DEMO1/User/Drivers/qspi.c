/*********************************************************************
 * File Name          : qspi.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2026/08/12
 * Description        : QSPI driver for the 4-line (SIOX) LCD panel.
 *********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "qspi.h"

#define QSPI_LCD_DMA_CH         DMA2_Channel1
#define QSPI_LCD_DMA_TC_FLAG    DMA_FLAG_TC1

static void QSPI_LCD_GPIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB | RCC_PB2Periph_AFIO |
                          QSPI_LCD_CS_RCC, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource2, GPIO_AF5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF5);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_8 | GPIO_Pin_9 |
                                    GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = QSPI_LCD_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(QSPI_LCD_CS_PORT, &GPIO_InitStructure);

    QSPI_LCD_CS_SET();
}

static void QSPI_LCD_DMAInit(void)
{
    DMA_InitTypeDef DMA_InitStructure = {0};

    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA2, ENABLE);

    DMA_DeInit(QSPI_LCD_DMA_CH);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&QSPI1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr    = (uint32_t)NULL;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize         = 0;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(QSPI_LCD_DMA_CH, &DMA_InitStructure);
}

static void QSPI_LCD_WaitTC(void)
{
    uint32_t timeout = 0x1000000u;

    while ((QSPI1->SR & QSPI_FLAG_TC) == 0u)
    {
        if (--timeout == 0u)
        {
            break;
        }
    }

    QSPI1->FCR = QSPI_FLAG_TC;
}

static void QSPI_LCD_WaitIdle(void)
{
    uint32_t timeout = 0x1000000u;

    while ((QSPI1->SR & QSPI_FLAG_IDLE) == 0u)
    {
        if (--timeout == 0u)
        {
            break;
        }
    }
}

static void QSPI_LCD_WaitDMA(void)
{
    uint32_t timeout = 0x1000000u;

    while (QSPI_LCD_DMA_CH->CNTR > 0u)
    {
        if (--timeout == 0u)
        {
            DMA_Cmd(QSPI_LCD_DMA_CH, DISABLE);
            break;
        }
    }
}

static void QSPI_LCD_Config(uint32_t ins, uint32_t dmode, uint32_t adsize)
{
    QSPI_ComConfig_InitTypeDef QSPI_ComConfig_InitStructure = {0};

    QSPI_ComConfig_StructInit(&QSPI_ComConfig_InitStructure);

    QSPI_ComConfig_InitStructure.QSPI_ComConfig_FMode = QSPI_ComConfig_FMode_Indirect_Write;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_IMode = QSPI_ComConfig_IMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_Ins   = ins;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADMode  = QSPI_ComConfig_ADMode_1Line;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_ADSize  = adsize;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DMode   = dmode;
    QSPI_ComConfig_InitStructure.QSPI_ComConfig_DummyCycles = 0;

    QSPI_ComConfig_Init(QSPI1, &QSPI_ComConfig_InitStructure);
}

/*********************************************************************
 * @fn      QSPI_LCD_Init
 *
 * @brief   Initialize QSPI1 and its GPIO for the 4-line LCD panel
 *
 * @return  none
 */
void QSPI_LCD_Init(void)
{
    QSPI_InitTypeDef QSPI_InitStructure = {0};

    QSPI_LCD_GPIOInit();

    RCC_HBPeriphClockCmd(RCC_HBPeriph_QSPI1, ENABLE);

    QSPI_DeInit(QSPI1);

    QSPI_InitStructure.QSPI_Prescaler = QSPI_LCD_PRESCALER;
    QSPI_InitStructure.QSPI_CKMode    = QSPI_CKMode_Mode3;
    QSPI_InitStructure.QSPI_CSHTime   = QSPI_CSHTime_7Cycle;
    QSPI_InitStructure.QSPI_FSize     = QSPI_LCD_FSIZE;
    QSPI_InitStructure.QSPI_FSelect   = QSPI_FSelect_2;
    QSPI_InitStructure.QSPI_DFlash    = QSPI_DFlash_Disable;
    QSPI_Init(QSPI1, &QSPI_InitStructure);

    QSPI_Cmd(QSPI1, ENABLE);
    QSPI_EnableQuad(QSPI1, ENABLE);

    QSPI_LCD_DMAInit();
}

/*********************************************************************
 * @fn      QSPI_LCD_Cmd
 *
 * @brief   Send one command byte, no parameter
 *
 * @param   cmd - the command to send
 *
 * @return  none
 */
void QSPI_LCD_Cmd(uint8_t cmd)
{
    QSPI_LCD_Config(QSPI_LCD_INS_SINGLE, QSPI_ComConfig_DMode_NoData,
                    QSPI_ComConfig_ADSize_24bit);

    QSPI_SetAddress(QSPI1, ((uint32_t)cmd) << 8);

    QSPI1->FCR = QSPI_FLAG_TC;

    QSPI_LCD_CS_CLR();
    QSPI_Start(QSPI1);
    QSPI_LCD_WaitTC();
    QSPI_LCD_WaitIdle();
    QSPI_LCD_CS_SET();
}

/*********************************************************************
 * @fn      QSPI_LCD_CmdData
 *
 * @brief   Send one command byte followed by its parameters (1-line)
 *
 * @param   cmd  - the command to send
 *          data - the parameter bytes
 *          len  - the length of the parameters
 *
 * @return  none
 */
void QSPI_LCD_CmdData(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    uint16_t i;

    if ((data == 0) || (len == 0u))
    {
        QSPI_LCD_Cmd(cmd);
        return;
    }

    QSPI_LCD_Config(QSPI_LCD_INS_SINGLE, QSPI_ComConfig_DMode_1Line,
                    QSPI_ComConfig_ADSize_24bit);

    QSPI_SetDataLength(QSPI1, len);
    QSPI_SetAddress(QSPI1, ((uint32_t)cmd) << 8);

    QSPI1->FCR = QSPI_FLAG_TC;

    QSPI_LCD_CS_CLR();
    QSPI_Start(QSPI1);

    for (i = 0; i < len; i++)
    {
        uint32_t timeout = 0x10000u;

        while ((QSPI1->SR & QSPI_FLAG_FT) == 0u)
        {
            if (--timeout == 0u)
            {
                break;
            }
        }

        QSPI_SendData8(QSPI1, data[i]);
    }

    QSPI_LCD_WaitTC();
    QSPI_LCD_WaitIdle();
    QSPI_LCD_CS_SET();
}

/*********************************************************************
 * @fn      QSPI_LCD_CmdPixels
 *
 * @brief   Send one command byte followed by pixels (4-line data phase)
 *
 * @param   cmd    - the memory write command
 *          pixels - the RGB565 pixels
 *          count  - the pixel count
 *
 * @return  none
 */
void QSPI_LCD_CmdPixels(uint8_t cmd, const uint16_t *pixels, uint32_t count)
{
    while (count > 0u)
    {
        uint16_t n = (count > 0xFFFFu) ? (uint16_t)0xFFFFu : (uint16_t)count;

        QSPI_LCD_Config(QSPI_LCD_INS_QUAD, QSPI_ComConfig_DMode_4Line,
                        QSPI_ComConfig_ADSize_24bit);

        QSPI_SetDataLength(QSPI1, (uint32_t)n * 2u);
        QSPI_SetAddress(QSPI1, ((uint32_t)cmd) << 8);

        DMA_Cmd(QSPI_LCD_DMA_CH, DISABLE);
        DMA_ClearFlag(DMA2, QSPI_LCD_DMA_TC_FLAG);
        QSPI_LCD_DMA_CH->MADDR = (uint32_t)pixels;
        QSPI_LCD_DMA_CH->CNTR  = n;

        QSPI1->FCR = QSPI_FLAG_TC;

        QSPI_LCD_CS_CLR();
        QSPI_DMACmd(QSPI1, ENABLE);
        DMA_Cmd(QSPI_LCD_DMA_CH, ENABLE);
        QSPI_Start(QSPI1);

        QSPI_LCD_WaitDMA();
        QSPI_LCD_WaitTC();
        QSPI_LCD_WaitIdle();

        QSPI_DMACmd(QSPI1, DISABLE);
        QSPI_LCD_CS_SET();

        pixels += n;
        count -= n;
    }
}

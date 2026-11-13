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

#define LCD_SOFT_RESET          (0x01)
#define LCD_READ_ID             (0x04)
#define LCD_READ_DISPLAY_STATUS (0x09)
#define LCD_ON                  (0x28)
#define LCD_OFF                 (0x29)

#define LCD_SET_X               (0x2a)
#define LCD_SET_Y               (0x2b)
#define LCD_MEM_WRITE           (0x2C)
#define LCD_MEM_READ            (0x2e)

static void LCD_WR_REG(uint8_t data)
{
    LCD_DC_CLR();
    SPI2_Write(data);
}

static void LCD_WR_DATA(uint8_t data)
{
    LCD_DC_SET();
    SPI2_Write(data);
}

/*********************************************************************
 * @fn      Lcd_WriteData_16Bit
 * 
 * @brief   write two bytes to the lcd screen
 * 
 * @param   Data - the data to write
 * 
 * @return  none
 */
void Lcd_WriteData_16Bit(uint16_t Data)
{
    SPI2_Write(Data >> 8);
    SPI2_Write(Data);
}

/*********************************************************************
 * @fn      LCD_GPIOInit
 * 
 * @brief   Configuring the control gpio for the lcd screen
 * 
 * @return  none
 */
void LCD_GPIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_PB2PeriphClockCmd(LCD_LED_RCC, ENABLE);
    RCC_PB2PeriphClockCmd(LCD_DC_RCC, ENABLE);
    RCC_PB2PeriphClockCmd(LCD_RST_RCC, ENABLE);
    RCC_PB2PeriphClockCmd(LCD_CS_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = LCD_LED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_LED_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = LCD_DC_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_DC_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = LCD_RST_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_RST_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = LCD_CS_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_High;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_CS_PORT, &GPIO_InitStructure);

    LCD_LED_CLR();
    LCD_CS_SET();
}

/*********************************************************************
 * @fn      LCD_RESET
 * 
 * @brief   reset the lcd screen by the rst pin
 * 
 * @return  none
 */
void LCD_RESET(void)
{
    Delay_Ms(50);
    LCD_RST_CLR();
    Delay_Ms(100);
    LCD_RST_SET();
    Delay_Ms(50);
}

static const uint8_t lcd_st7789_init_cmds[] = {

    1,
    0x3A,
    0x05,

    1,
    0x36,
    LCD_MADCTL,

    5,
    0xB2,
    0x0C,
    0x0C,
    0x00,
    0x33,
    0x33,

    1,
    0xB7,
    0x35,

    1,
    0xBB,
    0x19,

    1,
    0xC0,
    0x2C,

    1,
    0xC2,
    0x01,

    1,
    0xC3,
    0x12,

    1,
    0xC4,
    0x20,

    1,
    0xC6,
    0x0F,

    2,
    0xD0,
    0xA4,
    0xA1,

    14,
    0xE0,
    0xD0,
    0x04,
    0x0D,
    0x11,
    0x13,
    0x2B,
    0x3F,
    0x54,
    0x4C,
    0x18,
    0x0D,
    0x0B,
    0x1F,
    0x23,

    14,
    0xE1,
    0xD0,
    0x04,
    0x0C,
    0x11,
    0x13,
    0x2C,
    0x3F,
    0x44,
    0x51,
    0x2F,
    0x1F,
    0x1F,
    0x20,
    0x23,

    0,
    0x21,

    0,
    0x13,

    0xff,
    0xff,
};

#if 0
#define LCD_WRITE_TYPE_D8   1
#define LCD_WRITE_TYPE_D16  2
#define LCD_WRITE_TYPE_Dn   3
#define LCD_WRITE_TYPE_END  0xFF

static const uint8_t lcd_st77916_init_cmds[] = {
    LCD_WRITE_TYPE_D8, 0xF0, 0x28,
    LCD_WRITE_TYPE_D8, 0xF2, 0x28,
    LCD_WRITE_TYPE_D8, 0x73, 0xF0,
    LCD_WRITE_TYPE_D8, 0x7C, 0xD1,
    LCD_WRITE_TYPE_D8, 0x83, 0xE0,
    LCD_WRITE_TYPE_D8, 0x84, 0x61,
    LCD_WRITE_TYPE_D8, 0xF2, 0x82,
    LCD_WRITE_TYPE_D8, 0xF0, 0x00,

    LCD_WRITE_TYPE_D8, 0xF0, 0x01,
    LCD_WRITE_TYPE_D8, 0xF1, 0x01,
    LCD_WRITE_TYPE_D8, 0xB0, 0x5E,
    LCD_WRITE_TYPE_D8, 0xB1, 0x55,
    LCD_WRITE_TYPE_D8, 0xB2, 0x24,
    LCD_WRITE_TYPE_D8, 0xB3, 0x01,
    LCD_WRITE_TYPE_D8, 0xB4, 0x87,
    LCD_WRITE_TYPE_D8, 0xB5, 0x44,
    LCD_WRITE_TYPE_D8, 0xB6, 0x8B,
    LCD_WRITE_TYPE_D8, 0xB7, 0x40,
    LCD_WRITE_TYPE_D8, 0xB8, 0x86,
    LCD_WRITE_TYPE_D8, 0xB9, 0x15,
    LCD_WRITE_TYPE_D8, 0xBA, 0x00,
    LCD_WRITE_TYPE_D8, 0xBB, 0x08,
    LCD_WRITE_TYPE_D8, 0xBC, 0x08,
    LCD_WRITE_TYPE_D8, 0xBD, 0x00,
    LCD_WRITE_TYPE_D8, 0xBE, 0x00,
    LCD_WRITE_TYPE_D8, 0xBF, 0x07,
    LCD_WRITE_TYPE_D8, 0xC0, 0x80,
    LCD_WRITE_TYPE_D8, 0xC1, 0x10,
    LCD_WRITE_TYPE_D8, 0xC2, 0x37,
    LCD_WRITE_TYPE_D8, 0xC3, 0x80,
    LCD_WRITE_TYPE_D8, 0xC4, 0x10,
    LCD_WRITE_TYPE_D8, 0xC5, 0x37,
    LCD_WRITE_TYPE_D8, 0xC6, 0xA9,
    LCD_WRITE_TYPE_D8, 0xC7, 0x41,
    LCD_WRITE_TYPE_D8, 0xC8, 0x01,
    LCD_WRITE_TYPE_D8, 0xC9, 0xA9,
    LCD_WRITE_TYPE_D8, 0xCA, 0x41,
    LCD_WRITE_TYPE_D8, 0xCB, 0x01,
    LCD_WRITE_TYPE_D8, 0xCC, 0x7F,
    LCD_WRITE_TYPE_D8, 0xCD, 0x7F,
    LCD_WRITE_TYPE_D8, 0xCE, 0xFF,
    LCD_WRITE_TYPE_D8, 0xD0, 0x91,
    LCD_WRITE_TYPE_D8, 0xD1, 0x68,
    LCD_WRITE_TYPE_D8, 0xD2, 0x68,
    LCD_WRITE_TYPE_D16, 0xF5, 0x00, 0xA5,
    LCD_WRITE_TYPE_D8, 0xDD, 0x40,
    LCD_WRITE_TYPE_D8, 0xDE, 0x40,
    LCD_WRITE_TYPE_D8, 0xF1, 0x10,
    LCD_WRITE_TYPE_D8, 0xF0, 0x00,

    LCD_WRITE_TYPE_D8, 0xF0, 0x02,
    LCD_WRITE_TYPE_Dn, 0xE0, 14,
    0xF0, 0x10, 0x18, 0x0D, 0x0C, 0x38, 0x3E, 0x44,
    0x51, 0x39, 0x15, 0x15, 0x30, 0x34,
    LCD_WRITE_TYPE_Dn, 0xE1, 14,
    0xF0, 0x0F, 0x17, 0x0D, 0x0B, 0x07, 0x3E, 0x33,
    0x51, 0x39, 0x15, 0x15, 0x30, 0x34,

    LCD_WRITE_TYPE_D8, 0xF0, 0x10,
    LCD_WRITE_TYPE_D8, 0xF3, 0x10,
    LCD_WRITE_TYPE_D8, 0xE0, 0x08,
    LCD_WRITE_TYPE_D8, 0xE1, 0x00,
    LCD_WRITE_TYPE_D8, 0xE2, 0x00,
    LCD_WRITE_TYPE_D8, 0xE3, 0x00,
    LCD_WRITE_TYPE_D8, 0xE4, 0xE0,
    LCD_WRITE_TYPE_D8, 0xE5, 0x06,
    LCD_WRITE_TYPE_D8, 0xE6, 0x21,
    LCD_WRITE_TYPE_D8, 0xE7, 0x03,
    LCD_WRITE_TYPE_D8, 0xE8, 0x05,
    LCD_WRITE_TYPE_D8, 0xE9, 0x02,
    LCD_WRITE_TYPE_D8, 0xEA, 0xE9,
    LCD_WRITE_TYPE_D8, 0xEB, 0x00,
    LCD_WRITE_TYPE_D8, 0xEC, 0x00,
    LCD_WRITE_TYPE_D8, 0xED, 0x14,
    LCD_WRITE_TYPE_D8, 0xEE, 0xFF,
    LCD_WRITE_TYPE_D8, 0xEF, 0x00,
    LCD_WRITE_TYPE_D8, 0xF8, 0xFF,
    LCD_WRITE_TYPE_D8, 0xF9, 0x00,
    LCD_WRITE_TYPE_D8, 0xFA, 0x00,
    LCD_WRITE_TYPE_D8, 0xFB, 0x30,
    LCD_WRITE_TYPE_D8, 0xFC, 0x00,
    LCD_WRITE_TYPE_D8, 0xFD, 0x00,
    LCD_WRITE_TYPE_D8, 0xFE, 0x00,
    LCD_WRITE_TYPE_D8, 0xFF, 0x00,
    LCD_WRITE_TYPE_D8, 0x60, 0x40,
    LCD_WRITE_TYPE_D8, 0x61, 0x05,
    LCD_WRITE_TYPE_D8, 0x62, 0x00,
    LCD_WRITE_TYPE_D8, 0x63, 0x42,
    LCD_WRITE_TYPE_D8, 0x64, 0xDA,
    LCD_WRITE_TYPE_D8, 0x65, 0x00,
    LCD_WRITE_TYPE_D8, 0x66, 0x00,
    LCD_WRITE_TYPE_D8, 0x67, 0x00,
    LCD_WRITE_TYPE_D8, 0x68, 0x00,
    LCD_WRITE_TYPE_D8, 0x69, 0x00,
    LCD_WRITE_TYPE_D8, 0x6A, 0x00,
    LCD_WRITE_TYPE_D8, 0x6B, 0x00,
    LCD_WRITE_TYPE_D8, 0x70, 0x40,
    LCD_WRITE_TYPE_D8, 0x71, 0x04,
    LCD_WRITE_TYPE_D8, 0x72, 0x00,
    LCD_WRITE_TYPE_D8, 0x73, 0x42,
    LCD_WRITE_TYPE_D8, 0x74, 0xD9,
    LCD_WRITE_TYPE_D8, 0x75, 0x00,
    LCD_WRITE_TYPE_D8, 0x76, 0x00,
    LCD_WRITE_TYPE_D8, 0x77, 0x00,
    LCD_WRITE_TYPE_D8, 0x78, 0x00,
    LCD_WRITE_TYPE_D8, 0x79, 0x00,
    LCD_WRITE_TYPE_D8, 0x7A, 0x00,
    LCD_WRITE_TYPE_D8, 0x7B, 0x00,
    LCD_WRITE_TYPE_D8, 0x80, 0x48,
    LCD_WRITE_TYPE_D8, 0x81, 0x00,
    LCD_WRITE_TYPE_D8, 0x82, 0x07,
    LCD_WRITE_TYPE_D8, 0x83, 0x02,
    LCD_WRITE_TYPE_D8, 0x84, 0xD7,
    LCD_WRITE_TYPE_D8, 0x85, 0x04,
    LCD_WRITE_TYPE_D8, 0x86, 0x00,
    LCD_WRITE_TYPE_D8, 0x87, 0x00,
    LCD_WRITE_TYPE_D8, 0x88, 0x48,
    LCD_WRITE_TYPE_D8, 0x89, 0x00,
    LCD_WRITE_TYPE_D8, 0x8A, 0x09,
    LCD_WRITE_TYPE_D8, 0x8B, 0x02,
    LCD_WRITE_TYPE_D8, 0x8C, 0xD9,
    LCD_WRITE_TYPE_D8, 0x8D, 0x04,
    LCD_WRITE_TYPE_D8, 0x8E, 0x00,
    LCD_WRITE_TYPE_D8, 0x8F, 0x00,
    LCD_WRITE_TYPE_D8, 0x90, 0x48,
    LCD_WRITE_TYPE_D8, 0x91, 0x00,
    LCD_WRITE_TYPE_D8, 0x92, 0x0B,
    LCD_WRITE_TYPE_D8, 0x93, 0x02,
    LCD_WRITE_TYPE_D8, 0x94, 0xDB,
    LCD_WRITE_TYPE_D8, 0x95, 0x04,
    LCD_WRITE_TYPE_D8, 0x96, 0x00,
    LCD_WRITE_TYPE_D8, 0x97, 0x00,
    LCD_WRITE_TYPE_D8, 0x98, 0x48,
    LCD_WRITE_TYPE_D8, 0x99, 0x00,
    LCD_WRITE_TYPE_D8, 0x9A, 0x0D,
    LCD_WRITE_TYPE_D8, 0x9B, 0x02,
    LCD_WRITE_TYPE_D8, 0x9C, 0xDD,
    LCD_WRITE_TYPE_D8, 0x9D, 0x04,
    LCD_WRITE_TYPE_D8, 0x9E, 0x00,
    LCD_WRITE_TYPE_D8, 0x9F, 0x00,
    LCD_WRITE_TYPE_D8, 0xA0, 0x48,
    LCD_WRITE_TYPE_D8, 0xA1, 0x00,
    LCD_WRITE_TYPE_D8, 0xA2, 0x06,
    LCD_WRITE_TYPE_D8, 0xA3, 0x02,
    LCD_WRITE_TYPE_D8, 0xA4, 0xD6,
    LCD_WRITE_TYPE_D8, 0xA5, 0x04,
    LCD_WRITE_TYPE_D8, 0xA6, 0x00,
    LCD_WRITE_TYPE_D8, 0xA7, 0x00,
    LCD_WRITE_TYPE_D8, 0xA8, 0x48,
    LCD_WRITE_TYPE_D8, 0xA9, 0x00,
    LCD_WRITE_TYPE_D8, 0xAA, 0x08,
    LCD_WRITE_TYPE_D8, 0xAB, 0x02,
    LCD_WRITE_TYPE_D8, 0xAC, 0xD8,
    LCD_WRITE_TYPE_D8, 0xAD, 0x04,
    LCD_WRITE_TYPE_D8, 0xAE, 0x00,
    LCD_WRITE_TYPE_D8, 0xAF, 0x00,
    LCD_WRITE_TYPE_D8, 0xB0, 0x48,
    LCD_WRITE_TYPE_D8, 0xB1, 0x00,
    LCD_WRITE_TYPE_D8, 0xB2, 0x0A,
    LCD_WRITE_TYPE_D8, 0xB3, 0x02,
    LCD_WRITE_TYPE_D8, 0xB4, 0xDA,
    LCD_WRITE_TYPE_D8, 0xB5, 0x04,
    LCD_WRITE_TYPE_D8, 0xB6, 0x00,
    LCD_WRITE_TYPE_D8, 0xB7, 0x00,
    LCD_WRITE_TYPE_D8, 0xB8, 0x48,
    LCD_WRITE_TYPE_D8, 0xB9, 0x00,
    LCD_WRITE_TYPE_D8, 0xBA, 0x0C,
    LCD_WRITE_TYPE_D8, 0xBB, 0x02,
    LCD_WRITE_TYPE_D8, 0xBC, 0xDC,
    LCD_WRITE_TYPE_D8, 0xBD, 0x04,
    LCD_WRITE_TYPE_D8, 0xBE, 0x00,
    LCD_WRITE_TYPE_D8, 0xBF, 0x00,
    LCD_WRITE_TYPE_D8, 0xC0, 0x10,
    LCD_WRITE_TYPE_D8, 0xC1, 0x47,
    LCD_WRITE_TYPE_D8, 0xC2, 0x56,
    LCD_WRITE_TYPE_D8, 0xC3, 0x65,
    LCD_WRITE_TYPE_D8, 0xC4, 0x74,
    LCD_WRITE_TYPE_D8, 0xC5, 0x88,
    LCD_WRITE_TYPE_D8, 0xC6, 0x99,
    LCD_WRITE_TYPE_D8, 0xC7, 0x01,
    LCD_WRITE_TYPE_D8, 0xC8, 0xBB,
    LCD_WRITE_TYPE_D8, 0xC9, 0xAA,
    LCD_WRITE_TYPE_D8, 0xD0, 0x10,
    LCD_WRITE_TYPE_D8, 0xD1, 0x47,
    LCD_WRITE_TYPE_D8, 0xD2, 0x56,
    LCD_WRITE_TYPE_D8, 0xD3, 0x65,
    LCD_WRITE_TYPE_D8, 0xD4, 0x74,
    LCD_WRITE_TYPE_D8, 0xD5, 0x88,
    LCD_WRITE_TYPE_D8, 0xD6, 0x99,
    LCD_WRITE_TYPE_D8, 0xD7, 0x01,
    LCD_WRITE_TYPE_D8, 0xD8, 0xBB,
    LCD_WRITE_TYPE_D8, 0xD9, 0xAA,
    LCD_WRITE_TYPE_D8, 0xF3, 0x01,
    LCD_WRITE_TYPE_D8, 0xF0, 0x00,

    LCD_WRITE_TYPE_D8, 0x3A, 0x55,

    LCD_WRITE_TYPE_END, 0xFF,
};
#endif

void LCD_exec_init(uint8_t* init_code)
{
    uint8_t* p = init_code;

    uint8_t data_len;
    uint8_t cmd;

    LCD_CS_CLR();
    while (1)
    {
        data_len = *p++;
        cmd      = *p++;

        if (cmd == 0xFF)
        {
            break;
        }

        LCD_WR_REG(cmd);

        for (uint8_t i = 0; i < data_len; i++)
        {
            LCD_WR_DATA(*p++);
        }
    }
    LCD_CS_SET();
}

void LCD_Init(void)
{

    SPI2_Init();
    SPI2_DMA_Init();
    LCD_GPIOInit();

    LCD_RESET();

    LCD_CS_CLR();
    LCD_WR_REG(0x01);
    LCD_CS_SET();
    Delay_Ms(120);

    LCD_CS_CLR();
    LCD_WR_REG(0x11);
    LCD_CS_SET();
    Delay_Ms(120);

    LCD_exec_init((uint8_t*)lcd_st7789_init_cmds);

    LCD_CS_CLR();
    LCD_WR_REG(0x29);
    LCD_CS_SET();
    Delay_Ms(120);

    // LCD_Clear(LCD_BLACK);

    Delay_Ms(10);
    LCD_LED_SET();
}

/*********************************************************************
 * @fn      LCD_SetWindows
 * 
 * @brief   Setting LCD display window
 * 
 * @param   xStar -  the bebinning x coordinate of the LCD display window
 *          yStar -  the bebinning y coordinate of the LCD display window
 *          xEnd -  the endning x coordinate of the LCD display window
 *          yEnd -  the endning y coordinate of the LCD display window
 * 
 * @return  none
 */
void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd)
{
    LCD_CS_CLR();
    LCD_WR_REG(LCD_SET_X);
    LCD_WR_DATA(xStar >> 8);
    LCD_WR_DATA(0x00FF & xStar);
    LCD_WR_DATA(xEnd >> 8);
    LCD_WR_DATA(0x00FF & xEnd);

    LCD_WR_REG(LCD_SET_Y);
    LCD_WR_DATA(yStar >> 8);
    LCD_WR_DATA(0x00FF & yStar);
    LCD_WR_DATA(yEnd >> 8);
    LCD_WR_DATA(0x00FF & yEnd);
    LCD_WR_REG(LCD_MEM_WRITE);

    LCD_CS_SET();
}

// 设置引脚为高电平
void LCD_LED_SET(void)
{
    LCD_LED_PORT->BSHR = LCD_LED_PIN;
}

void LCD_CS_SET(void)
{
    LCD_CS_PORT->BSHR = LCD_CS_PIN;
}

void LCD_DC_SET(void)
{
    LCD_DC_PORT->BSHR = LCD_DC_PIN;
}

void LCD_RST_SET(void)
{
    LCD_RST_PORT->BSHR = LCD_RST_PIN;
}

// 设置引脚为低电平
void LCD_LED_CLR(void)
{
    LCD_LED_PORT->BCR = LCD_LED_PIN;
}

void LCD_CS_CLR(void)
{
    LCD_CS_PORT->BCR = LCD_CS_PIN;
}

void LCD_DC_CLR(void)
{
    LCD_DC_PORT->BCR = LCD_DC_PIN;
}

void LCD_RST_CLR(void)
{
    LCD_RST_PORT->BCR = LCD_RST_PIN;
}

void SPI1_wait_for_bsy()
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET)
        ;
}

void SPI2_wait_for_bsy()
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET)
        ;
}

void SPI2_wait_for_txe()
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
        ;
}

void SPI1_wait_for_DMA()
{
    while (SPI1_DMA_TX_CH->CNTR > 0)
    {
    }
}

void SPI2_wait_for_DMA()
{
    while (SPI2_DMA_TX_CH->CNTR > 0)
    {
    }
}
 
void LCD_Clear(uint16_t color)
{
    LCD_Fill(0, 0, LCD_W, LCD_H, color);
}

 
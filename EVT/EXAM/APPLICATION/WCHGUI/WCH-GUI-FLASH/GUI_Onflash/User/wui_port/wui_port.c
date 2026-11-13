/********************************** (C) COPYRIGHT  *******************************
 * File Name          : wui_port.c
 * Author             : WCH
 * Version            : V1.2.0
 * Date               : 2026/08/15
 * Description        : Port layer functions
 *********************************************************************************
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "wui.h"
#include "wui_user/wui_page.h"
#include "wui_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "flash.h"
#include "images.h"
#include "ch32v205_qspi.h"
#include "touch.h"
#if WUI_INTERNAL_FLASH
#include "wui_internal_data.h"
#endif
static cst816d_touch_t touch;

/*********************************************************************
 * @fn      HW_Key_event
 * 
 * @brief   Hardware key event handler.
 * 
 * @param   _key_code - Key code of the pressed key.
 * 
 * @return  none
 */
void HW_Key_event(uint32_t _key_code)
{
    printf("[KEY] press %c\n", _key_code);
    wui_set_key_event(_key_code);
}

/*********************************************************************
 * @fn      touch_read
 * 
 * @brief   Read touch screen coordinates and pressed state.
 *          User should implement this function according to actual hardware.
 * 
 * @param   x - Pointer to store X coordinate.
 * @param   y - Pointer to store Y coordinate.
 * @param   pressed - Pointer to store pressed state (1: pressed, 0: released).
 * 
 * @return  none
 */
static void touch_read(uint16_t* x, uint16_t* y, uint8_t* pressed)
{
    // TODO: User should implement touch screen read logic
    // Example: Read coordinates and state from touch chip
    uint8_t cst816d_read(cst816d_touch_t * t);
    uint8_t press = cst816d_read(&touch);
    if (press)
    {
        *x = touch.y;
        *y = (360 - touch.x);
        *pressed = 1;

    }
    else
    {
        *x       = 0;
        *y       = 0;
        *pressed = 0;
    }
}

/*********************************************************************
 * @fn      flash_read
 * 
 * @brief   Read data from SPI Flash or Internal flash.
 *          User should implement this function according to actual hardware.
 * 
 * @param   outBuffer - Pointer to data buffer to store read data.
 * @param   ReadAddr - Start address to read from.
 * @param   size - Number of bytes to read.
 * 
 * @return  none
 */
static void flash_read(uint8_t* outBuffer, uint32_t ReadAddr, uint32_t size)
{
    // TODO: User should implement SPI Flash or Internal flash read logic
    #if WUI_INTERNAL_FLASH
    memcpy(outBuffer, wui_internal_data + ReadAddr, size);
    #else
    SPI_Flash_Read_dma_start(ReadAddr);
    SPI1_Read_DMA(outBuffer, size);
    SPI1_wait_for_bsy();
    SPI_Flash_Read_dma_end();
    #endif

}

/*********************************************************************
 * @fn      lcd_set_windows
 * 
 * @brief   Set LCD display window.
 *          User should implement this function according to actual hardware.
 * 
 * @param   x_start - Start X coordinate.
 * @param   y_start - Start Y coordinate.
 * @param   x_end - End X coordinate.
 * @param   y_end - End Y coordinate.
 * 
 * @return  none
 */
static void lcd_set_windows(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    // TODO: User should implement LCD window set logic
    LCD_SetWindows(x_start, y_start, x_end, y_end);
}

/*********************************************************************
 * @fn      lcd_dma_start
 * 
 * @brief   Start LCD DMA transfer.
 *          User should implement this function according to actual hardware.
 * 
 * @return  none
 */
static void lcd_dma_start(void)
{
    // TODO: User should implement LCD DMA start logic

    void          qspi_cs_clr();
    void          QSPI_sendCmd_raw(uint8_t cmd);

    qspi_cs_clr();
    QSPI_sendCmd_raw(0x2c);
}

/*********************************************************************
 * @fn      lcd_dma_send
 * 
 * @brief   Send data via LCD DMA.
 *          User should implement this function according to actual hardware.
 * 
 * @param   buf - Pointer to data buffer to send.
 * @param   len - Length of data in bytes.
 * 
 * @return  none
 */
static void lcd_dma_send(const uint8_t* buf, uint32_t len)
{
    // TODO: User should implement LCD DMA send logic
    void QSPI_WriteBuffer_raw(uint8_t* data, uint32_t len);

    QSPI_WriteBuffer_raw((uint8_t *)buf, len);
}

/*********************************************************************
 * @fn      lcd_dma_wait
 * 
 * @brief   Wait for LCD DMA transfer to complete.
 *          User should implement this function according to actual hardware.
 * 
 * @return  none
 */
static void lcd_dma_wait(void)
{
    // TODO: User should implement LCD DMA wait logic

    while (DMA2_Channel1->CNTR>0)
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
 * @fn      lcd_dma_end
 * 
 * @brief   End LCD DMA transfer.
 *          User should implement this function according to actual hardware.
 * 
 * @return  none
 */
static void lcd_dma_end(void)
{
    // TODO: User should implement LCD DMA end logic
    while (QSPI_GetFlagStatus(QSPI1, QSPI_FLAG_IDLE) == RESET)
    {
    }
    qspi_cs_set();
}

/*********************************************************************
 * @fn      wui_get_hal
 *
 * @brief   Get WUI HAL interface structure.
 *          User should register implemented HAL functions in this function.
 *
 * @return  Pointer to HAL interface structure.
 */
const wui_hal_t* wui_get_hal(void)
{
    static const wui_hal_t hal = {
        .touch_read      = touch_read,
        .flash_read      = flash_read,
        .lcd_set_windows = lcd_set_windows,
        .lcd_dma_start   = lcd_dma_start,
        .lcd_dma_send    = lcd_dma_send,
        .lcd_dma_wait    = lcd_dma_wait,
        .lcd_dma_end     = lcd_dma_end,
    };

    return &hal;
}
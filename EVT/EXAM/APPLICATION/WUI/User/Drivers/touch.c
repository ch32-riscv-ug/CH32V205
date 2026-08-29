/********************************** (C) COPYRIGHT  *******************************
* File Name          : touch.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2025/08/01
* Description        : This file provides some functions.
 *********************************************************************************
 * Copyright (c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "touch.h"

#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

#define SUCCESS        0
#define ERROR          1

#define Touch_INT_PORT GPIOB
#define Touch_INT_PIN  GPIO_Pin_4

#define Touch_RST_PORT GPIOB
#define Touch_RST_PIN  GPIO_Pin_5

static uint8_t CST816D_ADDR = 0x15;

void IIC_Scan(void)
{
    uint8_t devices[16];
    uint8_t count, i;

    count = Soft_IIC_ScanAddress(devices, 16);

    if (count > 16)
    {
        count = 16;
    }

    if (count == 0)
    {
        printf("no iic devices\n");
        return;
    }
    else
    {
        printf("iic devices %d\n", count);
        for (i = 0; i < count; i++)
        {
            printf("get iic device  %x\n", devices[i] << 1);
        }
    }
    CST816D_ADDR = devices[0] << 1;
}

/* 读取单个寄存器 */
static uint8_t cst816d_read_reg(uint8_t reg)
{
    uint8_t val;

    Soft_IIC_Start();
    Soft_IIC_WriteByte(CST816D_ADDR);        // 0x2A, 自动等待ACK
    Soft_IIC_WriteByte(reg);                 // 自动等待ACK
    Soft_IIC_Start();                        // 重新开始
    Soft_IIC_WriteByte((CST816D_ADDR) | 1);  // 0x2B, 自动等待ACK
    val = Soft_IIC_ReadByte(0);              // 0 = 发送NACK
    Soft_IIC_Stop();

    return val;
}

/* 连续读取多个寄存器 */
static void cst816d_read_regs(uint8_t reg, uint8_t* buf, uint8_t len)
{
    Soft_IIC_Start();
    Soft_IIC_WriteByte(CST816D_ADDR);        // 0x2A
    Soft_IIC_WriteByte(reg);                 // 寄存器地址
    Soft_IIC_Start();                        // 重新开始
    Soft_IIC_WriteByte((CST816D_ADDR) | 1);  // 0x2B

    for (uint8_t i = 0; i < len; i++)
    {
        if (i < len - 1)
        {
            buf[i] = Soft_IIC_ReadByte(1);  // 1 = 发送ACK
        }
        else
        {
            buf[i] = Soft_IIC_ReadByte(0);  // 0 = 发送NACK
        }
    }
    Soft_IIC_Stop();
}

/* 写单个寄存器 */
static void cst816d_write_reg(uint8_t reg, uint8_t val)
{
    Soft_IIC_Start();
    Soft_IIC_WriteByte(CST816D_ADDR);  // 0x2A
    Soft_IIC_WriteByte(reg);           // 寄存器地址
    Soft_IIC_WriteByte(val);           // 数据
    Soft_IIC_Stop();
}

// ============================================================
// 数据类型定义
// ============================================================

// ============================================================
// 初始化函数
// ============================================================

static void cst816d_init(void)
{

    /* 配置触摸芯片 */
    cst816d_write_reg(0xFE, 0x01); /* 禁用自动休眠 */
    cst816d_write_reg(0xFA, 0x20); /* 中断模式: 触摸/变化 */

    /* 读取ID验证 */
    uint8_t id = cst816d_read_reg(0xA7);
    printf("CST816D id=0x%02x\n", id);
}

// ============================================================
// 触摸数据读取函数
// ============================================================

uint8_t cst816d_read(cst816d_touch_t* t)
{
    uint8_t buf[7];

    cst816d_read_regs(0x02, buf, 7);

    t->gesture = buf[0];
    t->fingers = buf[0];  // 注意: 实际应该根据手册调整
    t->x       = ((uint16_t)(buf[1] & 0x0F) << 8) | buf[2];
    t->y       = ((uint16_t)(buf[3] & 0x0F) << 8) | buf[4];

    /* 限制坐标范围 */
    if (t->x > 359)
        t->x = 359;
    if (t->y > 359)
        t->y = 359;

    return t->gesture > 0 ? 1 : 0;
}

/*********************************************************************
 * @fn      Touch_Init
 *
 * @brief   Initialize the chip.
 *
 * @return  status - SUCCESS or ERROR.
 */
void Touch_Init(void)
{

    Soft_IIC_GPIO_Config();

    IIC_Scan();

    cst816d_init();
}

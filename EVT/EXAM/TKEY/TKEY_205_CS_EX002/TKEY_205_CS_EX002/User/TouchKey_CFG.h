/********************************** (C) COPYRIGHT *******************************
* File Name          : TouchKey_CFG.h
* Author             : WCH
* Version            : V1.0
* Date               : 2025/11/24
* Description        : Touch button parameter configuration header file
* ********************************************************************************
* Copyright(c) 2023 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention : This software(modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __TOUCH_KEY_CFG_H__
#define __TOUCH_KEY_CFG_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef enum _TKY_QUEUE_ID {
    TKY_QUEUE_0 = 0,
    TKY_QUEUE_1,
    TKY_QUEUE_2,
    TKY_QUEUE_3,
    TKY_QUEUE_4,
    TKY_QUEUE_5,
    TKY_QUEUE_6,
    TKY_QUEUE_7,
    TKY_QUEUE_8,
    TKY_QUEUE_9,
    TKY_QUEUE_10,
    TKY_QUEUE_11,
    TKY_QUEUE_12,
    TKY_QUEUE_13,
    TKY_QUEUE_14,
    TKY_QUEUE_15,
    /* Add new above this */
    TKY_QUEUE_END
} TKY_QUEUE_ID;

#define TKY_FILTER_MODE     			            4
#define TKY_FILTER_GRADE     			            2
#define TKY_BASE_REFRESH_ON_PRESS     			    0
#define TKY_BASE_UP_REFRESH_DOUBLE     			    0
#define TKY_BASE_DOWN_REFRESH_SLOW     			    0
#define TKY_BASE_REFRESH_SAMPLE_NUM     			100
#define TKY_SHIELD_EN     			                1
#define TKY_SINGLE_PRESS_MODE     			        2
#define TKY_MAX_QUEUE_NUM     			            8             


#define GEN_TKY_CH_INIT(qNum,chNum,chTime,disChTime,slpmode,chBaseline,trs,trs2) \
    {\
     .queueNum=qNum,.channelNum=chNum,\
     .chargeTime=chTime,\
     .disChargeTime=disChTime,\
     .sleepStatus=slpmode,\
     .baseLine = chBaseline,\
     .threshold=trs,\
     .threshold2=trs2\
    }

#define TKY_CHS_INIT                                                 \
        GEN_TKY_CH_INIT (TKY_QUEUE_0,  10,  3, 0x1ff, 0, 2621, 120, 110), \
        GEN_TKY_CH_INIT (TKY_QUEUE_1,  12,  3, 0x1ff, 0, 2600, 120, 110), \
        GEN_TKY_CH_INIT (TKY_QUEUE_2,  2,   3, 0x1ff, 0, 2360, 120, 110), \
        GEN_TKY_CH_INIT (TKY_QUEUE_3,  4,   3, 0x1ff, 0, 2233, 120, 110), \
        GEN_TKY_CH_INIT (TKY_QUEUE_4,  11,  3, 0x1ff, 0, 2604, 120, 110), \
        GEN_TKY_CH_INIT (TKY_QUEUE_5,  13,  3, 0x1ff, 0, 2604, 120, 110), \
        GEN_TKY_CH_INIT (TKY_QUEUE_6,  3,   3, 0x1ff, 0, 2604, 110, 100), \
        GEN_TKY_CH_INIT (TKY_QUEUE_7,  5,   3, 0x1ff, 0, 2604, 100, 90)


//***********************************************************
#ifdef __cplusplus
}
#endif

#endif /* __TOUCH_KEY_CFG_H__ */

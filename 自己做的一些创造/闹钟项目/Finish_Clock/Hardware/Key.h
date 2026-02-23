#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"
#include <stdint.h>

// 这里沿用标准类型定义
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef enum
{
    KEY_NONE = 0,

    KEY_NUM1,   // 数字键 1
    KEY_NUM2,
    KEY_NUM3,
    KEY_MENU,   // 菜单键

    KEY_NUM4,
    KEY_NUM5,
    KEY_NUM6,
    KEY_YES,   // 确认键

    KEY_NUM7,
    KEY_NUM8,
    KEY_NUM9,
    KEY_Exit,   // 退出键

    KEY_UP_LEFT,     // 上翻 / 左选
    KEY_NUM0,        // 数字键 0
    KEY_DOWN_RIGHT,  // 下翻 / 右选
    KEY_SETTING         // 设定模式键
} KeyCode;

// 管脚分配
#define KEY1_PORT            GPIOA  
#define KEY1_PIN             GPIO_Pin_15
#define KEY1_PORT_RCC        RCC_APB2Periph_GPIOA

#define KEY2_PORT            GPIOA  
#define KEY2_PIN             GPIO_Pin_14
#define KEY2_PORT_RCC        RCC_APB2Periph_GPIOA

#define KEY3_PORT            GPIOA  
#define KEY3_PIN             GPIO_Pin_13
#define KEY3_PORT_RCC        RCC_APB2Periph_GPIOA

#define KEY4_PORT            GPIOA  
#define KEY4_PIN             GPIO_Pin_12
#define KEY4_PORT_RCC        RCC_APB2Periph_GPIOA     

// 用位带方式来操作
#define KEY1     PAin(15)
#define KEY2     PAin(14)
#define KEY3     PAin(13)
#define KEY4     PAin(12)

// 定义每个按键的值  
#define KEY1_PRESS       1
#define KEY2_PRESS       2
#define KEY3_PRESS       3
#define KEY4_PRESS       4

// 矩阵按键用到的端口
#define KEY_MATRIX_H1_PORT           GPIOB
#define KEY_MATRIX_H1_PIN            GPIO_Pin_8
#define KEY_MATRIX_H1_PORT_RCC       RCC_APB2Periph_GPIOB

#define KEY_MATRIX_H2_PORT           GPIOB
#define KEY_MATRIX_H2_PIN            GPIO_Pin_9
#define KEY_MATRIX_H2_PORT_RCC       RCC_APB2Periph_GPIOB

#define KEY_MATRIX_H3_PORT           GPIOB
#define KEY_MATRIX_H3_PIN            GPIO_Pin_10
#define KEY_MATRIX_H3_PORT_RCC       RCC_APB2Periph_GPIOB

#define KEY_MATRIX_H4_PORT           GPIOB
#define KEY_MATRIX_H4_PIN            GPIO_Pin_11
#define KEY_MATRIX_H4_PORT_RCC       RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L1_PORT           GPIOB
#define KEY_MATRIX_L1_PIN            GPIO_Pin_12
#define KEY_MATRIX_L1_PORT_RCC       RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L2_PORT           GPIOB
#define KEY_MATRIX_L2_PIN            GPIO_Pin_13
#define KEY_MATRIX_L2_PORT_RCC       RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L3_PORT           GPIOB
#define KEY_MATRIX_L3_PIN            GPIO_Pin_14
#define KEY_MATRIX_L3_PORT_RCC       RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L4_PORT           GPIOB
#define KEY_MATRIX_L4_PIN            GPIO_Pin_15
#define KEY_MATRIX_L4_PORT_RCC       RCC_APB2Periph_GPIOB

// 函数声明
void KEY_Init(void);
u8 KEY_Scan(u8 mode);
void KEY_Matrix_Init(void);
KeyCode KEY_Matrix_Scan(void);

#endif /* __KEY_H */


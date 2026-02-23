#ifndef _BUZZER_H
#define _BUZZER_H

#include "system.h"

/* 蜂鸣器硬件引脚定义（PB0） */
#define BUZZER_PORT        GPIOB   
#define BUZZER_PIN         GPIO_Pin_0
#define BUZZER_PORT_RCC    RCC_APB2Periph_GPIOB

/* 位带操作控制蜂鸣器 (PB0) */
#define BUZZER   PBout(0)     // 1=响，0=关
void PC13LED_Init(void);

#define PC13_LED_ON()      GPIO_ResetBits(GPIOC, GPIO_Pin_13)   // 大部分最小系统板 PC13 低电平亮
#define PC13_LED_OFF()     GPIO_SetBits(GPIOC, GPIO_Pin_13)
#define PC13_LED_TOGGLE()  GPIOC->ODR ^= GPIO_Pin_13            // 翻转当前状态
/* 函数声明 */
void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Beep(uint16_t time_ms);
void Buzzer_N_Beep(uint8_t count);
void PC13_LED_Blink(uint8_t times);
#endif

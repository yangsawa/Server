#ifndef _BUZZER_H
#define _BUZZER_H

#include "system.h"

/* 蜂鸣器接在 PB0，这里先把引脚定义好 */
#define BUZZER_PORT        GPIOB   
#define BUZZER_PIN         GPIO_Pin_0
#define BUZZER_PORT_RCC    RCC_APB2Periph_GPIOB

/* 用位带方式直接控制 PB0 */
#define BUZZER   PBout(0)     // 1=响，0=关

/* 函数声明 */
void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Beep(uint16_t time_ms);
void Buzzer_N_Beep(uint8_t count);

#endif

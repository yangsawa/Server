#ifndef _SysTick_H
#define _SysTick_H

#include "system.h"



void SysTick_Init(u8 SYSCLK);
void Delay_ms(u16 nms);
void Delay_us(u32 nus);

// 这里沿用其他文件的小写命名，方便统一风格


#endif

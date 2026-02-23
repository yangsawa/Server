#ifndef _SysTick_H
#define _SysTick_H

#include "system.h"



void SysTick_Init(u8 SYSCLK);
void Delay_ms(u16 nms);
void Delay_us(u32 nus);

// 兼容其他文件用的小写写法


#endif

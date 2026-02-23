#ifndef _system_H
#define _system_H

#include "stm32f10x.h"

// 位带操作，用起来就像 51 上直接操控 GPIO 那样
// 想看底层原理可以翻《CM3 权威指南》第 87~92 页

// IO 位带相关的宏

/**
 * @brief 把某个地址的单个位换算成别名区地址
 * @param addr 外设或内存的原始地址
 * @param bitnum 想操作的位号(0-31)
 * @return 对应位在别名区里的地址
 */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 

/**
 * @brief 直接按地址取值
 * @param addr 要访问的地址
 * @return 该地址处的值
 */
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 

/**
 * @brief 访问位带别名区
 * @param addr 原始地址
 * @param bitnum 位编号
 * @return 别名区里对应位的值
 */
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

// IO口地址映射
// 输出数据寄存器(ODR)地址定义
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

// 输入数据寄存器(IDR)地址定义  
#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
// 以下这些宏只针对单个 IO 引脚
// n 只能取 0~15

// GPIOA端口操作宏
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  // GPIOA输出控制 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  // GPIOA输入读取

// GPIOB端口操作宏  
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  // GPIOB输出控制
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  // GPIOB输入读取

// GPIOC端口操作宏
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  // GPIOC输出控制
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  // GPIOC输入读取

// GPIOD端口操作宏
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  // GPIOD输出控制
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  // GPIOD输入读取

// GPIOE端口操作宏
#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  // GPIOE输出控制
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  // GPIOE输入读取

// GPIOF端口操作宏
#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  // GPIOF输出控制
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  // GPIOF输入读取

// GPIOG端口操作宏
#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  // GPIOG输出控制
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  // GPIOG输入读取

// 把常用的头文件都拉进来

// 系统相关
#include "stm32f10x.h"                  // Device header
#include "stdint.h"

// 软件流程相关
#include "system.h"
#include "Systick.h"
#include "usart.h"
#include "RTC.h"
#include "clock.h"
// 硬件驱动相关
#include "Buzzer.h"
#include "oled.h"
#include "LED.h"


// 系统函数声明

/**
 * @brief 系统上电后的初始化流程
 */
void SysInit(void);

/**
 * @brief 预留的系统功能入口
 */
void SysFunc(void);

/**
 * @brief 系统主循环
 */
void SysLoop(void);

#endif


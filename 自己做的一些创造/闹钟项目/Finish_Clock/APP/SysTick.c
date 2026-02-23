#include "SysTick.h"

static u8  fac_us=0;							// us 延时的倍数系数			   
static u16 fac_ms=0;							// ms 延时的倍数系数


// 初始化 SysTick，当延时用
// SysTick 的时钟固定为 AHB 时钟的 1/8
// SYSCLK: 当前系统主频
void SysTick_Init(u8 SYSCLK)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 
	fac_us=SYSCLK/8;					
	fac_ms=(u16)fac_us*1000;				   
}								    


/*******************************************************************************
* 函 数 名         : delay_us
* 函数功能		   : us 级别的延时
* 输    入         : nus：要延时的 us 数
					注意:nus 不要超过 798915us(最大值 2^24/fac_us@fac_us=21)
* 输    出         : 无
*******************************************************************************/		    								   
void Delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					// 装载计数值	  		 
	SysTick->VAL=0x00;        					// 清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	// 开始倒计时	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		// 计时结束后再往下走   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	// 停止计数
	SysTick->VAL =0X00;      					 // 再清一次计数器	 
}

/*******************************************************************************
* 函 数 名         : delay_ms
* 函数功能		   : ms 级别的延时
* 输    入         : nms：要延时的 ms 数
					注意:SysTick->LOAD 是 24 位寄存器，
					不要大于 0xffffff*8*1000/SYSCLK
					在 72M 条件下，nms<=1864ms 
* 输    出         : 无
*******************************************************************************/
void Delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				// 装载计数值(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							// 清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	// 开始倒计时  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		// 计时结束后再往下走   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	// 停止计数
	SysTick->VAL =0X00;       					// 再清一次计数器	  	    
} 

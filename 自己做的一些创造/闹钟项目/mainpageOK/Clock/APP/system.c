#include "system.h"
#include "clock.h"   
#include "RTC.h"

/***************************************************
 * 系统初始化
 ***************************************************/
void SysInit(void)
{
    SysTick_Init(72);       // 1ms 基准
    OLED_Init();            // OLED
    KEY_Matrix_Init();      // 4x4 矩阵按键
    Buzzer_Init();          // 蜂鸣器
    MyRTC_Init();           // RTC 初始化（从备份寄存器恢复时间）:contentReference[oaicite:0]{index=0}

    // 蜂鸣器响两下
    Buzzer_N_Beep(2);       // 已经在 Buzzer.c 里实现:contentReference[oaicite:1]{index=1}
		Delay_ms(200);
	  welcome_page();
    Delay_ms(1000);
		
       OLED_Clear();

}
void SysFunc(void){
}

void SysLoop(void)
{
    while (1)
    {
        main_page();      // 里面会读一次 RTC + 刷一屏
        
    }
}



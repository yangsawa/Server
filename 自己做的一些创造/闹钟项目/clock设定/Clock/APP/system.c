#include "system.h"
#include "clock.h"   
#include "RTC.h"
#include "Key.h"
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


}
void SysFunc(void){
}

void SysLoop(void)
{
    KeyCode key;

    //============== 1. 上电欢迎界面 ==============
    ui_state = UI_WELCOME;
    welcome_page();          // 显示欢迎
    Delay_ms(1500);          // 延时 1.5 秒
		OLED_Clear();
    //============== 2. 进入主界面 ==============
    ui_state = UI_MAIN;
    main_page();             // 先画一次主界面

    //============== 3. 主循环 ==============
    while (1)
    {
        // 扫描矩阵按键
        key = KEY_Matrix_Scan();

        // 如果有按键，就交给 UI_KeyHandler 处理
        if (key != KEY_NONE)
        {
            UI_KeyHandler(key);
        }

        // 主界面时，定期刷新时间
        if (ui_state == UI_MAIN)
        {
            main_page();
        }
				
        Delay_ms(50);   // 50ms 刷新一次就够了
    }
}


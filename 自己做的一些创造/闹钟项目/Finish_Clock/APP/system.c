#include "system.h"
#include "clock.h"   
#include "RTC.h"
#include "Key.h"

/***************************************************
 * 系统启动时先把外设都准备好
 ***************************************************/
void SysInit(void)
{
    SysTick_Init(72);
    OLED_Init();
    KEY_Matrix_Init();
    Buzzer_Init();
    LED_Init();
    MyRTC_Init();

    Buzzer_N_Beep(2);
    Delay_ms(200);
}
void SysFunc(void){
}

void SysLoop(void)
{
    KeyCode key;

    // 先展示欢迎页
    ui_state = UI_WELCOME;
    welcome_page();
    Delay_ms(1500);
    OLED_Clear();

    // 然后进入主界面
    ui_state = UI_MAIN;
    main_page();

    while (1)
    {
        Clock_BackgroundTick();

        key = KEY_Matrix_Scan();
        if (key != KEY_NONE)
        {
            UI_KeyHandler(key);
        }

        if (ui_state == UI_MAIN)
        {
            main_page();
        }
        else if (ui_state == UI_TIMER_RUN)
        {
            timer_run_page();
        }

        Delay_ms(50);
    }
}

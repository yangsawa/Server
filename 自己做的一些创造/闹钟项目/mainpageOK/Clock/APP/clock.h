#ifndef __CLOCK_H
#define __CLOCK_H

#include "system.h"   // 里面有 u8、u32 等类型定义
#include "oled.h"     // 用到 OLED_ShowChar / OLED_ShowString 等
typedef enum
{
    UI_WELCOME = 0,   // 开机欢迎界面
    UI_MAIN,          // 主界面（时间、CNT 等，后面再写）
		UI_MENU,
    // 以后再加：UI_MENU, UI_CLOCK_SET, UI_ALARM_LIST, ...
} UI_State;

extern UI_State ui_state;
// clock.h
void welcome_page(void);
void main_page(void);
void menu_page(void);
#endif

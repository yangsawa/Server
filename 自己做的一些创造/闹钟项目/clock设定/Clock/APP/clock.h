#ifndef __CLOCK_H
#define __CLOCK_H

#include "system.h"
#include "oled.h"
#include "Key.h"      // ? 要用 KeyCode / KEY_MENU 等

typedef enum
{
    UI_WELCOME = 0,   // 开机欢迎界面
    UI_MAIN,          // 主界面：显示当前时间日期
    UI_MENU,          // 主菜单：Clock / Time / Timer

    UI_CLOCK_LIST,    // Clock 列表：Clock1 / Clock2 / NewClock
    UI_CLOCK_OPTS,    // Clock 操作：Open / Close / Change / NewClk

    // 后面可以扩展：
    UI_TIME_SET,      // 校时界面（调小时、分钟）
    UI_TIMER_SET,     // 倒计时设置
    UI_TIMER_RUN      // 倒计时运行
} UI_State;
typedef struct
{
    u8 hour;
    u8 min;
    u8 sec;
    u8 enable;   // 1=开, 0=关
} Alarm_t;

extern Alarm_t g_alarms[3];      // 0:Clock1 1:Clock2 2:NewClock

extern UI_State ui_state;
extern u8 g_menu_index;   // 0: Clock, 1: Time, 2: Timer
// 新增：
extern u8 g_clock_index;     // Clock 列表索引：0:Clock1 1:Clock2 2:NewClock
extern u8 g_clock_opt_index; // Clock 操作索引：0:Open 1:Close 2:Change 3:NewClk

void welcome_page(void);
void main_page(void);
void menu_page(void);


// 新增：按键处理函数
void UI_KeyHandler(KeyCode key);
void clock_list_page(void);
void clock_opts_page(void);

#endif

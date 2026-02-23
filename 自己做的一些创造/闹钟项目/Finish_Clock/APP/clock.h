#ifndef __CLOCK_H
#define __CLOCK_H

#include "system.h"
#include "oled.h"
#include "Key.h"

typedef enum
{
    UI_WELCOME = 0,   // Welcome splash
    UI_MAIN,          // Main clock page: show current time/date
    UI_MENU,          // Main menu: Clock / Time / Timer

    UI_CLOCK_LIST,    // Clock list: Clock1 / Clock2 / NewClock
    UI_CLOCK_OPTS,    // Clock operations: Open / Close / Change / NewClk
    UI_CLOCK_SET,     // Clock time editing

    // Other features
    UI_TIME_SET,      // RTC adjust page (YYYY-MM-DD HH:MM:SS)
    UI_TIMER_SET,     // Countdown setup
    UI_TIMER_RUN      // Countdown running
} UI_State;

typedef struct
{
    u8 hour;
    u8 min;
    u8 sec;
    u8 enable;   // 1 = on, 0 = off
} Alarm_t;

extern Alarm_t g_alarms[3];      // 0:Clock1 1:Clock2 2:NewClock

extern UI_State ui_state;
extern u8 g_menu_index;      // 0: Clock, 1: Time, 2: Timer
extern u8 g_clock_index;     // Clock list index: 0:Clock1 1:Clock2 2:NewClock

void welcome_page(void);
void main_page(void);
void menu_page(void);

void UI_KeyHandler(KeyCode key);
void Clock_BackgroundTick(void);
void clock_list_page(void);
void clock_opts_page(void);
void time_set_page(void);
void clock_set_page(void);
void timer_set_page(void);
void timer_run_page(void);
#endif

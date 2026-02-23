#include "clock.h"
#include "system.h"
#include "oled.h"
#include "RTC.h"
#include "Key.h"
#include "Buzzer.h"
#include "LED.h"
#include <stdio.h>
#include <string.h>

// 当前界面状态
UI_State ui_state = UI_WELCOME;
u8 g_menu_index  = 0;
u8 g_clock_index = 0;

// 闹钟列表：Clock1、Clock2，再加一个新建项
Alarm_t g_alarms[3] =
{
    {7,  0, 0, 1},
    {8, 30, 0, 0},
    {0,  0, 0, 0}
};

// 校时用到的缓冲区
static char g_date_str[11];      // "YYYY-MM-DD"
static char g_hms_str[9];        // "HH:MM:SS"
static u8 g_time_cursor_k = 0;   // 0~13 的位置索引
static const u8 s_date_digit_pos[8] = {0,1,2,3,5,6,8,9};
static const u8 s_time_digit_pos[6] = {0,1,3,4,6,7};

// 编辑闹钟时用的游标与缓存
static char g_alarm_str[9];       // "HH:MM:SS"
static u8  g_alarm_cursor = 0;    // 0~5 依次指向各位数字
static u8  g_alarm_edit_index = 0;// 0/1/2 -> Clock1/2/New
static const u8 s_alarm_digit_pos[6] = {0,1,3,4,6,7};

// 倒计时相关的状态
static char g_timer_str[9];       // "HH:MM:SS"
static u8  g_timer_cursor = 0;
static const u8 s_timer_digit_pos[6] = {0,1,3,4,6,7};
static u32 g_timer_total_sec = 0;
static u32 g_timer_remain_sec = 0;
static u8  g_timer_running = 0;
static u8  g_timer_set_reset = 1;
static u8  g_timer_page_reset = 1;

// 交互状态
static u8 g_edit_mode = 0;          // 0=锁定，1=可编辑
static u8 g_clock_opts_cursor = 0;
static u8 g_save_hint = 0;          // 保存提示倒计时（秒）

// 响铃状态
static u8 g_alarm_ringing = 0;
static u8 g_buzzer_on = 0;
static u16 g_buzzer_tick = 0;       // 50ms tick
static u8  g_main_page_reset = 1;
static u8  g_ring_from_timer = 0;   // 1 表示倒计时触发的响铃

static u8 IsSettingPage(void)
{
    return (ui_state == UI_TIME_SET ||
            ui_state == UI_CLOCK_SET ||
            ui_state == UI_TIMER_SET);
}


// ------------ 一些工具方法 ------------
static void TimeSet_GetCursorPos(u8 *line, u8 *pos)
{
    if (g_time_cursor_k < 8)
    {
        *line = 0;
        *pos  = s_date_digit_pos[g_time_cursor_k];
    }
    else
    {
        *line = 1;
        *pos  = s_time_digit_pos[g_time_cursor_k - 8];
    }
}
static void TimeSet_MoveNextDigit(void) { g_time_cursor_k = (g_time_cursor_k >= 13) ? 0 : g_time_cursor_k + 1; }
static void TimeSet_MovePrevDigit(void) { g_time_cursor_k = (g_time_cursor_k == 0) ? 13 : g_time_cursor_k - 1; }

static void TimeSet_InitFromRTC(void)
{
    MyRTC_ReadTime();
    sprintf(g_date_str, "%04d-%02d-%02d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    sprintf(g_hms_str,  "%02d:%02d:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
    g_time_cursor_k = 0;
}
static void TimeSet_ApplyToRTC(void)
{
    u16 year = (g_date_str[0]-'0')*1000 + (g_date_str[1]-'0')*100 + (g_date_str[2]-'0')*10 + (g_date_str[3]-'0');
    u16 mon  = (g_date_str[5]-'0')*10 + (g_date_str[6]-'0');
    u16 day  = (g_date_str[8]-'0')*10 + (g_date_str[9]-'0');
    u16 hour = (g_hms_str[0]-'0')*10 + (g_hms_str[1]-'0');
    u16 min  = (g_hms_str[3]-'0')*10 + (g_hms_str[4]-'0');
    u16 sec  = (g_hms_str[6]-'0')*10 + (g_hms_str[7]-'0');
    if (hour > 23) hour = 23;
    if (mon  > 12) mon  = 12;
    if (day  > 31) day  = 31;
    if (min  > 59) min  = 59;
    if (sec  > 59) sec  = 59;
    MyRTC_Adjust(year, mon, day, hour, min, sec);
    g_save_hint = 3;
}

static void ClockSet_InitFromAlarm(u8 index)
{
    Alarm_t *p = &g_alarms[index];
    sprintf(g_alarm_str, "%02d:%02d:%02d", p->hour, p->min, p->sec);
    g_alarm_cursor = 0;
    g_alarm_edit_index = index;
}
static void ClockSet_GetCursorPos(u8 *pos) { *pos = s_alarm_digit_pos[g_alarm_cursor]; }
static void ClockSet_MoveNext(void) { g_alarm_cursor = (g_alarm_cursor >= 5) ? 0 : g_alarm_cursor + 1; }
static void ClockSet_MovePrev(void) { g_alarm_cursor = (g_alarm_cursor == 0) ? 5 : g_alarm_cursor - 1; }
static void ClockSet_ApplyToAlarm(void)
{
    u8 hour = (g_alarm_str[0]-'0')*10 + (g_alarm_str[1]-'0');
    u8 min  = (g_alarm_str[3]-'0')*10 + (g_alarm_str[4]-'0');
    u8 sec  = (g_alarm_str[6]-'0')*10 + (g_alarm_str[7]-'0');
    if (hour > 23) hour = 23;
    if (min  > 59) min  = 59;
    if (sec  > 59) sec  = 59;
    g_alarms[g_alarm_edit_index].hour   = hour;
    g_alarms[g_alarm_edit_index].min    = min;
    g_alarms[g_alarm_edit_index].sec    = sec;
    g_alarms[g_alarm_edit_index].enable = 1;
    g_save_hint = 3;
}

static void TimerSet_FromRemain(void)
{
    u8 h = (g_timer_remain_sec / 3600) % 24;
    u8 m = (g_timer_remain_sec / 60) % 60;
    u8 s = g_timer_remain_sec % 60;
    sprintf(g_timer_str, "%02d:%02d:%02d", h, m, s);
    g_timer_cursor = 0;
}
static void TimerSet_GetCursorPos(u8 *pos) { *pos = s_timer_digit_pos[g_timer_cursor]; }
static void TimerSet_MoveNext(void) { g_timer_cursor = (g_timer_cursor >= 5) ? 0 : g_timer_cursor + 1; }
static void TimerSet_MovePrev(void) { g_timer_cursor = (g_timer_cursor == 0) ? 5 : g_timer_cursor - 1; }
static void TimerSet_ApplyFromStr(void)
{
    u32 h = (g_timer_str[0]-'0')*10 + (g_timer_str[1]-'0');
    u32 m = (g_timer_str[3]-'0')*10 + (g_timer_str[4]-'0');
    u32 s = (g_timer_str[6]-'0')*10 + (g_timer_str[7]-'0');
    if (h > 23) h = 23;
    if (m > 59) m = 59;
    if (s > 59) s = 59;
    g_timer_total_sec  = h*3600 + m*60 + s;
    g_timer_remain_sec = g_timer_total_sec;
    g_save_hint = 3;
}

// 居中显示相关的小工具
static u8 OLED_GetCenterColumn(const char *str)
{
    u8 len = 0; const char *p = str;
    while (*p >= ' ' && *p <= '~') { len++; p++; }
    if (len >= 16) return 1;
    return (16 - len) / 2 + 1;
}
static u8 OLED_GetCenterX(const char *str, u8 size)
{
    u8 len = 0; const char *p = str;
    while (*p >= ' ' && *p <= '~') { len++; p++; }
    u8 char_w = size / 2;
    u16 total_w = len * char_w;
    if (total_w >= 128) return 0;
    return (128 - total_w) / 2;
}
static void OLED_ShowString_Mode(u8 x, u8 y, const char *p, u8 size, u8 mode)
{
    while (*p >= ' ' && *p <= '~')
    {
        if (x > (128 - (size / 2))) { x = 0; y += size; }
        if (y > (64 - size)) { y = x = 0; OLED_Clear(); }
        OLED_ShowChar(x, y, *p, size, mode);
        x += size / 2; p++;
    }
}
static void Menu_DrawItem(u8 line, const char *text, u8 selected)
{
    u8 col = OLED_GetCenterColumn(text);
    u8 x = (col - 1) * 8;
    u8 y = (line - 1) * 16;
    if (selected)
    {
        OLED_Fill(0, y, 127, y + 15, 1);
        OLED_ShowString_Mode(x, y, text, 16, 0);
    }
    else
    {
        OLED_ShowString_Mode(x, y, text, 16, 1);
    }
}
static void Highlight_RC(u8 line, u8 col_start, const char *str, u8 pos)
{
    if (!str) return;
    u8 len = 0;
    while (str[len] >= ' ' && str[len] <= '~') len++;
    if (pos >= len) return;
    OLED_ShowString_RC(line, col_start, str);
    u8 x = (col_start - 1) * 8 + pos * 8;
    u8 y = (line - 1) * 16;
    OLED_ShowChar(x, y, str[pos], 16, 0);
}

// 把按键转换成数字键，其他按键返回 0xFF
static u8 KeyToDigit(KeyCode key)
{
    switch (key)
    {
        case KEY_NUM0: return 0;
        case KEY_NUM1: return 1;
        case KEY_NUM2: return 2;
        case KEY_NUM3: return 3;
        case KEY_NUM4: return 4;
        case KEY_NUM5: return 5;
        case KEY_NUM6: return 6;
        case KEY_NUM7: return 7;
        case KEY_NUM8: return 8;
        case KEY_NUM9: return 9;
        default: return 0xFF;
    }
}

// 响铃/倒计时
static void Alarm_StopRing(void)
{
    g_alarm_ringing = 0;
    g_buzzer_on = 0;
    Buzzer_Off();
    LED0 = 0;
}
static void Alarm_StartRing(u8 idx)
{
    g_ring_from_timer = (idx == 255);
    g_alarm_ringing = 1;
    g_buzzer_tick = 0;
    Buzzer_On();
    g_buzzer_on = 1;
    LED0 = 1;
}
static void Alarm_RingTick(void)
{
    if (!g_alarm_ringing) return;
    LED0 = 1;
    g_buzzer_tick++;
    if (g_buzzer_tick >= 2) // 大约每 100ms 翻转一次蜂鸣器和 LED
    {
        g_buzzer_tick = 0;
        if (g_buzzer_on) { Buzzer_Off(); g_buzzer_on = 0; LED0 = 0; }
        else             { Buzzer_On();  g_buzzer_on = 1; LED0 = 1; }
    }
}
static void Timer_SecondTick(void)
{
    if (g_timer_running && g_timer_remain_sec > 0)
    {
        g_timer_remain_sec--;
        if (g_timer_remain_sec == 0)
        {
            g_timer_running = 0;
            Alarm_StartRing(255);
        }
    }
}

void Clock_BackgroundTick(void)
{
    static u8 last_sec = 0xFF;
    Alarm_RingTick();
    MyRTC_ReadTime();
    if (MyRTC_Time[5] != last_sec)
    {
        last_sec = MyRTC_Time[5];

        if (!g_alarm_ringing)
        {
            if (IsSettingPage()) LED0 = 1; // 设置页常亮
            else                 LED0 = !LED0; // 其他页面按秒闪烁
        }

        for (u8 i = 0; i < 3; i++)
        {
            if (!g_alarms[i].enable) continue;
            if (MyRTC_Time[3] == g_alarms[i].hour &&
                MyRTC_Time[4] == g_alarms[i].min  &&
                MyRTC_Time[5] == g_alarms[i].sec)
            {
                Alarm_StartRing(i);
            }
        }
        Timer_SecondTick();
        if (g_save_hint > 0) g_save_hint--;
    }
}

// ---------- 各个界面 ----------
void welcome_page(void)
{
    OLED_Clear();
    u8 col = OLED_GetCenterColumn("WELCOME");
    OLED_ShowString_RC(2, col, "WELCOME");
    col = OLED_GetCenterColumn("CLOCK");
    OLED_ShowString_RC(3, col, "CLOCK");
    OLED_Refresh_Gram();
}

void main_page(void)
{
    static u8 first_drawn = 0;
    static char last_time[9] = {0};
    static char last_date[11] = {0};
    char buf[20];
    u8 col;
    u8 changed = 0;
    MyRTC_ReadTime();

    if (!first_drawn || g_main_page_reset)
    {
        OLED_Clear();
        col = OLED_GetCenterColumn("BeijingRealTime");
        OLED_ShowString_RC(1, col, "BeijingRealTime");
        first_drawn = 1;
        g_main_page_reset = 0;
        memset(last_time, 0, sizeof(last_time));
        memset(last_date, 0, sizeof(last_date));
        changed = 1;
    }

    sprintf(buf, "%02d:%02d:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
    if (strcmp(buf, last_time) != 0)
    {
        u8 x = OLED_GetCenterX(buf, 24);
        OLED_Fill(0, 16, 127, 31, 0);
        OLED_ShowString(x, 16, (u8*)buf, 24);
        strcpy(last_time, buf);
        changed = 1;
    }

    sprintf(buf, "%04d-%02d-%02d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    if (strcmp(buf, last_date) != 0)
    {
        col = OLED_GetCenterColumn(buf);
        OLED_Fill(0, 48, 127, 63, 0);
        OLED_ShowString_RC(4, col, buf);
        strcpy(last_date, buf);
        changed = 1;
    }

    if (changed) OLED_Refresh_Gram();
}

void menu_page(void)
{
    OLED_Clear();
    u8 col = OLED_GetCenterColumn("Menu");
    OLED_ShowString_RC(1, col, "Menu");
    const char *items[3] = {"Clock","Time","Timer"};
    for (u8 i = 0; i < 3; i++) Menu_DrawItem(2 + i, items[i], i == g_menu_index);
    OLED_Refresh_Gram();
}

void clock_list_page(void)
{
    OLED_Clear();
    u8 col = OLED_GetCenterColumn("Clock");
    OLED_ShowString_RC(1, col, "Clock");
    char buf[20];
    for (u8 i = 0; i < 3; i++)
    {
        u8 line = 2 + i;
        if (i < 2)
            sprintf(buf, "Clock%d %02d:%02d %s", i+1, g_alarms[i].hour, g_alarms[i].min, g_alarms[i].enable ? "Open" : "Close");
        else
            sprintf(buf, "New %02d:%02d %s", g_alarms[2].hour, g_alarms[2].min, g_alarms[2].enable ? "Open" : "Close");
        Menu_DrawItem(line, buf, i == g_clock_index);
    }
    OLED_Refresh_Gram();
}

void clock_opts_page(void)
{
    OLED_Clear();
    char buf[20];
    u8 col;
    const char *title = (g_clock_index == 0) ? "Clock1" : (g_clock_index == 1) ? "Clock2" : "NewClock";
    col = OLED_GetCenterColumn(title);
    OLED_ShowString_RC(1, col, title);

    sprintf(buf, "%02d:%02d:%02d", g_alarms[g_clock_index].hour, g_alarms[g_clock_index].min, g_alarms[g_clock_index].sec);
    col = OLED_GetCenterColumn(buf);
    OLED_ShowString_RC(2, col, buf);

    const char *status = g_alarms[g_clock_index].enable ? "Open" : "Close";
    col = OLED_GetCenterColumn(status);
    OLED_ShowString_RC(3, col, status);

    if (g_save_hint > 0) OLED_ShowString_RC(4, 1, "Saved");
    else                 OLED_ShowString_RC(4, 1, "YES toggle SET edit");

    OLED_Refresh_Gram();
}

void clock_set_page(void)
{
    OLED_Clear();
    char buf[20];
    u8 col;
    sprintf(buf, "Set C%d", g_alarm_edit_index + 1);
    col = OLED_GetCenterColumn(buf);
    OLED_ShowString_RC(1, col, buf);

    u8 pos;
    ClockSet_GetCursorPos(&pos);
    u8 base_col = OLED_GetCenterColumn(g_alarm_str);
    Highlight_RC(2, base_col, g_alarm_str, pos);

    const char *status = g_alarms[g_alarm_edit_index].enable ? "Open" : "Close";
    col = OLED_GetCenterColumn(status);
    OLED_ShowString_RC(3, col, status);

    if (g_save_hint > 0) OLED_ShowString_RC(4, 1, "Saved");
    else                 OLED_ShowString_RC(4, 1, g_edit_mode ? "EDIT" : "LOCK");

    OLED_Refresh_Gram();
}

void timer_set_page(void)
{
    static u8 first_drawn = 0;
    if (g_timer_set_reset)
    {
        g_timer_set_reset = 0;
        first_drawn = 0;
    }

    if (!first_drawn)
    {
        OLED_Clear();
        u8 col = OLED_GetCenterColumn("Set Timer");
        OLED_ShowString_RC(1, col, "Set Timer");
        first_drawn = 1;
    }

    u8 pos;
    TimerSet_GetCursorPos(&pos);
    u8 base_col = OLED_GetCenterColumn(g_timer_str);
    OLED_Fill(0, 16, 127, 31, 0);
    Highlight_RC(2, base_col, g_timer_str, pos);

    OLED_Fill(0, 32, 127, 47, 0);
    if (g_save_hint > 0) OLED_ShowString_RC(3, 1, "Saved");
    else                 OLED_ShowString_RC(3, 1, g_edit_mode ? "EDIT" : "LOCK");
    OLED_Refresh_Gram();
}

void time_set_page(void)
{
    OLED_Clear();
    u8 col = OLED_GetCenterColumn("Set Time");
    OLED_ShowString_RC(1, col, "Set Time");
    u8 line, pos;
    TimeSet_GetCursorPos(&line, &pos);
    if (g_edit_mode && line == 0)
    {
        u8 base_col = OLED_GetCenterColumn(g_date_str);
        Highlight_RC(2, base_col, g_date_str, pos);
        col = OLED_GetCenterColumn(g_hms_str);
        OLED_ShowString_RC(3, col, g_hms_str);
    }
    else if (g_edit_mode)
    {
        col = OLED_GetCenterColumn(g_date_str);
        OLED_ShowString_RC(2, col, g_date_str);
        u8 base_col = OLED_GetCenterColumn(g_hms_str);
        Highlight_RC(3, base_col, g_hms_str, pos);
    }
    else
    {
        col = OLED_GetCenterColumn(g_date_str);
        OLED_ShowString_RC(2, col, g_date_str);
        col = OLED_GetCenterColumn(g_hms_str);
        OLED_ShowString_RC(3, col, g_hms_str);
    }
    if (g_save_hint > 0) OLED_ShowString_RC(4, 1, "Saved");
    else                 OLED_ShowString_RC(4, 1, g_edit_mode ? "EDIT" : "LOCK");
    OLED_Refresh_Gram();
}

void timer_run_page(void)
{
    static u32 last_sec = 0xFFFFFFFF;
    static u8  last_run = 0xFF;
    static u8  first_drawn = 0;
    u8 changed = 0;
    if (g_timer_page_reset)
    {
        g_timer_page_reset = 0;
        first_drawn = 0;
        last_sec = 0xFFFFFFFF;
        last_run = 0xFF;
    }

    if (!first_drawn)
    {
        OLED_Clear();
        u8 col = OLED_GetCenterColumn("Timer");
        OLED_ShowString_RC(1, col, "Timer");
        OLED_ShowString_RC(4, 1, "YES/Set pause");
        first_drawn = 1;
        changed = 1;
    }

    u32 h = (g_timer_remain_sec / 3600) % 24;
    u32 m = (g_timer_remain_sec / 60) % 60;
    u32 s = g_timer_remain_sec % 60;
    char buf[20];
    sprintf(buf, "%02u:%02u:%02u", (unsigned)h, (unsigned)m, (unsigned)s);

    if (g_timer_remain_sec != last_sec)
    {
        u8 col = OLED_GetCenterColumn(buf);
        OLED_Fill(0, 16, 127, 31, 0);
        OLED_ShowString_RC(2, col, buf);
        last_sec = g_timer_remain_sec;
        changed = 1;
    }

    if (g_timer_running != last_run)
    {
        const char *status = g_timer_running ? "Running" : "Paused";
        u8 col = OLED_GetCenterColumn(status);
        OLED_Fill(0, 32, 127, 47, 0);
        OLED_ShowString_RC(3, col, status);
        last_run = g_timer_running;
        changed = 1;
    }

    if (changed) OLED_Refresh_Gram();
}

// ---------- 按键事件 ----------
void UI_KeyHandler(KeyCode key)
{
    if (g_alarm_ringing && key != KEY_NONE)
    {
        Alarm_StopRing();
        if (g_ring_from_timer)
        {
            g_timer_running = 0;
            g_timer_remain_sec = g_timer_total_sec;
            TimerSet_FromRemain();
            g_timer_set_reset = 1;
            g_timer_page_reset = 1;
            g_edit_mode = 1;          // 切回可编辑的设定页
            ui_state = UI_TIMER_SET;
            timer_set_page();
            g_ring_from_timer = 0;
            return;
        }
        g_ring_from_timer = 0;
    }

    switch (ui_state)
    {
        case UI_MAIN:
            if (key == KEY_MENU) { ui_state = UI_MENU; g_menu_index = 0; menu_page(); }
            break;

        case UI_MENU:
            if (key == KEY_UP_LEFT)       { g_menu_index = (g_menu_index == 0) ? 2 : g_menu_index - 1; menu_page(); }
            else if (key == KEY_DOWN_RIGHT) { g_menu_index = (g_menu_index == 2) ? 0 : g_menu_index + 1; menu_page(); }
            else if (key == KEY_MENU || key == KEY_YES)
            {
                if (g_menu_index == 0) { ui_state = UI_CLOCK_LIST; g_clock_index = 0; g_clock_opts_cursor = 0; clock_list_page(); }
                else if (g_menu_index == 1) { ui_state = UI_TIME_SET; TimeSet_InitFromRTC(); g_edit_mode = 0; time_set_page(); }
                else if (g_menu_index == 2) { ui_state = UI_TIMER_SET; if (g_timer_total_sec == 0) { g_timer_total_sec = 60; g_timer_remain_sec = 60; } TimerSet_FromRemain(); g_edit_mode = 1; g_timer_set_reset = 1; timer_set_page(); }
            }
            else if (key == KEY_Exit) { ui_state = UI_MAIN; g_main_page_reset = 1; main_page(); }
            break;

        case UI_CLOCK_LIST:
            if (key == KEY_UP_LEFT)       { g_clock_index = (g_clock_index == 0) ? 2 : g_clock_index - 1; clock_list_page(); }
            else if (key == KEY_DOWN_RIGHT) { g_clock_index = (g_clock_index == 2) ? 0 : g_clock_index + 1; clock_list_page(); }
            else if (key == KEY_MENU || key == KEY_YES) { ui_state = UI_CLOCK_OPTS; g_clock_opts_cursor = 0; clock_opts_page(); }
            else if (key == KEY_Exit) { ui_state = UI_MENU; menu_page(); }
            break;

        case UI_CLOCK_OPTS:
        {
            Alarm_t *p = &g_alarms[g_clock_index];
            if (key == KEY_YES)
            {
                p->enable = !p->enable;
                g_save_hint = 3;
                clock_opts_page();
            }
            else if (key == KEY_SETTING)
            {
                ui_state = UI_CLOCK_SET;
                ClockSet_InitFromAlarm(g_clock_index);
                g_edit_mode = 1;
                clock_set_page();
            }
            else if (key == KEY_Exit || key == KEY_MENU)
            {
                ui_state = UI_CLOCK_LIST;
                clock_list_page();
            }
        }
        break;

        case UI_CLOCK_SET:
            if (key == KEY_SETTING || key == KEY_YES)
            {
                ClockSet_ApplyToAlarm();
                g_edit_mode = 0;
                ui_state = UI_CLOCK_OPTS;
                clock_opts_page();
            }
            else if (g_edit_mode && key == KEY_UP_LEFT)      { ClockSet_MovePrev(); clock_set_page(); }
            else if (g_edit_mode && key == KEY_DOWN_RIGHT)   { ClockSet_MoveNext(); clock_set_page(); }
            else if (g_edit_mode)
            {
                u8 digit = KeyToDigit(key);
                if (digit <= 9)
                {
                    u8 pos; ClockSet_GetCursorPos(&pos);
                    g_alarm_str[pos] = '0' + digit;
                    ClockSet_MoveNext();
                    clock_set_page();
                }
            }
            else if (key == KEY_Exit)
            {
                g_edit_mode = 0;
                ui_state = UI_CLOCK_OPTS;
                clock_opts_page();
            }
            break;

        case UI_TIME_SET:
            if (key == KEY_SETTING)
            {
                g_edit_mode = 1;
                time_set_page();
            }
            else if (g_edit_mode && key == KEY_YES)
            {
                TimeSet_ApplyToRTC();
                g_edit_mode = 0;
                ui_state = UI_MAIN;
                g_main_page_reset = 1;
                main_page();
            }
            else if (g_edit_mode && key == KEY_UP_LEFT)      { TimeSet_MovePrevDigit(); time_set_page(); }
            else if (g_edit_mode && key == KEY_DOWN_RIGHT)   { TimeSet_MoveNextDigit(); time_set_page(); }
            else if (g_edit_mode)
            {
                u8 digit = KeyToDigit(key);
                if (digit <= 9)
                {
                    u8 line, pos; TimeSet_GetCursorPos(&line, &pos);
                    char ch = '0' + digit;
                    if (line == 0) g_date_str[pos] = ch; else g_hms_str[pos] = ch;
                    TimeSet_MoveNextDigit(); time_set_page();
                }
            }
            else if (key == KEY_Exit)
            {
                g_edit_mode = 0;
                ui_state = UI_MENU;
                g_menu_index = 1; // 回到菜单高亮 Time
                menu_page();
            }
            break;

        case UI_TIMER_SET:
            if (key == KEY_SETTING)
            {
                TimerSet_ApplyFromStr();
                g_edit_mode = 0;
                ui_state = UI_TIMER_RUN;
                g_timer_running = (g_timer_remain_sec > 0);
                g_timer_page_reset = 1;
                timer_run_page();
            }
            else if (g_edit_mode && key == KEY_UP_LEFT)      { TimerSet_MovePrev(); timer_set_page(); }
            else if (g_edit_mode && key == KEY_DOWN_RIGHT)   { TimerSet_MoveNext(); timer_set_page(); }
            else if (g_edit_mode)
            {
                u8 digit = KeyToDigit(key);
                if (digit <= 9)
                {
                    u8 pos; TimerSet_GetCursorPos(&pos);
                    g_timer_str[pos] = '0' + digit;
                    TimerSet_MoveNext();
                    timer_set_page();
                }
            }
            else if (key == KEY_Exit)
            {
                g_edit_mode = 0;
                ui_state = UI_MENU;
                g_menu_index = 2; // 回到菜单高亮 Timer
                menu_page();
            }
            break;

        case UI_TIMER_RUN:
            if (key == KEY_YES || key == KEY_SETTING)
            {
                g_timer_running = !g_timer_running;
                timer_run_page();
            }
            else if (key == KEY_UP_LEFT)
            {
                g_timer_running = 0;
                g_timer_remain_sec = g_timer_total_sec;
                TimerSet_FromRemain();
                g_timer_set_reset = 1;
                g_timer_page_reset = 1;
                g_edit_mode = 1;
                ui_state = UI_TIMER_SET;
                timer_set_page();
            }
            else if (key == KEY_Exit)
            {
                g_timer_running = 0;
                ui_state = UI_MENU;
                g_menu_index = 2; // 回到菜单高亮 Timer
                menu_page();
            }
            break;

        default:
            break;
    }
}













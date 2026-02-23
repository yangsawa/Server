#include "clock.h"
#include "system.h"
#include "oled.h"
#include "RTC.h"
#include "Key.h"
// UI 状态
UI_State ui_state = UI_WELCOME;

u8 g_menu_index      = 0;
u8 g_clock_index     = 0;
u8 g_clock_opt_index = 0;

// ===== 多闹钟数据 =====
Alarm_t g_alarms[3] =
{
    {7,  0,  0, 1},   // Clock1  默认 07:00:00 开
    {8, 30,  0, 0},   // Clock2  默认 08:30:00 关
    {0,  0,  0, 0}    // NewClock 默认 00:00:00 关
};

/**
 * 计算字符串在 16 列字符中的居中列号（适合 RC 行列接口）
 */
static u8 OLED_GetCenterColumn(const char *str)
{
    u8 len = 0;
    const char *p = str;

    while (*p >= ' ' && *p <= '~')
    {
        len++;
        p++;
    }

    if (len >= 16) return 1;
    return (16 - len) / 2 + 1;
}

/**
 * 计算字符串在 128 像素宽度下，指定字体 size（12/16/24）时的居中 X 坐标
 */
static u8 OLED_GetCenterX(const char *str, u8 size)
{
    u8 len = 0;
    const char *p = str;

    while (*p >= ' ' && *p <= '~')
    {
        len++;
        p++;
    }

    u8  char_w  = size / 2;      // 你的 OLED_ShowString 是按 size/2 当宽度的
    u16 total_w = len * char_w;

    if (total_w >= 128) return 0;
    return (128 - total_w) / 2;
}
// 带模式的字符串显示：mode = 1 正常, mode = 0 阴刻/反色
static void OLED_ShowString_Mode(u8 x, u8 y, const char *p, u8 size, u8 mode)
{
    while ((*p <= '~') && (*p >= ' ')) // 判断是不是合法 ASCII
    {
        if (x > (128 - (size / 2))) { x = 0; y += size; }
        if (y > (64 - size)) { y = x = 0; OLED_Clear(); }

        OLED_ShowChar(x, y, *p, size, mode);
        x += size / 2;
        p++;
    }
}
// 画一行菜单项：line = 2~4，对应菜单三行；selected=1 时高亮阴刻，0 时普通
static void Menu_DrawItem(u8 line, const char *text, u8 selected)
{
    u8 col = OLED_GetCenterColumn(text);
    u8 x   = (col - 1) * 8;       // 列 → 像素X
    u8 y   = (line - 1) * 16;     // 行 → 像素Y（16 点阵）

    if (selected)
    {
        // 1. 把这一整行填成“亮条”（光条），占 16 像素高度
        OLED_Fill(0, y, 127, y + 15, 1);

        // 2. 在亮条上用 阴刻（反色）方式画字
        OLED_ShowString_Mode(x, y, text, 16, 0);  // mode = 0 阴刻
    }
    else
    {
        // 普通行：直接正常画字
        OLED_ShowString_Mode(x, y, text, 16, 1);  // mode = 1 正常
    }
}

/**
 * @brief 欢迎界面：居中显示两行
 *        第2行: WELCOME
 *        第3行: CLOCK
 *        （先用英文调试，中文可以后面再用汉字点阵替换）
 */
void welcome_page(void)
{
    OLED_Clear();

    // --- Line 2: "WELCOME" ---
    // 居中计算
    u8 col = OLED_GetCenterColumn("WELCOME");
    OLED_ShowString_RC(2, col, "WELCOME");

    // --- Line 3: "CLOCK" ---
    col = OLED_GetCenterColumn("CLOCK");
    OLED_ShowString_RC(3, col, "CLOCK");

    OLED_Refresh_Gram();
}


/**
 * @brief 主界面（先写一个占位，后面再慢慢完善）
 */
void main_page(void)
{
    char buf[20];
    u8 col;

    MyRTC_ReadTime();

    //--------------------------------------
    // 第 1 行：标题
    //--------------------------------------
    col = OLED_GetCenterColumn("BeijingRealTime");
    OLED_ShowString_RC(1, col, "BeijingRealTime");

    //--------------------------------------
    // ★ 大号时间（24点阵，占两行高度）
    //   从 y=16 开始画 → 覆盖 [16 ~ 39]
    //--------------------------------------
// ★ 大号时间（24点阵，占两行高度）
sprintf(buf, "%02d:%02d:%02d",
        MyRTC_Time[3],
        MyRTC_Time[4],
        MyRTC_Time[5]);

// 用像素居中：24 点阵，每个字符宽 size/2 = 12 像素
u8 x = OLED_GetCenterX(buf, 24);
OLED_ShowString(x, 16, (u8*)buf, 24);


    //--------------------------------------
    // 第 4 行：日期
    // 日期用普通 16 点阵即可
    //--------------------------------------
    sprintf(buf, "%04d-%02d-%02d",
            MyRTC_Time[0],
            MyRTC_Time[1],
            MyRTC_Time[2]);

    col = OLED_GetCenterColumn(buf);
    OLED_ShowString_RC(4, col, buf);

    OLED_Refresh_Gram();
}

void menu_page(void)
{
    OLED_Clear();

    // 标题
    u8 col = OLED_GetCenterColumn("Menu");
    OLED_ShowString_RC(1, col, "Menu");

    // 三个菜单项文本
    const char *items[3] = {
        "Clock",   // 0
        "Time",    // 1
        "Timer"    // 2
    };

    // 逐行画第 2~4 行菜单
    for (u8 i = 0; i < 3; i++)
    {
        u8 line = 2 + i;
        u8 selected = (i == g_menu_index) ? 1 : 0;
        Menu_DrawItem(line, items[i], selected);
    }

    OLED_Refresh_Gram();
}


void UI_KeyHandler(KeyCode key)
{
    switch (ui_state)
    {
        /************* 主界面：显示时间 *************/
        case UI_MAIN:
            if (key == KEY_MENU)        // 按 MENU 进入主菜单
            {
                ui_state = UI_MENU;
                g_menu_index = 0;
                menu_page();
            }
            break;

        /************* 主菜单：Clock / Time / Timer *************/
        case UI_MENU:
            if (key == KEY_UP_LEFT)         // 上
            {
                if (g_menu_index == 0)
                    g_menu_index = 2;
                else
                    g_menu_index--;

                menu_page();
            }
            else if (key == KEY_DOWN_RIGHT) // 下
            {
                if (g_menu_index == 2)
                    g_menu_index = 0;
                else
                    g_menu_index++;

                menu_page();
            }
            else if (key == KEY_MENU || key == KEY_YES)   // 确认进入子菜单
            {
                if (g_menu_index == 0)      // Clock
                {
                    ui_state = UI_CLOCK_LIST;
                    g_clock_index = 0;
                    clock_list_page();
                }
                else if (g_menu_index == 1) // Time（校时，后面实现）
                {
                    ui_state = UI_TIME_SET;
                    // TODO: 进入校时界面函数，比如 time_set_page();
                }
                else if (g_menu_index == 2) // Timer（倒计时，后面实现）
                {
                    ui_state = UI_TIMER_SET;
                    // TODO: 进入倒计时设置界面
                }
            }
            else if (key == KEY_Exit)       // 退出主菜单，回主界面
            {
                ui_state = UI_MAIN;
                main_page();
            }
            break;

        /************* Clock 列表：Clock1 / Clock2 / NewClock *************/
        case UI_CLOCK_LIST:
            if (key == KEY_UP_LEFT)
            {
                if (g_clock_index == 0)
                    g_clock_index = 2;
                else
                    g_clock_index--;

                clock_list_page();
            }
            else if (key == KEY_DOWN_RIGHT)
            {
                if (g_clock_index == 2)
                    g_clock_index = 0;
                else
                    g_clock_index++;

                clock_list_page();
            }
            else if (key == KEY_MENU || key == KEY_YES)   // 进入操作页
            {
                ui_state = UI_CLOCK_OPTS;
                g_clock_opt_index = 0;
                clock_opts_page();
            }
            else if (key == KEY_Exit)  // 返回主菜单
            {
                ui_state = UI_MENU;
                menu_page();
            }
            break;

        /************* Clock 操作：Open / Close / Change / NewClk *************/
           /************* Clock 操作：Clock1/2 或 NewClock *************/
    case UI_CLOCK_OPTS:

        // ---------- NewClock 的按键逻辑 ----------
        if (g_clock_index == 2)
        {
            if (key == KEY_UP_LEFT || key == KEY_DOWN_RIGHT || key == KEY_MENU || key == KEY_YES)
            {
                // 任意确认/上下键：切换开关状态
                g_alarms[2].enable = !g_alarms[2].enable;
                clock_opts_page();      // 重新显示，开/关 会变化
            }
            else if (key == KEY_Exit)
            {
                ui_state = UI_CLOCK_LIST;
                clock_list_page();
            }
            break;
        }

        // ---------- Clock1 / Clock2 的通用按键逻辑 ----------
        if (key == KEY_UP_LEFT)
        {
            if (g_clock_opt_index == 0)
                g_clock_opt_index = 2;      // 0 上翻到 2
            else
                g_clock_opt_index--;

            clock_opts_page();
        }
        else if (key == KEY_DOWN_RIGHT)
        {
            if (g_clock_opt_index == 2)
                g_clock_opt_index = 0;      // 2 下翻到 0
            else
                g_clock_opt_index++;

            clock_opts_page();
        }
        else if (key == KEY_MENU || key == KEY_YES)   // 确认操作
        {
            Alarm_t *pAlm = &g_alarms[g_clock_index];

            switch (g_clock_opt_index)
            {
                case 0: // Open
                    pAlm->enable = 1;
                    break;

                case 1: // Close
                    pAlm->enable = 0;
                    break;

                case 2: // Change
                    // TODO: 进入闹钟时间设置界面（后面实现）
                    // 例如：
                    //   ui_state = UI_TIME_SET;
                    //   g_edit_index = g_clock_index;  // 要编辑哪一个 Clock
                    break;
            }

            // 操作完重新显示当前页面
            clock_opts_page();
        }
        else if (key == KEY_Exit)   // 返回 Clock 列表
        {
            ui_state = UI_CLOCK_LIST;
            clock_list_page();
        }
        break;


        /************* 预留：Time / Timer 其它状态 *************/
        case UI_TIME_SET:
            // TODO: 之后实现校时界面的按键处理
            break;

        case UI_TIMER_SET:
        case UI_TIMER_RUN:
            // TODO: 之后实现倒计时相关按键处理
            break;

        default:
            break;
    }
}

void clock_list_page(void)
{
    OLED_Clear();

    // 标题：Clock
    u8 col = OLED_GetCenterColumn("Clock");
    OLED_ShowString_RC(1, col, "Clock");

    char buf[20];

    for (u8 i = 0; i < 3; i++)
    {
        u8 line = 2 + i;            // 第 2~4 行
        u8 selected = (i == g_clock_index);

        // 生成显示字符串
        if (i < 2)  // Clock1 / Clock2
        {
            // 例如: "C1 07:00 On"
            sprintf(buf, "C%d %02d:%02d %s",
                    i + 1,
                    g_alarms[i].hour,
                    g_alarms[i].min,
                    g_alarms[i].enable ? "On" : "Off");
        }
        else        // NewClock
        {
            // 例如: "New 00:00:00"
            sprintf(buf, "New %02d:%02d:%02d",
                    g_alarms[2].hour,
                    g_alarms[2].min,
                    g_alarms[2].sec);
        }

        Menu_DrawItem(line, buf, selected);
    }

    OLED_Refresh_Gram();
}

void clock_opts_page(void)
{
    OLED_Clear();
    char buf[20];
    u8  col;

    // ========== NewClock 的特殊页面 ==========
    if (g_clock_index == 2)
    {
        // 标题：NewClock
        col = OLED_GetCenterColumn("NewClock");
        OLED_ShowString_RC(1, col, "NewClock");

        // 第 2 行：时间（默认 00:00:00）
        sprintf(buf, "%02d:%02d:%02d",
                g_alarms[2].hour,
                g_alarms[2].min,
                g_alarms[2].sec);
        col = OLED_GetCenterColumn(buf);
        OLED_ShowString_RC(2, col, buf);

        // 第 3 行：开 / 关 （先用 ON/OFF，之后可以换成中文）
        const char *status = g_alarms[2].enable ? "Open" : "Close";
        col = OLED_GetCenterColumn(status);
        OLED_ShowString_RC(3, col, status);

        OLED_Refresh_Gram();
        return;    // 注意：NewClock 不再往下画通用菜单
    }

    // ========== Clock1 / Clock2 的通用页面 ==========
    const char *title = (g_clock_index == 0) ? "Clock1" : "Clock2";
    col = OLED_GetCenterColumn(title);
    OLED_ShowString_RC(1, col, title);

    // 在标题下面直接显示闹钟时间
    sprintf(buf, "%02d:%02d:%02d",
            g_alarms[g_clock_index].hour,
            g_alarms[g_clock_index].min,
            g_alarms[g_clock_index].sec);
    col = OLED_GetCenterColumn(buf);
    OLED_ShowString_RC(2, col, buf);

    // 三个操作项：Open / Close / Change，显示在第 3/4 行
    const char *items[3] = {
        "Open",
        "Close",
        "Change"
    };

    for (u8 i = 0; i < 3; i++)
    {
        u8 line = 3 + i;                // 第 3,4,5 行；如果你只有 4 行，可以改成只要前两项
        if (line > 4) break;            // 防止越界屏幕

        u8 selected = (i == g_clock_opt_index);
        Menu_DrawItem(line, items[i], selected);
    }

    OLED_Refresh_Gram();
}


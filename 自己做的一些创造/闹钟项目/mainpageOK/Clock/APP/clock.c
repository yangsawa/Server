#include "clock.h"
#include "system.h"
#include "oled.h"
#include "RTC.h"

// UI 状态
UI_State ui_state = UI_WELCOME;

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

void menu_page(void){
}


//硬件基础部分

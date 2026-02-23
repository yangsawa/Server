#include "system.h"
#include "stdio.h"
#include "HX711.h"
#include "key.h"
#include "Buzzer.h"

#define FILTER_ALPHA       0.5f     /* low-pass filter coefficient */
#define THRESH_STEP_G      10.0f    /* threshold step per key press */
#define RESET_GAP_G        10.0f    /* hysteresis gap for alarm reset */
#define ZERO_CAL_SAMPLES   40       /* boot tare samples */
#define FILTER_PRIME_NUM   20       /* samples to prime filter at boot */

static uint8_t g_overload_muted  = 0;   /* 1 = alarm is muted/acknowledged */
static uint8_t g_alarm_enabled   = 1;   /* 0 = alarm disabled */
static float   g_alarm_threshold = 340.0f;
static float   g_weight_filtered = 0.0f;
static uint8_t g_filter_inited   = 0;

/* threshold edit buffer via digit keys */
static uint16_t g_thresh_edit    = 0;
static uint8_t  g_edit_active    = 0;
static uint8_t  g_edit_digits    = 0;

static float alarm_reset_level(void)
{
    float v = g_alarm_threshold - RESET_GAP_G;
    return (v < 0.0f) ? 0.0f : v;
}

/* boot zero calibration: tare with multiple samples, then prime filter by averaging */
static void Boot_Zero_Calibrate(void)
{
    HX711_Tare(ZERO_CAL_SAMPLES);

    float sum = 0.0f;
    for (uint8_t i = 0; i < FILTER_PRIME_NUM; i++)
    {
        sum += HX711_GetWeight(5);
        Delay_ms(10);
    }
    g_weight_filtered = sum / (float)FILTER_PRIME_NUM;
    g_filter_inited   = 1;
}

static void reset_edit_buffer(void)
{
    g_thresh_edit = 0;
    g_edit_digits = 0;
    g_edit_active = 0;
}

static void apply_edit_buffer(void)
{
    if (g_edit_active && g_edit_digits > 0)
    {
        g_alarm_threshold = (float)g_thresh_edit;
    }
    reset_edit_buffer();
}

static void handle_key(KeyCode key)
{
    /* any key press will stop current alarm */
    g_overload_muted = 1;
    Buzzer_Off();
    PC13_LED_OFF();

    switch (key)
    {
        case KEY_NUM0: case KEY_NUM1: case KEY_NUM2: case KEY_NUM3:
        case KEY_NUM4: case KEY_NUM5: case KEY_NUM6: case KEY_NUM7:
        case KEY_NUM8: case KEY_NUM9:
        {
            uint8_t digit = 0;
            if      (key == KEY_NUM1) digit = 1;
            else if (key == KEY_NUM2) digit = 2;
            else if (key == KEY_NUM3) digit = 3;
            else if (key == KEY_NUM4) digit = 4;
            else if (key == KEY_NUM5) digit = 5;
            else if (key == KEY_NUM6) digit = 6;
            else if (key == KEY_NUM7) digit = 7;
            else if (key == KEY_NUM8) digit = 8;
            else if (key == KEY_NUM9) digit = 9;
            /* accumulate up to 4 digits */
            if (!g_edit_active)
            {
                g_thresh_edit = 0;
                g_edit_digits = 0;
                g_edit_active = 1;
            }
            if (g_edit_digits < 4)
            {
                g_thresh_edit = g_thresh_edit * 10 + digit;
                g_edit_digits++;
            }
            break;
        }
        case KEY_EDIT_START:
            reset_edit_buffer();
            g_edit_active = 1;
            break;
        case KEY_EDIT_OK:
            apply_edit_buffer();
            break;
        case KEY_THRESH_UP:
            apply_edit_buffer();
            g_alarm_threshold += THRESH_STEP_G;
            break;
        case KEY_THRESH_DOWN:
            apply_edit_buffer();
            if (g_alarm_threshold > THRESH_STEP_G)
                g_alarm_threshold -= THRESH_STEP_G;
            break;
        case KEY_ALARM_TOGGLE:
            if (g_edit_active && g_edit_digits > 0) apply_edit_buffer();
            else g_alarm_enabled = !g_alarm_enabled;
            break;
        case KEY_ALARM_MUTE:
        default:
            apply_edit_buffer(); /* commit edit on any other key */
            break;
    }
}

void Overload_AlarmTask(float weight)
{
    float reset_th = alarm_reset_level();

    if (!g_alarm_enabled)
    {
        Buzzer_Off();
        PC13_LED_OFF();
        g_overload_muted = 1;
        return;
    }

    if (weight > g_alarm_threshold)
    {
        if (!g_overload_muted)
        {
            PC13_LED_ON();      /* active low LED */
            Buzzer_On();
            Delay_ms(20);       /* shorter pulse to reduce blocking */
            Buzzer_Off();
            PC13_LED_OFF();
        }
        else
        {
            Buzzer_Off();
            PC13_LED_OFF();
        }
    }
    else
    {
        Buzzer_Off();
        PC13_LED_OFF();
        if (weight < reset_th)
        {
            g_overload_muted = 0;   /* allow next alarm after falling below reset threshold */
        }
    }
}

void SysInit(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    HX711_GPIO_Init();
    SysTick_Init(72);
    OLED_Init();
    Buzzer_Init();
    KEY_Matrix_Init();
    PC13LED_Init();
    Buzzer_N_Beep(2);
    Delay_ms(200);

    /* preload calibration (factory) */
    HX711_SetCalibration(-11140, 673.47f);
    /* zero calibration on boot (empty load) with more samples */
    Boot_Zero_Calibrate();
    PC13_LED_Blink(3);
    OLED_Clear();
    OLED_ShowString_RC(2, 1, (u8 *)"W:      g");
    OLED_ShowString_RC(3, 1, (u8 *)"T:     g");
    OLED_Refresh_Gram();
}

void SysFunc(void) {}

void SysLoop(void)
{
    float w_raw;
    char num_buf[12];
    char thr_buf[12];
    char label[4] = "T:";

    while (1)
    {
        KeyCode key = KEY_Matrix_Scan();
        if (key != KEY_NONE)
        {
            handle_key(key);
        }

        /* read weight */
        w_raw = HX711_GetWeight(10);

        /* simple low-pass filter */
        if (!g_filter_inited)
        {
            g_weight_filtered = w_raw;
            g_filter_inited   = 1;
        }
        else
        {
            g_weight_filtered = FILTER_ALPHA * w_raw + (1.0f - FILTER_ALPHA) * g_weight_filtered;
        }

        /* display weight */
        sprintf(num_buf, "%6.1f", g_weight_filtered);
        OLED_ShowString_RC(2, 3, (u8 *)num_buf);

        /* display threshold */
        if (g_edit_active)
        {
            label[0] = 'E'; label[1] = ':';
            sprintf(thr_buf, "%5u", g_thresh_edit);
        }
        else
        {
            label[0] = 'T'; label[1] = ':';
            sprintf(thr_buf, "%5.0f", g_alarm_threshold);
        }
        OLED_ShowString_RC(3, 1, (u8 *)label);
        OLED_ShowString_RC(3, 3, (u8 *)thr_buf);

        OLED_Refresh_Gram();

        /* alarm handling */
        Overload_AlarmTask(g_weight_filtered);

        Delay_ms(50);
    }
}

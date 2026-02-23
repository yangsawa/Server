#include "key.h"
#include "SysTick.h"
#include "system.h"

/* 4x4 matrix keypad scanner: PB8~PB11 drive rows, PB12~PB15 read columns */
static uint8_t read_cols(void)
{
    uint8_t v1 = GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT, KEY_MATRIX_L1_PIN);
    uint8_t v2 = GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT, KEY_MATRIX_L2_PIN);
    uint8_t v3 = GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT, KEY_MATRIX_L3_PIN);
    uint8_t v4 = GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT, KEY_MATRIX_L4_PIN);
    return (v1 << 0) | (v2 << 1) | (v3 << 2) | (v4 << 3);
}

static void set_rows(uint8_t h1, uint8_t h2, uint8_t h3, uint8_t h4)
{
    if (h1) GPIO_SetBits(KEY_MATRIX_H1_PORT, KEY_MATRIX_H1_PIN); else GPIO_ResetBits(KEY_MATRIX_H1_PORT, KEY_MATRIX_H1_PIN);
    if (h2) GPIO_SetBits(KEY_MATRIX_H2_PORT, KEY_MATRIX_H2_PIN); else GPIO_ResetBits(KEY_MATRIX_H2_PORT, KEY_MATRIX_H2_PIN);
    if (h3) GPIO_SetBits(KEY_MATRIX_H3_PORT, KEY_MATRIX_H3_PIN); else GPIO_ResetBits(KEY_MATRIX_H3_PORT, KEY_MATRIX_H3_PIN);
    if (h4) GPIO_SetBits(KEY_MATRIX_H4_PORT, KEY_MATRIX_H4_PIN); else GPIO_ResetBits(KEY_MATRIX_H4_PORT, KEY_MATRIX_H4_PIN);
}

void KEY_Matrix_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(KEY_MATRIX_H1_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_H2_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_H3_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_H4_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L1_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L2_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L3_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L4_PORT_RCC, ENABLE);

    /* rows output push-pull */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_H1_PIN;
    GPIO_Init(KEY_MATRIX_H1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_H2_PIN;
    GPIO_Init(KEY_MATRIX_H2_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_H3_PIN;
    GPIO_Init(KEY_MATRIX_H3_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_H4_PIN;
    GPIO_Init(KEY_MATRIX_H4_PORT, &GPIO_InitStructure);

    /* columns input pull-down */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L1_PIN;
    GPIO_Init(KEY_MATRIX_L1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L2_PIN;
    GPIO_Init(KEY_MATRIX_L2_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L3_PIN;
    GPIO_Init(KEY_MATRIX_L3_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L4_PIN;
    GPIO_Init(KEY_MATRIX_L4_PORT, &GPIO_InitStructure);

    set_rows(0, 0, 0, 0);
}

/* Scan once and return one key; non-blocking except a short debounce */
KeyCode KEY_Matrix_Scan(void)
{
    uint8_t cols;

    /* pre-check: set all rows high to detect any press */
    set_rows(1, 1, 1, 1);
    cols = read_cols();
    if (cols == 0)
    {
        return KEY_NONE;
    }

    Delay_ms(3);    /* faster debounce */
    cols = read_cols();
    if (cols == 0)
    {
        return KEY_NONE;
    }

    /* row 1: 1 2 3 CALIBRATE */
    set_rows(1, 0, 0, 0);
    cols = read_cols();
    if (cols)
    {
        KeyCode k = KEY_NONE;
        if (cols & 0x01) k = KEY_NUM1;
        else if (cols & 0x02) k = KEY_NUM2;
        else if (cols & 0x04) k = KEY_NUM3;
        else if (cols & 0x08) k = KEY_EDIT_START;
        Delay_ms(1);             /* brief settle */
        return k;
    }

    /* row 2: 4 5 6 TARE */
    set_rows(0, 1, 0, 0);
    cols = read_cols();
    if (cols)
    {
        KeyCode k = KEY_NONE;
        if (cols & 0x01) k = KEY_NUM4;
        else if (cols & 0x02) k = KEY_NUM5;
        else if (cols & 0x04) k = KEY_NUM6;
        else if (cols & 0x08) k = KEY_EDIT_OK;
        Delay_ms(1);
        return k;
    }

    /* row 3: 7 8 9 THRESH_UP */
    set_rows(0, 0, 1, 0);
    cols = read_cols();
    if (cols)
    {
        KeyCode k = KEY_NONE;
        if (cols & 0x01) k = KEY_NUM7;
        else if (cols & 0x02) k = KEY_NUM8;
        else if (cols & 0x04) k = KEY_NUM9;
        else if (cols & 0x08) k = KEY_THRESH_UP;
        Delay_ms(1);
        return k;
    }

    /* row 4: ALARM_MUTE, 0, ALARM_TOGGLE, THRESH_DOWN */
    set_rows(0, 0, 0, 1);
    cols = read_cols();
    if (cols)
    {
        KeyCode k = KEY_NONE;
        if (cols & 0x01) k = KEY_ALARM_MUTE;
        else if (cols & 0x02) k = KEY_NUM0;
        else if (cols & 0x04) k = KEY_ALARM_TOGGLE;
        else if (cols & 0x08) k = KEY_THRESH_DOWN;
        Delay_ms(1);
        return k;
    }

    return KEY_NONE;
}

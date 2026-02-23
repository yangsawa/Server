#ifndef _key_H
#define _key_H

#include "system.h"

/* Matrix keypad pins: PB8~PB11 rows (H1~H4), PB12~PB15 columns (L1~L4) */
#define KEY_MATRIX_H1_PORT            GPIOB
#define KEY_MATRIX_H1_PIN             GPIO_Pin_8
#define KEY_MATRIX_H1_PORT_RCC        RCC_APB2Periph_GPIOB

#define KEY_MATRIX_H2_PORT            GPIOB
#define KEY_MATRIX_H2_PIN             GPIO_Pin_9
#define KEY_MATRIX_H2_PORT_RCC        RCC_APB2Periph_GPIOB

#define KEY_MATRIX_H3_PORT            GPIOB
#define KEY_MATRIX_H3_PIN             GPIO_Pin_10
#define KEY_MATRIX_H3_PORT_RCC        RCC_APB2Periph_GPIOB

#define KEY_MATRIX_H4_PORT            GPIOB
#define KEY_MATRIX_H4_PIN             GPIO_Pin_11
#define KEY_MATRIX_H4_PORT_RCC        RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L1_PORT            GPIOB
#define KEY_MATRIX_L1_PIN             GPIO_Pin_12
#define KEY_MATRIX_L1_PORT_RCC        RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L2_PORT            GPIOB
#define KEY_MATRIX_L2_PIN             GPIO_Pin_13
#define KEY_MATRIX_L2_PORT_RCC        RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L3_PORT            GPIOB
#define KEY_MATRIX_L3_PIN             GPIO_Pin_14
#define KEY_MATRIX_L3_PORT_RCC        RCC_APB2Periph_GPIOB

#define KEY_MATRIX_L4_PORT            GPIOB
#define KEY_MATRIX_L4_PIN             GPIO_Pin_15
#define KEY_MATRIX_L4_PORT_RCC        RCC_APB2Periph_GPIOB

/* Key codes: digits keep their positions; six custom keys on col4 and row4-col1/col3 */
typedef enum
{
    KEY_NONE = 0,

    KEY_NUM1,
    KEY_NUM2,
    KEY_NUM3,
    KEY_EDIT_START,    /* row1 col4: 开始/清除阈值输入 */

    KEY_NUM4,
    KEY_NUM5,
    KEY_NUM6,
    KEY_EDIT_OK,       /* row2 col4: 确认阈值输入 */

    KEY_NUM7,
    KEY_NUM8,
    KEY_NUM9,
    KEY_THRESH_UP,     /* row3 col4 */

    KEY_ALARM_MUTE,    /* row4 col1 */
    KEY_NUM0,
    KEY_ALARM_TOGGLE,  /* row4 col3 */
    KEY_THRESH_DOWN    /* row4 col4 */
} KeyCode;

void     KEY_Matrix_Init(void);
KeyCode  KEY_Matrix_Scan(void);

#endif

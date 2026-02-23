#ifndef __HX711_H
#define __HX711_H

#include "Systick.h"
#include <stdint.h>
#define HX711_OFFSET_DEFAULT   (-11140)
#define HX711_SCALE_DEFAULT    (673.47f)

//============ GPIO 定义修改 ============//
// 使用 PA0 作为 DATA
// 使用 PA1 作为 SCK
#define HX711_DATA   PAin(0)
#define HX711_SCK    PAout(1)

// GPIO 时钟
#define HX711_GPIO_CLK            RCC_APB2Periph_GPIOA
#define HX711_GPIO_APBxClkCmd     RCC_APB2PeriphClockCmd

// DATA 引脚
#define HX711_DATA_GPIO_PORT      GPIOA
#define HX711_DATA_GPIO_PIN       GPIO_Pin_0

// SCK 引脚
#define HX711_SCK_GPIO_PORT       GPIOA
#define HX711_SCK_GPIO_PIN        GPIO_Pin_1

// 函数声明
void    HX711_GPIO_Init(void);
void    HX711_Data_In(void);
void    HX711_Data_Out(void);

int32_t Read_HX711(void);

// ====== 新增：去皮 / 校准 / 称重接口 ======
void    HX711_Tare(uint16_t times);
void    HX711_Calibrate(uint16_t times, float known_weight_g);
float   HX711_GetWeight(uint16_t times);
int32_t HX711_GetOffset(void);
float   HX711_GetScale(void);
void    HX711_SetCalibration(int32_t offset, float scale);

#endif

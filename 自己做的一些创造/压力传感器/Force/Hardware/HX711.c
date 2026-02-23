#include "HX711.h"
#include "Systick.h"

static int32_t hx_offset = 0;      // 空载零点
static float   hx_scale  = 1.0f;   // 每克对应的 counts
// static int32_t last_net  = 0;   // 如果暂时不用稳定判断，可以先不用

/*******************************************************
 * GPIO 初始化
 *******************************************************/
void HX711_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    HX711_GPIO_APBxClkCmd(HX711_GPIO_CLK, ENABLE);

    // SCK 输出
    GPIO_InitStructure.GPIO_Pin   = HX711_SCK_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HX711_SCK_GPIO_PORT, &GPIO_InitStructure);

    // DATA 输入
    GPIO_InitStructure.GPIO_Pin  = HX711_DATA_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(HX711_DATA_GPIO_PORT, &GPIO_InitStructure);

    HX711_SCK = 0;
}

/*******************************************************
 * DATA 改为输出（一般很少用）
 *******************************************************/
void HX711_Data_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    HX711_GPIO_APBxClkCmd(HX711_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = HX711_DATA_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HX711_DATA_GPIO_PORT, &GPIO_InitStructure);
}

/*******************************************************
 * DATA 改为输入
 *******************************************************/
void HX711_Data_In(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    HX711_GPIO_APBxClkCmd(HX711_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin  = HX711_DATA_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(HX711_DATA_GPIO_PORT, &GPIO_InitStructure);
}

/*******************************************************
 * 读一次 HX711 原始 24 位值（标准有符号写法）
 *******************************************************/
int32_t Read_HX711(void)
{
    uint8_t  i;
    int32_t  value = 0;

    // 初始状态：SCK 低，DATA 拉高，再切回输入
    HX711_Data_Out();
    HX711_DATA = 1;          // 初始 DATA 高
    Delay_us(1);
    HX711_SCK  = 0;          // 初始 SCK 低
    HX711_Data_In();

    // 等待 DATA 变为低电平（数据准备好）
    while (HX711_DATA);
    Delay_us(1);

    // 24 位数据，从高位到低位
    for (i = 0; i < 24; i++)
    {
        HX711_SCK = 1;
        Delay_us(1);

        value <<= 1;
        if (HX711_DATA)
        {
            value |= 0x01;
        }

        HX711_SCK = 0;
        Delay_us(1);
    }

    // 第 25 个时钟，用来选择下次通道/增益（这里是 A 通道 128）
    HX711_SCK = 1;
    Delay_us(1);
    HX711_SCK = 0;
    Delay_us(1);

    // --- 正确的 24bit 补码 → 32bit 补码 符号扩展 ---
    if (value & 0x800000)       // 如果最高位是 1，负数
    {
        value |= 0xFF000000;    // 高 8 位补 1
    }
    else
    {
        value &= 0x00FFFFFF;    // 高 8 位清 0（可写可不写）
    }

    return value;
}

/*******************************************************
 * 多次读取求平均（内部工具函数）
 *******************************************************/
static int32_t HX711_ReadAverage(uint16_t times)
{
    int64_t sum = 0;
    uint16_t i;

    for (i = 0; i < times; i++)
    {
        sum += Read_HX711();
        Delay_ms(5);
    }

    return (int32_t)(sum / times);
}

/*******************************************************
 * 去皮：记录空载零点
 * times 建议 10~20
 *******************************************************/
void HX711_Tare(uint16_t times)
{
    hx_offset = HX711_ReadAverage(times);
}

/*******************************************************
 * 校准：放上已知重量（克），算比例系数
 * known_weight_g 例如 345.0f
 *******************************************************/
void HX711_Calibrate(uint16_t times, float known_weight_g)
{
    int32_t raw = HX711_ReadAverage(times);
    int32_t net = raw - hx_offset;   // 去掉零点

    if (known_weight_g > 0.1f)
    {
        hx_scale = (float)net / known_weight_g;   // 每克对应多少 counts
    }
    else
    {
        hx_scale = 1.0f;
    }
}

/*******************************************************
 * 称重：返回当前重量（克）
 * times 建议 5~10，做一点平均
 *******************************************************/
float HX711_GetWeight(uint16_t times)
{
    int32_t raw = HX711_ReadAverage(times);
    int32_t net = raw - hx_offset;

    if (hx_scale == 0.0f)
        hx_scale = 1.0f;

    return (float)net / hx_scale;
}
/*******************************************************
 * 读取当前的空载零点（用于显示 / 保存）
 *******************************************************/
int32_t HX711_GetOffset(void)
{
    return hx_offset;
}

/*******************************************************
 * 读取当前的比例系数（每克对应多少 counts）
 *******************************************************/
float HX711_GetScale(void)
{
    return hx_scale;
}

/*******************************************************
 * 直接设置标定参数（用于下次开机直接使用）
 *******************************************************/
void HX711_SetCalibration(int32_t offset, float scale)
{
    hx_offset = offset;
    hx_scale  = scale;
}



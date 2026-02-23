#include "Buzzer.h"
#include "SysTick.h"     // 用于 Delay_ms()

/*******************************************************************************
* 函数名  : Buzzer_Init
* 功能    : 蜂鸣器 PB0 初始化
*******************************************************************************/

void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // PB0 → TIM3_CH3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 定时器 72MHz
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1; // 1MHz
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;  // 1kHz PWM
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // PWM 模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 500; // 50% 占空比
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);

    TIM_Cmd(TIM3, ENABLE);
}



/*******************************************************************************
* 函数名  : Buzzer_On
* 功能    : 打开蜂鸣器
*******************************************************************************/
void Buzzer_On(void)
{
    TIM3->CCR3 = 900;   // 50% 占空比
}

/*******************************************************************************
* 函数名  : Buzzer_Off
* 功能    : 关闭蜂鸣器
*******************************************************************************/
void Buzzer_Off(void)
{
  TIM3->CCR3 = 0;     // 输出 0，蜂鸣器不响
}

/*******************************************************************************
* 函数名  : Buzzer_Beep
* 功能    : 蜂鸣器响 time_ms 毫秒
*******************************************************************************/
void Buzzer_Beep(uint16_t ms)
{
    Buzzer_On();
    Delay_ms(ms);
    Buzzer_Off();
}

void Buzzer_N_Beep(uint8_t count)
{
    for(uint8_t i = 0; i < count; i++)
    {
        Buzzer_On();
        Delay_ms(500);
        Buzzer_Off();
        Delay_ms(500);
    }
}


void PC13LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. 开启 GPIOC 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // 2. 配置 PC13 为推挽输出
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 3. 初始状态：熄灭（大部分板子是低亮高灭）
    PC13_LED_OFF();
}
void PC13_LED_Blink(uint8_t times)
{
    for(uint8_t i = 0; i < times; i++)
    {
        PC13_LED_ON();      // 亮
        Delay_ms(150);

        PC13_LED_OFF();     // 灭
        Delay_ms(150);
    }
}


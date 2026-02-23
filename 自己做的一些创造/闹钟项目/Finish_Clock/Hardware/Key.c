#include "Key.h"
#include "SysTick.h"

/*******************************************************************************
* 函 数 名         : KEY_Init
* 函数功能		   : 按键初始化
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; // 定义结构体变量	
	RCC_APB2PeriphClockCmd(KEY1_PORT_RCC,ENABLE);
	RCC_APB2PeriphClockCmd(KEY2_PORT_RCC,ENABLE);
	RCC_APB2PeriphClockCmd(KEY3_PORT_RCC,ENABLE);
	RCC_APB2PeriphClockCmd(KEY4_PORT_RCC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);
	
    // KEY1~KEY4 上拉输入（按下=0，松开=1）
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;      // 上拉输入  

    GPIO_InitStructure.GPIO_Pin   = KEY1_PIN;
    GPIO_Init(KEY1_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY2_PIN;
    GPIO_Init(KEY2_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY3_PIN;
    GPIO_Init(KEY3_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = KEY4_PIN;
    GPIO_Init(KEY4_PORT, &GPIO_InitStructure);
}

/*******************************************************************************
* 函 数 名         : KEY_Scan
* 函数功能		   : 单独按键扫描检测
* 输    入         : mode=0: 单次按下触发
*                    mode=1: 连续触发
*******************************************************************************/
u8 KEY_Scan(u8 mode)
{
    static u8 key = 1;
	
    if(mode == 1) // 允许长按连续触发
        key = 1;

    if(key == 1 && (KEY1 == 0 || KEY2 == 0 || KEY3 == 0 || KEY4 == 0)) // 有任意按键按下
    {
        Delay_ms(10);  // 简单消抖
        key = 0;
        if(KEY1 == 0)      return KEY1_PRESS; 
        else if(KEY2 == 0) return KEY2_PRESS; 
        else if(KEY3 == 0) return KEY3_PRESS; 
        else if(KEY4 == 0) return KEY4_PRESS; 
    }
    else if(KEY1 == 1 && KEY2 == 1 && KEY3 == 1 && KEY4 == 1)    // 没有按键按下
        key = 1;

    return 0;
}

/*******************************************************************************
* 函 数 名       : KEY_Matrix_Init
* 函数功能		 : 矩阵按键初始化（4x4）
*******************************************************************************/
void KEY_Matrix_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;// 定义结构体变量
	
    RCC_APB2PeriphClockCmd(KEY_MATRIX_H1_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_H2_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_H3_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_H4_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L1_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L2_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L3_PORT_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY_MATRIX_L4_PORT_RCC, ENABLE);
	
    // 行线：推挽输出
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
	
    // 列线：默认下拉，按下后会被对应行拉高为 1
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;	

    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L1_PIN;  
    GPIO_Init(KEY_MATRIX_L1_PORT, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L2_PIN;  
    GPIO_Init(KEY_MATRIX_L2_PORT, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L3_PIN;  
    GPIO_Init(KEY_MATRIX_L3_PORT, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin   = KEY_MATRIX_L4_PIN;  
    GPIO_Init(KEY_MATRIX_L4_PORT, &GPIO_InitStructure);
}

/*******************************************************************************
* 函 数 名       : KEY_Matrix_Scan
* 函数功能		 : 检测矩阵按键是否按下，按下则返回对应键值
* 输    出    	 : 1-16，对应S1-S16键，0：按键未按下
*******************************************************************************/
KeyCode KEY_Matrix_Scan(void)
{
    u8 col1, col2, col3, col4;
    KeyCode key_value = KEY_NONE;     // 当前识别出的键值

	
       // 先把 4 行全拉高，判断有没有键被按下
    GPIO_SetBits(KEY_MATRIX_H1_PORT, KEY_MATRIX_H1_PIN);
    GPIO_SetBits(KEY_MATRIX_H2_PORT, KEY_MATRIX_H2_PIN);
    GPIO_SetBits(KEY_MATRIX_H3_PORT, KEY_MATRIX_H3_PIN);
    GPIO_SetBits(KEY_MATRIX_H4_PORT, KEY_MATRIX_H4_PIN);
	
    if((GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN) |
        GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN) |
        GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN) |
        GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN)) == 0)  
        return KEY_NONE; // 没有按键按下  

    Delay_ms(5);    // 等 5ms 去抖动

    if((GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN) |
        GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN) |
        GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN) |
        GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN)) == 0)
        return KEY_NONE;

    /********** 第1行：1 2 3 MENU **********/
    GPIO_SetBits(KEY_MATRIX_H1_PORT, KEY_MATRIX_H1_PIN);
    GPIO_ResetBits(KEY_MATRIX_H2_PORT, KEY_MATRIX_H2_PIN);
    GPIO_ResetBits(KEY_MATRIX_H3_PORT, KEY_MATRIX_H3_PIN);
    GPIO_ResetBits(KEY_MATRIX_H4_PORT, KEY_MATRIX_H4_PIN); 
	
    col1 = GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN);
    col2 = GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN);
    col3 = GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN);
    col4 = GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN);
	
    if(col1) key_value = KEY_NUM1;
    if(col2) key_value = KEY_NUM2;
    if(col3) key_value = KEY_NUM3;
    if(col4) key_value = KEY_MENU;

    while((GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN)) > 0);

    /********** 第2行：4 5 6 YES **********/
    GPIO_ResetBits(KEY_MATRIX_H1_PORT, KEY_MATRIX_H1_PIN);
    GPIO_SetBits(KEY_MATRIX_H2_PORT, KEY_MATRIX_H2_PIN);
    GPIO_ResetBits(KEY_MATRIX_H3_PORT, KEY_MATRIX_H3_PIN);
    GPIO_ResetBits(KEY_MATRIX_H4_PORT, KEY_MATRIX_H4_PIN); 
	
    col1 = GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN);
    col2 = GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN);
    col3 = GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN);
    col4 = GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN);
	
    if(col1) key_value = KEY_NUM4;
    if(col2) key_value = KEY_NUM5;
    if(col3) key_value = KEY_NUM6;
    if(col4) key_value = KEY_YES;

    while((GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN)) > 0);

    /********** 第3行：7 8 9 Exit **********/
    GPIO_ResetBits(KEY_MATRIX_H1_PORT, KEY_MATRIX_H1_PIN);
    GPIO_ResetBits(KEY_MATRIX_H2_PORT, KEY_MATRIX_H2_PIN);
    GPIO_SetBits(KEY_MATRIX_H3_PORT, KEY_MATRIX_H3_PIN);
    GPIO_ResetBits(KEY_MATRIX_H4_PORT, KEY_MATRIX_H4_PIN); 
	
    col1 = GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN);
    col2 = GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN);
    col3 = GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN);
    col4 = GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN);
	
    if(col1) key_value = KEY_NUM7;
    if(col2) key_value = KEY_NUM8;
    if(col3) key_value = KEY_NUM9;
    if(col4) key_value = KEY_Exit;

    while((GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN)) > 0);

    /********** 第4行：UP_LEFT  0  DOWN_RIGHT  SETTING **********/
    GPIO_ResetBits(KEY_MATRIX_H1_PORT, KEY_MATRIX_H1_PIN);
    GPIO_ResetBits(KEY_MATRIX_H2_PORT, KEY_MATRIX_H2_PIN);
    GPIO_ResetBits(KEY_MATRIX_H3_PORT, KEY_MATRIX_H3_PIN);
    GPIO_SetBits(KEY_MATRIX_H4_PORT, KEY_MATRIX_H4_PIN); 
	
    col1 = GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN);
    col2 = GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN);
    col3 = GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN);
    col4 = GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN);
	
    if(col1) key_value = KEY_UP_LEFT;
    if(col2) key_value = KEY_NUM0;
    if(col3) key_value = KEY_DOWN_RIGHT;
    if(col4) key_value = KEY_SETTING;

    while((GPIO_ReadInputDataBit(KEY_MATRIX_L1_PORT,KEY_MATRIX_L1_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L2_PORT,KEY_MATRIX_L2_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L3_PORT,KEY_MATRIX_L3_PIN) |
           GPIO_ReadInputDataBit(KEY_MATRIX_L4_PORT,KEY_MATRIX_L4_PIN)) > 0);
	
    return key_value;
}




#include "oled.h"
#include "SysTick.h"
#include "oledfont.h" 
#include "system.h"
/*******************************************************************************
* Function Name: IIC_Init
* Description: IIC初始化
*******************************************************************************/
void IIC_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(IIC_SCL_PORT_RCC|IIC_SDA_PORT_RCC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
	GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
	
	IIC_SCL = 1;
	IIC_SDA = 1;	
}

/*******************************************************************************
* Function Name: SDA_OUT
* Description: SDA设置为输出
*******************************************************************************/
void SDA_OUT(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name: SDA_IN
* Description: SDA设置为输入
*******************************************************************************/
void SDA_IN(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name: IIC_Start
* Description: 产生IIC起始信号
*******************************************************************************/
void IIC_Start(void)
{
	SDA_OUT();
	IIC_SDA = 1;	  
	IIC_SCL = 1;
	Delay_us(5);
 	IIC_SDA = 0; // START: when CLK is high, DATA change form high to low 
	Delay_us(6);
	IIC_SCL = 0; // 钳住I2C总线，准备发送数据
}	

/*******************************************************************************
* Function Name: IIC_Stop
* Description: 产生IIC停止信号
*******************************************************************************/
void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SCL = 0;
	IIC_SDA = 0; // STOP: when CLK is high DATA change form low to high
 	IIC_SCL = 1; 
	Delay_us(6); 
	IIC_SDA = 1; // 释放I2C总线
	Delay_us(6);							   	
}

/*******************************************************************************
* Function Name: IIC_Wait_Ack
* Description: 等待应答信号到来
* Return: 1，接收应答失败; 0，接收应答成功
*******************************************************************************/
u8 IIC_Wait_Ack(void)
{
	u8 tempTime = 0;
	
	IIC_SDA = 1;
	Delay_us(1);
	SDA_IN(); // SDA设置为输入
	IIC_SCL = 1;
	Delay_us(1);	 
	while(READ_SDA)
	{
		tempTime++;
		if(tempTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0; // 时钟输出0
	return 0;  
} 

/*******************************************************************************
* Function Name: IIC_Ack
* Description: 产生ACK应答
*******************************************************************************/
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	Delay_us(2);
	IIC_SCL = 1;
	Delay_us(5);
	IIC_SCL = 0;
}

/*******************************************************************************
* Function Name: IIC_NAck
* Description: 产生NACK非应答
*******************************************************************************/		    
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	Delay_us(2);
	IIC_SCL = 1;
	Delay_us(5);
	IIC_SCL = 0;
}	

/*******************************************************************************
* Function Name: IIC_Send_Byte
* Description: IIC发送一个字节
* Parameters: txd: 要发送的字节
*******************************************************************************/		  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL = 0; // 拉低时钟开始数据传输
    for(t = 0; t < 8; t++)
    {              
        if((txd & 0x80) > 0) // 0x80 = 1000 0000
			IIC_SDA = 1;
		else
			IIC_SDA = 0;
        txd <<= 1; 	  
		Delay_us(2);
		IIC_SCL = 1;
		Delay_us(2); 
		IIC_SCL = 0;	
		Delay_us(2);
    }	 
} 

/*******************************************************************************
* Function Name: IIC_Read_Byte
* Description: IIC读取一个字节
* Parameters: ack=1时发送ACK，ack=0发送nACK
* Return: 读取到的数据
*******************************************************************************/  
u8 IIC_Read_Byte(u8 ack)
{
	u8 i, receive = 0;
	SDA_IN(); // SDA设置为输入
    for(i = 0; i < 8; i++)
	{
        IIC_SCL = 0; 
        Delay_us(2);
		IIC_SCL = 1;
        receive <<= 1;
        if(READ_SDA) receive++;   
		Delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck(); // 发送nACK
    else
        IIC_Ack(); // 发送ACK   
    return receive;
}

// OLED显存
// 格式: 垂直8像素一页，共8页
u8 OLED_GRAM[128][8];

// IIC写命令
void write_iic_cmd(u8 cmd)
{
	IIC_Start();
	IIC_Send_Byte(0x78); // Slave address, SA0=0
	IIC_Wait_Ack();	
	IIC_Send_Byte(0x00); // write command
	IIC_Wait_Ack();	
	IIC_Send_Byte(cmd); 
	IIC_Wait_Ack();	
	IIC_Stop();
}

// IIC写数据
void write_iic_data(u8 dat)
{
	IIC_Start();
	IIC_Send_Byte(0x78); // Slave address, SA0=0
	IIC_Wait_Ack();	
	IIC_Send_Byte(0x40); // write data
	IIC_Wait_Ack();	
	IIC_Send_Byte(dat); 
	IIC_Wait_Ack();	
	IIC_Stop();
}

// 向SSD1306写入一个字节
// dat: 要写入的数据/命令
// cmd: 数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(u8 dat, u8 cmd)
{
	if(cmd)
		write_iic_data(dat);
	else
		write_iic_cmd(dat);
}

// 更新显存到LCD
void OLED_Refresh_Gram(void)
{
	int i, n;		    
	for(i = 7; i >= 0; i--) // 修改刷屏方向
	{  
		OLED_WR_Byte(0xb0 + i, OLED_CMD); // 设置页地址 0~7
		OLED_WR_Byte(0x00, OLED_CMD);     // 设置显示位置-列低地址
		OLED_WR_Byte(0x10, OLED_CMD);     // 设置显示位置-列高地址
		for(n = 0; n < 128; n++) 
			OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA); 
	}   
}

// 设置坐标
void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 
	OLED_WR_Byte(0xb0 + y, OLED_CMD);
	OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
	OLED_WR_Byte((x & 0x0f) | 0x01, OLED_CMD); 
}   	  

// 开启OLED显示
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  // SET DCDC命令
	OLED_WR_Byte(0X14, OLED_CMD);  // DCDC ON
	OLED_WR_Byte(0XAF, OLED_CMD);  // DISPLAY ON
}

// 关闭OLED显示
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D, OLED_CMD);  // SET DCDC命令
	OLED_WR_Byte(0X10, OLED_CMD);  // DCDC OFF
	OLED_WR_Byte(0XAE, OLED_CMD);  // DISPLAY OFF
}
		   			 
// 清屏函数
void OLED_Clear(void)  
{  
	u8 i, n;		    
	for(i = 0; i < 8; i++)  
	{    
		for(n = 0; n < 128; n++)
		{
			OLED_GRAM[n][i] = 0;
		}
	}
	OLED_Refresh_Gram(); // 更新显示
}

// m的n次方
u32 oled_pow(u8 m, u8 n)
{
	u32 result = 1;	 
	while(n--) result *= m;    
	return result;
}

// 画点
// x:0~127
// y:0~63
// t:1 填充 0,清空
void OLED_DrawPoint(u8 x, u8 y, u8 t)
{
	u8 pos, bx, temp = 0;
	if(x > 127 || y > 63) return; // 超出范围
	pos = 7 - y / 8;
	bx = y % 8;
	temp = 1 << (7 - bx);
	if(t) 
		OLED_GRAM[x][pos] |= temp;
	else 
		OLED_GRAM[x][pos] &= ~temp;		    
}

// 填充区域
// x1,y1,x2,y2 填充区域的左上角和右下角
void OLED_Fill(u8 x1, u8 y1, u8 x2, u8 y2, u8 dot)  
{  
	u8 x, y;  
	for(x = x1; x <= x2; x++)
	{
		for(y = y1; y <= y2; y++)
		{ 	
			OLED_DrawPoint(x, y, dot);	
		}
	}													    
}

// 在指定位置显示一个字符
// x:0~127
// y:0~63
// mode:0,反色显示;1,正常显示
// size:选择字体 12/16/24
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode)
{      			    
	u8 temp, t, t1;
	u8 y0 = y;
	u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // 得到字体一个字符对应点阵集所占的字节数
	chr = chr - ' '; // 得到偏移后的值
    for(t = 0; t < csize; t++)
    {   
		if(size == 12) temp = ascii_1206[chr][t];     // 调用1206字体
		else if(size == 16) temp = ascii_1608[chr][t]; // 调用1608字体
		else if(size == 24) temp = ascii_2412[chr][t]; // 调用2412字体
		else return; // 没有的字体
        for(t1 = 0; t1 < 8; t1++)
		{
			if(temp & 0x80) OLED_DrawPoint(x, y, mode);
			else OLED_DrawPoint(x, y, !mode);
			temp <<= 1;
			y++;
			if((y - y0) == size)
			{
				y = y0;
				x++;
				break;
			}
		}  	 
    }          
}

// 显示数字
// x,y :起始坐标
// len :数字的位数
// size:字体大小
// num:数值
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size)
{         	
	u8 t, temp;
	u8 enshow = 0;						   
	for(t = 0; t < len; t++)
	{
		temp = (num / oled_pow(10, len - t - 1)) % 10;
		if(enshow == 0 && t < (len - 1))
		{
			if(temp == 0)
			{
				OLED_ShowChar(x + (size / 2) * t, y, ' ', size, 1);
				continue;
			}
			else enshow = 1;
		}
	 	OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, 1); 
	}
} 

// 显示字符串
// x,y:起始坐标
// size:字体大小
// *p:字符串起始地址
void OLED_ShowString(u8 x, u8 y, const u8 *p, u8 size)
{	
    while((*p <= '~') && (*p >= ' ')) // 判断是不是非法字符
    {       
        if(x > (128 - (size / 2))) { x = 0; y += size; }
        if(y > (64 - size)) { y = x = 0; OLED_Clear(); }
        OLED_ShowChar(x, y, *p, size, 1);	 
        x += size / 2;
        p++;
    }  	
}

// OLED初始化
void OLED_Init(void)
{	  	
	IIC_Init();
	
	OLED_WR_Byte(0xAE, OLED_CMD); // 关闭显示
	OLED_WR_Byte(0xD5, OLED_CMD); // 设置时钟分频比,振荡器频率
	OLED_WR_Byte(80, OLED_CMD);   // [3:0],分频系数;[7:4],振荡器频率
	OLED_WR_Byte(0xA8, OLED_CMD); // 设置驱动路数
	OLED_WR_Byte(0X3F, OLED_CMD); // 默认0X3F(1/64) 
	OLED_WR_Byte(0xD3, OLED_CMD); // 设置显示偏移
	OLED_WR_Byte(0X00, OLED_CMD); // 默认为0

	OLED_WR_Byte(0x40, OLED_CMD); // 设置显示开始行 [5:0],行数.
													    
	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵设置
	OLED_WR_Byte(0x14, OLED_CMD); // bit2，开启/关闭
	OLED_WR_Byte(0x20, OLED_CMD); // 设置内存地址模式
	OLED_WR_Byte(0x02, OLED_CMD); // [1:0],00,行地址模式;01,列地址模式;10,页地址模式;默认10;
	OLED_WR_Byte(0xA1, OLED_CMD); // 段重定义设置,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0, OLED_CMD); // 设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
	OLED_WR_Byte(0xDA, OLED_CMD); // 设置COM硬件引脚配置
	OLED_WR_Byte(0x12, OLED_CMD); // [5:4]配置
		 
	OLED_WR_Byte(0x81, OLED_CMD); // 对比度设置
	OLED_WR_Byte(0xEF, OLED_CMD); // 1~255;默认0X7F (亮度设置,越大越亮)
	OLED_WR_Byte(0xD9, OLED_CMD); // 设置预充电周期
	OLED_WR_Byte(0xf1, OLED_CMD); // [3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB, OLED_CMD); // 设置VCOMH 电压倍率
	OLED_WR_Byte(0x30, OLED_CMD); // [6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4, OLED_CMD); // 全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_WR_Byte(0xA6, OLED_CMD); // 设置显示方式;bit0:1,反相显示;0,正常显示
	OLED_WR_Byte(0xAF, OLED_CMD); // 开启显示

	OLED_Clear();
}

// 只刷新部分页（page_start ~ page_end），减少刷新时间
// page 范围 0~7，对应整屏的 8 个“块”
void OLED_Refresh_Pages(u8 page_start, u8 page_end)
{
    int i, n;

    if(page_end > 7) page_end = 7;

    for(i = page_end; i >= page_start; i--)
    {
        OLED_WR_Byte(0xb0 + i, OLED_CMD); // 设置页地址
        OLED_WR_Byte(0x00, OLED_CMD);     // 列低地址
        OLED_WR_Byte(0x10, OLED_CMD);     // 列高地址

        for(n = 0; n < 128; n++)
        {
            OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
        }
    }
}
//======================================================================
// 行/列坐标兼容：把 (line, column) 转成像素坐标 (x, y)
//======================================================================
static void RC_To_XY(u8 line, u8 column, u8 *x, u8 *y)
{
    // 你现在用的是 16 点阵字符：宽 8 像素、高 16 像素
    *x = (column - 1) * 8;   // 列 1~16 -> X 像素 0~127
    *y = (line   - 1) * 16;  // 行 1~4  -> Y 像素 0~63
}

// 行/列版显示字符串：接口类似原教程 OLED_ShowString(1,1,"xxx");
void OLED_ShowString_RC(u8 line, u8 column, const char *str)
{
    u8 x, y;
    RC_To_XY(line, column, &x, &y);
    OLED_ShowString(x, y, (const u8 *)str, 16);  // 使用 16 点阵字体
}

// 行/列版显示数字：接口类似原教程 OLED_ShowNum(1,6,num,4);
void OLED_ShowNum_RC(u8 line, u8 column, u32 num, u8 len)
{
    u8 x, y;
    RC_To_XY(line, column, &x, &y);
    OLED_ShowNum(x, y, num, len, 16);            // 使用 16 点阵字体
}



#ifndef _oled_H
#define _oled_H

#include "system.h"




/* ==================== IIC硬件引脚定义 ==================== */
/* IIC_SCL时钟引脚配置 */
#define IIC_SCL_PORT 			GPIOB   
#define IIC_SCL_PIN 			(GPIO_Pin_6)
#define IIC_SCL_PORT_RCC		RCC_APB2Periph_GPIOB

/* IIC_SDA数据引脚配置 */
#define IIC_SDA_PORT 			GPIOB  
#define IIC_SDA_PIN 			(GPIO_Pin_7)
#define IIC_SDA_PORT_RCC		RCC_APB2Periph_GPIOB

/* IO口操作宏定义 */	 
#define IIC_SCL    PBout(6) // SCL引脚输出
#define IIC_SDA    PBout(7) // SDA引脚输出	 
#define READ_SDA   PBin(7)  // SDA引脚输入读取

/* ==================== IIC底层通信函数声明 ==================== */
/**
 * @brief  初始化IIC通信的GPIO引脚
 * @note   SCL/SDA均配置为推挽输出，初始电平为高
 * @param  无
 * @retval 无
 */
void IIC_Init(void);               

/**
 * @brief  产生IIC起始信号（SCL高电平时，SDA由高变低）
 * @param  无
 * @retval 无
 */
void IIC_Start(void);				

/**
 * @brief  产生IIC停止信号（SCL高电平时，SDA由低变高）
 * @param  无
 * @retval 无
 */
void IIC_Stop(void);	  			

/**
 * @brief  IIC总线发送一个字节数据
 * @param  txd: 要发送的8位数据
 * @retval 无
 */
void IIC_Send_Byte(u8 txd);			

/**
 * @brief  IIC总线读取一个字节数据
 * @param  ack: 应答标志，1=发送ACK，0=发送NACK
 * @retval 读取到的8位数据
 */
u8 IIC_Read_Byte(u8 ack);			

/**
 * @brief  等待IIC从设备的应答信号
 * @param  无
 * @retval 0=应答成功，1=应答超时/失败
 */
u8 IIC_Wait_Ack(void); 				

/**
 * @brief  主动发送ACK应答信号
 * @param  无
 * @retval 无
 */
void IIC_Ack(void);					

/**
 * @brief  主动发送NACK非应答信号
 * @param  无
 * @retval 无
 */
void IIC_NAck(void);				

/* ==================== OLED显示屏参数定义 ==================== */
#define SIZE 16                      // 默认字体大小（16号字体）
#define XLevelL		0x00             // 列地址低字节基础值
#define XLevelH		0x10             // 列地址高字节基础值
#define Max_Column	128              // OLED最大列数（0~127）
#define Max_Row		64               // OLED最大行数（0~63）
#define	Brightness	0xFF             // OLED默认亮度（0~255，值越大越亮）
#define X_WIDTH 	128              // OLED宽度（像素）
#define Y_WIDTH 	64               // OLED高度（像素）

#define OLED_CMD  0	                // 写命令标志
#define OLED_DATA 1	                // 写数据标志

/* ==================== OLED核心操作函数声明 ==================== */
/**
 * @brief  向OLED写入一个字节（命令/数据）
 * @param  dat: 要写入的字节数据
 * @param  cmd: 数据类型，0=命令，1=数据
 * @retval 无
 */
void OLED_WR_Byte(u8 dat,u8 cmd);	    

/**
 * @brief  开启OLED显示（唤醒屏幕）
 * @param  无
 * @retval 无
 */
void OLED_Display_On(void);

/**
 * @brief  关闭OLED显示（屏幕休眠，显存保留）
 * @param  无
 * @retval 无
 */
void OLED_Display_Off(void);

/**
 * @brief  设置OLED显示坐标
 * @param  x: 列坐标（0~127）
 * @param  y: 页坐标（0~7，每页对应8行像素）
 * @retval 无
 */
void OLED_Set_Pos(unsigned char x, unsigned char y);	   							   		    

/**
 * @brief  OLED初始化（配置SSD1306控制器寄存器）
 * @note   包含引脚初始化、时序配置、亮度配置等
 * @param  无
 * @retval 无
 */
void OLED_Init(void);

/**
 * @brief  刷新显存到OLED屏幕
 * @note   将OLED_GRAM数组数据写入屏幕显示
 * @param  无
 * @retval 无
 */
void OLED_Refresh_Gram(void);

/**
 * @brief  清屏操作（清空显存并刷新）
 * @param  无
 * @retval 无
 */
void OLED_Clear(void);

/* ==================== OLED绘图函数声明 ==================== */
/**
 * @brief  在指定坐标画点
 * @param  x: 列坐标（0~127）
 * @param  y: 行坐标（0~63）
 * @param  t: 点状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_DrawPoint(u8 x,u8 y,u8 t);

/**
 * @brief  填充指定矩形区域
 * @param  x1/y1: 矩形左上角坐标
 * @param  x2/y2: 矩形右下角坐标
 * @param  dot: 填充状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);

/**
 * @brief  填充指定尺寸的矩形（简化版）
 * @param  x0/y0: 矩形左上角坐标
 * @param  width: 矩形宽度（像素）
 * @param  height: 矩形高度（像素）
 * @param  color: 填充状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Fill_rectangle(u8 x0,u8 y0,u8 width,u8 height,u8 color);

/**
 * @brief  绘制任意直线（Bresenham算法）
 * @param  x1/y1: 直线起点坐标
 * @param  x2/y2: 直线终点坐标
 * @param  color: 线状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2,u8 color);

/**
 * @brief  绘制矩形框（仅边框）
 * @param  x1/y1: 矩形左上角坐标
 * @param  x2/y2: 矩形右下角坐标
 * @param  color: 框状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_DrawRectangle(u8 x1, u8 y1, u8 x2, u8 y2,u8 color);

/**
 * @brief  绘制圆形（仅边框，Bresenham算法）
 * @param  x0/y0: 圆心坐标
 * @param  r: 圆半径（像素）
 * @param  color: 圆状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Draw_Circle(u8 x0,u8 y0,u8 r,u8 color);

/**
 * @brief  绘制大圆点（3x3像素）
 * @param  x0/y0: 圆点中心坐标
 * @param  color: 点状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Draw_Bigpoint(u8 x0,u8 y0,u8 color);

/**
 * @brief  绘制垂直线
 * @param  x0/y0: 直线起点坐标
 * @param  len: 直线长度（像素）
 * @param  color: 线状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Draw_vline(u8 x0,u8 y0,u8 len,u8 color);

/**
 * @brief  绘制水平线
 * @param  x0/y0: 直线起点坐标
 * @param  len: 直线长度（像素）
 * @param  color: 线状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Draw_hline(u8 x0,u8 y0,u8 len,u8 color);

/**
 * @brief  填充圆形（实心圆）
 * @param  x0/y0: 圆心坐标
 * @param  r: 圆半径（像素）
 * @param  color: 填充状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Fill_circle(u8 x0,u8 y0,u8 r,u8 color);

/**
 * @brief  绘制椭圆（仅边框）
 * @param  x0/y0: 椭圆中心坐标
 * @param  rx: x轴半径（像素）
 * @param  ry: y轴半径（像素）
 * @param  color: 椭圆状态，1=点亮，0=熄灭
 * @retval 无
 */
void OLED_Draw_ellipse(u8 x0, u8 y0, u8 rx, u8 ry,u8 color);

/* ==================== OLED字符/字符串显示函数声明 ==================== */
/**
 * @brief  显示单个ASCII字符
 * @param  x: 起始列坐标（0~127）
 * @param  y: 起始行坐标（0~63）
 * @param  chr: 要显示的字符（ASCII码，范围:空格~~）
 * @param  size: 字体大小，支持12/16/24（对应12x6/16x8/24x12点阵）
 * @param  mode: 显示模式，0=反色显示，1=正常显示
 * @retval 无
 */
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);

/**
 * @brief  显示十进制数字（无符号）
 * @param  x: 起始列坐标（0~127）
 * @param  y: 起始行坐标（0~63）
 * @param  num: 要显示的数字（0~4294967295）
 * @param  len: 数字显示位数（1~10）
 * @param  size: 字体大小，支持12/16/24
 * @retval 无
 */
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);

/**
 * @brief  显示ASCII字符串
 * @param  x: 起始列坐标（0~127）
 * @param  y: 起始行坐标（0~63）
 * @param  p: 字符串指针（仅支持ASCII码:空格~~）
 * @param  size: 字体大小，支持12/16/24
 * @retval 无
 */
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size); 

/**
 * @brief  显示中文汉字（基于字库索引）
 * @param  x: 起始列坐标（0~127）
 * @param  y: 起始行坐标（0~63）
 * @param  pos: 汉字在字库中的索引位置
 * @param  size: 字体大小，支持12/16/24（对应12x12/16x16/24x24点阵）
 * @param  mode: 显示模式，0=反色显示，1=正常显示
 * @retval 无
 */
void OLED_ShowFontHZ(u8 x,u8 y,u8 pos,u8 size,u8 mode);

/**
 * @brief  显示BMP位图（128x64分辨率）
 * @param  x0/y0: 位图起始坐标
 * @param  x1/y1: 位图结束坐标
 * @param  BMP: 位图数据数组指针（格式:逐行8像素为1字节）
 * @retval 无
 */
void OLED_DrawBMP(u8 x0, u8 y0,u8 x1, u8 y1,u8 BMP[]);
void OLED_Refresh_Pages(u8 page_start, u8 page_end);

void OLED_ShowString_RC(u8 line, u8 column, const char *str);
void OLED_ShowNum_RC(u8 line, u8 column, u32 num, u8 len);
void OLED_ShowChinese16(u8 x, u8 y, u8 pos);
void OLED_ShowChinese_RC(u8 line, u8 column, u8 pos);
#endif


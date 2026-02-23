#include "usart.h"		 

int fputc(int ch,FILE *p)  // printf 输出会自动走这里发数据
{
	USART_SendData(USART1,(u8)ch);	
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	return ch;
}

// 串口1的中断就在这里处理
// 要先读 USARTx->SR，能避免一些莫名其妙的错误   	
u8 USART1_RX_BUF[USART1_REC_LEN];     // 接收缓存，最多存 USART_REC_LEN 个字节.
// 接收进度状态
// bit15：收到完整一帧
// bit14：已经收到了0x0d
// bit13~0：当前有效字节数量
u16 USART1_RX_STA=0;       // 记录当前的接收状态


/*******************************************************************************
* 函 数 名         : USART1_Init
* 函数功能		   : USART1初始化函数
* 输    入         : bound:波特率
* 输    出         : 无
*******************************************************************************/ 
void USART1_Init(u32 bound)
{
   // GPIO 口的配置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
 
	
	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;//TX			   // PA9 作为串口发送脚
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;	    // 复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);  /* 初始化串口输入IO */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//RX			 // PA10 作为串口接收脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;		  // 浮空输入
	GPIO_Init(GPIOA,&GPIO_InitStructure); /* 初始化GPIO */
	

	// 配置 USART1
	USART_InitStructure.USART_BaudRate = bound;// 设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 数据位用8位格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;// 停止位 1 位
	USART_InitStructure.USART_Parity = USART_Parity_No;// 不做奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// 不用硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	// 同时开收和发
	USART_Init(USART1, &USART_InitStructure); // 初始化串口1
	
	USART_Cmd(USART1, ENABLE);  // 打开串口1 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
		
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);// 打开接收中断

	// NVIC 里把串口1 的中断配好
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;// 选择串口1 的中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;// 抢占优先级 3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		// 次优先级 3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			// 使能这个 IRQ 通道
	NVIC_Init(&NVIC_InitStructure);	// 按上面的参数初始化 NVIC 寄存器	
}

/*******************************************************************************
* 函 数 名         : USART1_IRQHandler
* 函数功能		   : USART1 的中断处理
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/ 
void USART1_IRQHandler(void)                	// 串口1的中断服务程序
{
	u8 r;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  // 收到数据触发的中断
	{
		r =USART_ReceiveData(USART1);//(USART1->DR);	// 把收到的字节读出来
		if((USART1_RX_STA&0x8000)==0)// 还没收完
		{
			if(USART1_RX_STA&0x4000)// 已经收到了0x0d
			{
				if(r!=0x0a)USART1_RX_STA=0;// 校验不对就清零重来
				else USART1_RX_STA|=0x8000;	// 收满一帧了 
			}
			else // 还没等到0X0D
			{	
				if(r==0x0d)USART1_RX_STA|=0x4000;
				else
				{
					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=r;
					USART1_RX_STA++;
					if(USART1_RX_STA>(USART1_REC_LEN-1))USART1_RX_STA=0;// 超出缓存就清零重新收	  
				}		 
			}
		}   		 
	} 
}

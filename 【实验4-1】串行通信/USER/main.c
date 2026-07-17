#include "stm32f4xx.h"

/* 变量定义区 */
u16 i = 0; //一个循环用变量
u8 OD_Flag = 0; //收到换行符的标志
u8 Rx_Frame_Flag = 0; //收到完整帧的标志
u8 Rx_Buf[64]; //接收缓存
u16 Rx_Con; //接收计数器
u16 Rx_Len; //接收长度
u8 Tx_Buf[64]; //发送缓存
u8 Error = 0; //错误指令的标志
u8 Cmd_Buf[32]; //指令缓存
u16 Cmd_Len; //指令长度

/* 结构体变量定义 */
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

/* 函数声明区 */
void USART2_Send_Frame(u8* data, u16 len);
void USART2_Send_String(u8* str);
void delay_us(uint16_t nus);
void delay_ms(u16 nms);
void Process_Command(void);
void LED_Control(u8 led_num, u8 state);

int main(void)
{
	/*配置三个LED*/
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOB, ENABLE );
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1);
	GPIO_Init( GPIOB, &GPIO_InitStructure );

	/*配置蜂鸣器*/
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOA, ENABLE );
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	
	/*使能GPIOA和USART2时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

	/*USART2 GPIO配置*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/*USART2 对应引脚复用映射*/
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);

	/*USART2 端口配置*/
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/** 初始化串口后，发送欢迎信息 **/
	USART2_Send_String("\r\n========== LED控制系统 ==========\r\n");
	USART2_Send_String("指令格式: LEDx ON/OFF   (x=1,2,3)\r\n");
	USART2_Send_String("例如: LED1 ON  点亮LED1\r\n");
	USART2_Send_String("      LED2 OFF 熄灭LED2\r\n");
	USART2_Send_String("      BEEP     蜂鸣器响一声\r\n");
	USART2_Send_String("      HELP     显示帮助信息\r\n");
	USART2_Send_String("===================================\r\n");
	USART2_Send_String("\r\n请输入指令:\r\n");
	
	while(1)
	{
		if(Rx_Frame_Flag)//收到一帧数据
		{
			Rx_Frame_Flag = 0;//清除标志位
			Process_Command();//处理指令
		}
	}
}

/******** 指令处理函数 ********/
void Process_Command(void)
{
	u8 i;
	u8 led_num = 0;
	u8 state = 0;
	
	/* 复制指令到Cmd_Buf */
	Cmd_Len = Rx_Len;
	for(i = 0; i < Rx_Len; i++)
	{
		Cmd_Buf[i] = Rx_Buf[i];
	}
	
	/* 去掉末尾的回车换行符 */
	for(i = 0; i < Cmd_Len; i++)
	{
		if(Cmd_Buf[i] == '\r' || Cmd_Buf[i] == '\n')
		{
			Cmd_Buf[i] = '\0';
			Cmd_Len = i;
			break;
		}
	}
	
	/* 将指令转换为大写，方便比较 */
	for(i = 0; i < Cmd_Len; i++)
	{
		if(Cmd_Buf[i] >= 'a' && Cmd_Buf[i] <= 'z')
			Cmd_Buf[i] -= 32;
	}
	
	/* 判断指令类型 */
	if(('L' == Cmd_Buf[0]) && ('E' == Cmd_Buf[1]) && ('D' == Cmd_Buf[2]))//LED指令
	{
		/* 提取LED编号 */
		if(Cmd_Len >= 4 && Cmd_Buf[3] >= '1' && Cmd_Buf[3] <= '3')
		{
			led_num = Cmd_Buf[3] - '0';//转成数字 1,2,3
		}
		else
		{
			USART2_Send_String("错误: LED编号必须是1-3\r\n");
			return;
		}
		
		/* 解析ON/OFF状态 */
		if(('O' == Cmd_Buf[4]) && ('N' == Cmd_Buf[5]))//ON
		{
			state = 1;
			LED_Control(led_num, state);
			USART2_Send_String("LED");
			USART2_Send_Frame(&Cmd_Buf[3], 1);
			USART2_Send_String(" 已点亮\r\n");
		}
		else if(('O' == Cmd_Buf[4]) && ('F' == Cmd_Buf[5]) && ('F' == Cmd_Buf[6]))//OFF
		{
			state = 0;
			LED_Control(led_num, state);
			USART2_Send_String("LED");
			USART2_Send_Frame(&Cmd_Buf[3], 1);
			USART2_Send_String(" 已熄灭\r\n");
		}
		else
		{
			USART2_Send_String("错误: 状态必须是ON或OFF\r\n");
		}
	}
	else if(('B' == Cmd_Buf[0]) && ('E' == Cmd_Buf[1]) && ('E' == Cmd_Buf[2]) && ('P' == Cmd_Buf[3]))//BEEP指令
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);//蜂鸣器响
		delay_ms(100);
		GPIO_SetBits(GPIOA, GPIO_Pin_8);//蜂鸣器不响
		USART2_Send_String("蜂鸣器响一声\r\n");
	}
	else if(('H' == Cmd_Buf[0]) && ('E' == Cmd_Buf[1]) && ('L' == Cmd_Buf[2]) && ('P' == Cmd_Buf[3]))//HELP指令
	{
		USART2_Send_String("\r\n========== 帮助信息 ==========\r\n");
		USART2_Send_String("LED1 ON   - 点亮LED1\r\n");
		USART2_Send_String("LED1 OFF  - 熄灭LED1\r\n");
		USART2_Send_String("LED2 ON   - 点亮LED2\r\n");
		USART2_Send_String("LED2 OFF  - 熄灭LED2\r\n");
		USART2_Send_String("LED3 ON   - 点亮LED3\r\n");
		USART2_Send_String("LED3 OFF  - 熄灭LED3\r\n");
		USART2_Send_String("BEEP      - 蜂鸣器响一声\r\n");
		USART2_Send_String("HELP      - 显示此帮助\r\n");
		USART2_Send_String("===============================\r\n");
	}
	else if(Cmd_Len > 0)//未知指令
	{
		USART2_Send_String("未知指令! 输入HELP查看帮助\r\n");
	}
}

/******** LED控制函数 ********/
void LED_Control(u8 led_num, u8 state)
{
	switch(led_num)
	{
		case 1:
			if(state)
				GPIO_ResetBits(GPIOB, GPIO_Pin_5);//点亮LED1
			else
				GPIO_SetBits(GPIOB, GPIO_Pin_5);//熄灭LED1
			break;
		case 2:
			if(state)
				GPIO_ResetBits(GPIOB, GPIO_Pin_0);//点亮LED2
			else
				GPIO_SetBits(GPIOB, GPIO_Pin_0);//熄灭LED2
			break;
		case 3:
			if(state)
				GPIO_ResetBits(GPIOB, GPIO_Pin_1);//点亮LED3
			else
				GPIO_SetBits(GPIOB, GPIO_Pin_1);//熄灭LED3
			break;
		default:
			break;
	}
}

/******** UART2 帧发送函数 ********/
void USART2_Send_Frame(u8* data, u16 len)
{
	u16 i;
	for(i = 0; i < len; i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		USART_SendData(USART2, data[i]);
	}
}

/******** UART2 字符串发送函数 ********/
void USART2_Send_String(u8* str)
{
	while(*str)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		USART_SendData(USART2, *str++);
	}
}

/***** 简单延时函数 *****/
void delay_us(uint16_t nus)
{
	uint16_t i;
	while(nus--)
	{
		i = 31;
		while(i--);
	}
}

void delay_ms(u16 nms)
{
	uint16_t i;
	while(nms--)
	{
		i = 33800;
		while(i--);
	}
}

/*** 串口2中断处理函数 ***/
void USART2_IRQHandler(void)
{
	u8 res = 0;
	
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE))
	{
		res = USART_ReceiveData(USART2);
		
		if(OD_Flag)//已经接收到回车符的情况
		{
			if(res == 0x0A)//收到换行符，帧结束
			{
				Rx_Frame_Flag = 1;
				Rx_Len = Rx_Con;
				Rx_Con = 0;
				OD_Flag = 0;
			}
		}
		else//还没收到回车符的情况
		{
			if(res == 0x0D)//收到回车符
			{
				OD_Flag = 1;
			}
			else
			{
				Rx_Buf[Rx_Con] = res;
				Rx_Con++;
			}
		}
	}
}
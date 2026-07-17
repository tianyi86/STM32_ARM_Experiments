#include "stm32f4xx.h"
GPIO_InitTypeDef GPIO_InitStructure; //定义一个GPIO初始化用的结构体
/*定义GPIO、EXTI、NVIC初始化用到的结构体变量*/
GPIO_InitTypeDef  GPIO_InitStructure;
NVIC_InitTypeDef  NVIC_InitStructure;
EXTI_InitTypeDef  EXTI_InitStructure;

/*定义一个时间变量用于LED闪烁流水*/
u16 LedTime = 500;

/* 函数声明 */
void delay_us(uint16_t nus);
void delay_ms(u16 nms);
/* 函数声明 */
void delay_us(uint16_t nus);
void delay_ms(u16 nms);
//外部中断0服务程序
void EXTI0_IRQHandler(void)
{
   EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位
   LedTime = 100;//改变LED流水的时间（5倍速）
}

/***** 简单延时函数,ms,us *****/
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
int main(void)
{
   /*开启 GPIOB 的时钟，才能使用 GPIOB*/
   RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOB,  ENABLE ); //在AHB1上开启GPIOB的时钟
   /*配置三个LED所在的GPIO（PB5、PB0、PB1）*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1; //要初始化的引脚号（PB5、PB0、PB1）
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //GPIO设置为输出模式
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  //GPIO设置为推挽模式
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;  //速度设置为25MHz
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //没有上拉下拉电阻
  
   /*预先把输出设置为高电平，确保LED是初始化后是熄灭的*/
   GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1);
  
   /*使用上面配置的GPIO_InitStructure数据，初始化GPIOB*/
   GPIO_Init( GPIOB,  &GPIO_InitStructure );

   /*三个GPIO输出低电平，点亮三个LED*/
   GPIO_ResetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1);
  
   /*也可以逐个点亮，指令运行很快，两种方式看不出区别*/
//  GPIO_ResetBits(GPIOB, GPIO_Pin_5);
//  GPIO_ResetBits(GPIOB, GPIO_Pin_0);
//  GPIO_ResetBits(GPIOB, GPIO_Pin_1);
  
   /*如果三个GPIO输出高电平，熄灭三个LED*/
//  GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1);
   /*开启 GPIOA 的时钟，才能使用 GPIOA*/
   RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA,ENABLE); //AHB1上开启GPIOA时钟
  
   /*配置蜂鸣器所在的GPIO（PA8）*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //要初始化的引脚号
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //GPIO设置为输出模式
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  //GPIO设置为推挽模式
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;  //速度设置为25MHz
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //没有上拉下拉电阻
   GPIO_SetBits(GPIOA, GPIO_Pin_8);//预先设置为高电平，蜂鸣器是不响的
   GPIO_Init( GPIOA,  &GPIO_InitStructure );  //使用上述的数据，初始化GPIOB

   /*PA8输出低电平，蜂鸣器响*/
 //  GPIO_ResetBits(GPIOA, GPIO_Pin_8);
  
   /*如果PA8输出高电平，蜂鸣器停止鸣响*/
 GPIO_SetBits(GPIOA, GPIO_Pin_8);
 /*定义一个时间变量用于LED闪烁流水*/
u16 LedTime = 500;
/*初始化三个LED的GPIO*/
/*开启 GPIOB 的时钟，才能使用 GPIOB*/
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE );//在AHB1上开启GPIOB的时钟

/*配置三个LED所在的GPIO（PB5、PB0、PB1）*/
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1; //要初始化的引脚号（PB5、PB0、PB1）
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //GPIO设置为输出模式
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //GPIO设置为推挽模式
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz; //速度设置为25MHz
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; //没有上拉下拉电阻
/*使用上面配置的GPIO_InitStructure数据，初始化GPIOB*/
GPIO_Init( GPIOB,  &GPIO_InitStructure );//初始化GPIOB
/*初始化KEY1 PA0*/ 
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//开启GPIOA的时钟
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//开启SYSCFG的时钟

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //选择PA0
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //输入模式
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;  //25MHz速度
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉电阻
GPIO_Init(GPIOA, &GPIO_InitStructure);  //使用上述数据初始化PA0

SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);//PA0连接到中断线0

/* 配置EXTI_Line0 */
EXTI_InitStructure.EXTI_Line = EXTI_Line0; //选择中断线0
EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //中断模式
EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
EXTI_InitStructure.EXTI_LineCmd = ENABLE; //使能中断线
EXTI_Init(&EXTI_InitStructure); //使用上述数据初始化EXTI

NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; //设置中断向量
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级0
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02; //响应优先级2
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //开启使能
NVIC_Init(&NVIC_InitStructure); //初始化NVIC
   while(1)
   {
       GPIO_ResetBits(GPIOB, GPIO_Pin_5); //点亮LED1
    GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1); //熄灭LED2，3

    delay_ms(LedTime); //可变延时
        
    GPIO_ResetBits(GPIOB, GPIO_Pin_0); //点亮LED2
    GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_1); //熄灭LED1，3

    delay_ms(LedTime); //可变延时
        
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); //点亮LED3
    GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_0); //熄灭LED1，2

    delay_ms(LedTime); //可变延时
   }
}
#include <stdio.h>
#include "stm32f4xx.h"
#include "../UTILS/delay.h"
#define IR_RECV_TIM   TIM9

uint8_t IRState  = 0;
#define IR_STATE_LEAD_OK  0x80
#define IR_STATE_DATA_OK  0x40
#define IR_STATE_UP_OK    0x10

uint32_t  CapValue = 0;
uint32_t  IRCount  = 0;
uint32_t  IRData   = 0;

void IR_GPIO_Init(void);
void IR_TIM_Init(void);
// 声明中断函数，供it文件调用

void IR_Recv_Init(){
    IR_GPIO_Init();
    IR_TIM_Init();
}

void IR_GPIO_Init(void){
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_TIM9);
}

void IR_TIM_Init(void){
    NVIC_InitTypeDef NVIC_InitStruct;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStruct;
    TIM_ICInitTypeDef  TIM_ICInitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9,ENABLE);

    TIM_TimeBaseStruct.TIM_Period    = 10000;
    TIM_TimeBaseStruct.TIM_Prescaler = 167;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(IR_RECV_TIM, &TIM_TimeBaseStruct);

    TIM_ICInitStruct.TIM_Channel     = TIM_Channel_1;
    TIM_ICInitStruct.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStruct.TIM_ICFilter    = 0x03;
    TIM_ICInit(IR_RECV_TIM, &TIM_ICInitStruct);

    TIM_ITConfig( TIM9, TIM_IT_Update | TIM_IT_CC1, ENABLE);
    TIM_Cmd(IR_RECV_TIM, ENABLE );

    NVIC_InitStruct.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

//键码打印弱函数，可重写
__attribute__((weak)) void IR_Rece_Proc(uint16_t addr, uint8_t code ){
    printf("IRRecv:addr:%d, code:%d\r\n", addr, code);
}

//红外数据解析任务函数，main循环调用
void IR_Recv( void ) {
    uint16_t addr = 0;
    uint8_t byte1,byte2,byte3,byte4 = 0;

    if( !(IRState & IR_STATE_DATA_OK) ) {
        return;
    }

    IRState &= ~IR_STATE_DATA_OK;

    byte1 = IRData;
    byte2 = IRData >> 8;
    byte3 = IRData >> 16;
    byte4 = IRData >> 24;
    IRData = 0;

    if(byte3 != (uint8_t)~byte4){ return; }

    if(byte1 != (uint8_t)~byte2){
        addr = (byte2 << 8) | byte1;
    }
    else{
        addr = byte1;
    }

    IR_Rece_Proc(addr, byte3);
}
//TIM9输入捕获+溢出中断服务函数
void TIM1_BRK_TIM9_IRQHandler(){
    if(TIM_GetITStatus(IR_RECV_TIM, TIM_IT_CC1) != RESET){
        TIM_ClearITPendingBit(IR_RECV_TIM, TIM_IT_CC1);
        TIM_SetCounter(IR_RECV_TIM, 0);

        if( !(IRState & IR_STATE_UP_OK) ){
            IRState |= IR_STATE_UP_OK;
            return;
        }

        CapValue = TIM_GetCapture1(IR_RECV_TIM) - 560;

        if(CapValue>300 && CapValue<800){
            IRData >>= 1;
        }
        else if(CapValue>1400 && CapValue<1800){
            IRData >>= 1;
            IRData |=  0x80000000;
        }
        else if(CapValue>2200 && CapValue<2600){
            IRCount++;
        }
        else if(CapValue>4200 && CapValue<5000){
            IRData  = 0 ;
            IRState |= IR_STATE_LEAD_OK;
            IRCount = 0;
        }
    }

    if(TIM_GetITStatus(IR_RECV_TIM, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(IR_RECV_TIM, TIM_IT_Update);

        if( IRState & IR_STATE_LEAD_OK ) {
            IRState &= ~IR_STATE_UP_OK;
            IRState &= ~IR_STATE_LEAD_OK;
            printf("IRRecv:[%X]\r\n", IRData);
            IRState |= IR_STATE_DATA_OK ;
        }
    }
}
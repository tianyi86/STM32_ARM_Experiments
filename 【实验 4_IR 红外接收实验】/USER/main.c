#include <stdio.h>
#include "stm32f4xx.h"
#include "../BSP/usart.h"
#include "../BSP/ir_recv.h"
#include "../BSP/ir_send.h"

#define DELAY_MS_COUNT  400  // 修改为400，缩短等待

int main( void ){
    uint32_t i = 0;
    uint32_t uMSCount = 0;
    
    //Systick初始化，给发送微秒延时使用
    SysTick_Config(SystemCoreClock/1000);
    
    UART2_Init(115200);
    IR_Recv_Init();
    printf("IR Recv Init OK\r\n");
    IR_Send_Init();
    printf("IR Send Init OK\r\n");

    while(1){
        //循环处理红外接收解码
        IR_Recv();
        
        //每1000次计数≈1秒发送一次固定码 addr=1 code=123
        if(uMSCount >= 1000){ 
            printf("IRSend:addr:1,code:123\r\n");
            IR_Send(1, 123);
            uMSCount = 0;
        }

        //简易毫秒计数
        if(i >= DELAY_MS_COUNT){
            uMSCount++;
            i = 0;
        }
        else{
            i++;
        }
    }
}
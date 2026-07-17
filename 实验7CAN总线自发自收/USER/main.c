#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "can.h"

int main(void)
{
    uint8_t pTxData[8] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    SysTick_Init();     // 初始化滴答定时器，提供延时
    UART2_Init(115200); // 初始化调试串口，printf打印
    CAN1_Init();        // 初始化CAN1，自发自收模式

    while(1)
    {
        CAN1_SendMsg(0x1234, pTxData, 8);
        delay_ms(5000); // 5秒发送一次数据
    }
}
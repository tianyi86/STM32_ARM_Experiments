#include <stdio.h>
#include "stm32f4xx.h"
#include "dht11.h"
#include "usart.h"
#include "delay.h"
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStruct);
    GPIO_SetBits(GPIOF, GPIO_Pin_9);
}
// ========== 在这里修改 ==========
#define LED_PORT    GPIOF        // 改这里：LED使用的端口
#define LED_PIN     GPIO_Pin_9   // 改这里：LED使用的引脚
#define TEMP_ALARM  30           // 改这里：报警温度（单位：°C）
#define ALARM_FLASH_COUNT  10    // 改这里：报警闪烁次数
/**
* @brief 主函数
* @param 无
* @retval 无
*/
int main( void ){
DHT11_Data Data;

UART2_Init(115200);
DHT11_Init();
	LED_Init();
    
  
    
printf("DHT11 Init OK\r\n");

delay_ms(1000);

while(1){
if(DHT11_ReadData( & Data ) == 0){
	float temp = Data.temp_int + Data.temp_deci * 0.1f;
	if(temp >= 25)  // 30°C报警，可以改这个数字
        {
            // LED闪烁报警
            for(int i=0; i<10; i++)  // 闪10次
            {
                GPIO_ResetBits(GPIOF, GPIO_Pin_9);  // 亮
                delay_ms(150);
                GPIO_SetBits(GPIOF, GPIO_Pin_9);    // 灭
                delay_ms(150);
            }
        }
        else
        {
            GPIO_SetBits(GPIOF, GPIO_Pin_9);  // 温度正常→熄灭LED
        }
printf("HUMI:%d.%d_TEMP:%d.%d\r\n",\
Data.humi_int,Data.humi_deci,Data.temp_int,Data.temp_deci);
}
else {
printf("Read DHT11 ERROR!\r\n");
}
delay_ms(10000);
}
}
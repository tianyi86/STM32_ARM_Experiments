#include <stdio.h>
#include "stm32f4xx.h"
#include "../BSP/e2prom_at24c02.h"
#include "../BSP/usart.h"
#include "../UTILS/delay.h"

#define TEST_SIZE 256
uint8_t BufRead[TEST_SIZE]  ={0};
uint8_t BufWrite[TEST_SIZE] ={0};

//生成0~255测试数据
void make_test_data(){
    uint32_t i = 0;
    for(i=0; i<TEST_SIZE; i++ ){
        BufWrite[i] = i;
        printf("0x%02X ", BufWrite[i]);
        if(i%16 == 15){
            printf("\r\n");
        }
    }
    printf("\r\n");
}

//读出数据和写入数据对比校验
void check_test_data(){
    uint32_t i = 0;
    for(i=0; i<TEST_SIZE; i++ ){
        if(BufRead[i] != BufWrite[i]){
            printf("地址%d读取错误：写入0x%02X，读出0x%02X 测试失败\r\n",i,BufWrite[i],BufRead[i]);
            break;
        }
        printf("0x%02X ", BufRead[i]);
        if(i%16 == 15){
           printf("\r\n");
        }
    }
    printf("\r\n");
    if(i >= TEST_SIZE){
        printf("======== 全部数据匹配，E2PROM读写测试OK ========\r\n");
    }
}

int main(void){
    //1. 初始化系统滴答延时
    SysTick_Config(SystemCoreClock/1000);
    //2. 优先初始化串口，立刻打印，判断程序有没有跑起来
    UART2_Init(115200);
    printf("【第一步】串口初始化成功，程序已进入main\r\n");
    
    //3. I2C AT24C02初始化（PB6 PB7、AT24C02供电接线正确才不会卡死）
    AT24C02_Init();
    printf("【第二步】AT24C02 E2PROM初始化完成\r\n");
    
    //1、生成待写入数据并打印
    printf("\r\n===== 生成写入数据 =====\r\n");
    make_test_data();
    
    //2、将256字节全部写入AT24C02（起始地址0）
    printf("\r\n===== 正在写入E2PROM =====\r\n");
    AT24C02_BufferWrite(BufWrite, 0, TEST_SIZE);
    printf("写入完成\r\n");

    //3、从E2PROM读取全部256字节
    printf("\r\n===== 读取E2PROM数据 =====\r\n");
    AT24C02_BufferRead(BufRead, 0, TEST_SIZE);
    
    //4、校验读写数据
    printf("\r\n===== 读写数据对比校验 =====\r\n");
    check_test_data();
  
    //测试完成，循环待机
    while(1){
        systick_delay_ms(500);
        printf("测试结束，空闲循环\r\n");
    }
}
#include "usart.h"

int fputc(int ch, FILE *f)
{
    USART_SendData(USART2, (uint8_t)ch);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    return ch;
}

int fgetc(FILE *f)
{
    while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
    return (int)USART_ReceiveData(USART2);
}

void UART2_Init(uint32_t baud)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    USART_DeInit(USART2);
    usart.USART_BaudRate = baud;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);
}
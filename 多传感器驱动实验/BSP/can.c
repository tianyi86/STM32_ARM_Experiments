#include "can.h"

void CAN1_Init(void)
{
    GPIO_InitTypeDef gpio;
    CAN_InitTypeDef can;
    CAN_FilterInitTypeDef filter;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    // PA11 PA12 CAN收发引脚
    gpio.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);

    CAN_DeInit(CAN1);
    can.CAN_TTCM = DISABLE;
    can.CAN_ABOM = DISABLE;
    can.CAN_AWUM = DISABLE;
    can.CAN_NART = DISABLE;
    can.CAN_RFLM = DISABLE;
    can.CAN_TXFP = DISABLE;
    can.CAN_Mode = CAN_Mode_Normal; // 替换掉不存在的SelfTest，普通模式
    can.CAN_SJW = CAN_SJW_1tq;
    can.CAN_BS1 = CAN_BS1_10tq;
    can.CAN_BS2 = CAN_BS2_5tq;
    can.CAN_Prescaler = 4;
    CAN_Init(CAN1, &can);

    filter.CAN_FilterNumber = 0;
    filter.CAN_FilterMode = CAN_FilterMode_IdMask;
    filter.CAN_FilterScale = CAN_FilterScale_32bit;
    filter.CAN_FilterIdHigh = 0x0000;
    filter.CAN_FilterIdLow = 0x0000;
    filter.CAN_FilterMaskIdHigh = 0x0000;
    filter.CAN_FilterMaskIdLow = 0x0000;
    filter.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    filter.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&filter);
}

void CAN1_SendMsg(uint32_t id, uint8_t *buf, uint8_t len)
{
    CanTxMsg tx;
    tx.StdId = id;
    tx.ExtId = 0;
    tx.IDE = CAN_ID_STD;
    tx.RTR = CAN_RTR_DATA;
    tx.DLC = len;
    for(uint8_t i=0; i<len; i++) tx.Data[i] = buf[i];
    CAN_Transmit(CAN1, &tx);
}
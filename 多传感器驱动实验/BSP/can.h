#ifndef __CAN_H
#define __CAN_H
#include "stm32f4xx.h"

void CAN1_Init(void);
void CAN1_SendMsg(uint32_t id, uint8_t *buf, uint8_t len);

#endif
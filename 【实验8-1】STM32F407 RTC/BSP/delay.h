#ifndef _DELAY_H_
#define _DELAY_H_
#include "stm32f4xx.h"  //关键！没有这行 uint16_t 直接报红

void delay_us(uint16_t nus);
void delay_ms(u16 nms);

#endif
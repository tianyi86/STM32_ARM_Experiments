#ifndef __DELAY_H__
#define __DELAY_H__
#include "stm32f4xx.h"

void systick_delay_us(uint32_t us);
void systick_delay_ms(uint32_t ms);

#endif
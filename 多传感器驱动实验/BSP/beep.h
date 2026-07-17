#ifndef __BEEP_H__
#define __BEEP_H__

#include "stm32f4xx.h"

#define BEEP_PIN    GPIO_Pin_8   // Buzzer -> PA8
#define BEEP_PORT   GPIOA

#define BEEP_ON()   GPIO_ResetBits(BEEP_PORT, BEEP_PIN)   // PA8低电平 → 蜂鸣器响
#define BEEP_OFF()  GPIO_SetBits(BEEP_PORT, BEEP_PIN)     // PA8高电平 → 蜂鸣器不响

void BEEP_Init(void);

#endif

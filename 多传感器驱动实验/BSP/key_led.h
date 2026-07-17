#ifndef __KEY_LED_H
#define __KEY_LED_H
#include "stm32f4xx.h"
#include "delay.h"

#define KEY_PIN    GPIO_Pin_0   // KEY1 ¡ú PA0
#define KEY_PORT   GPIOA
#define LED_PIN    GPIO_Pin_5   // LED1 ¡ú PB5
#define LED_PORT   GPIOB

void KEY_LED_Init(void);
uint8_t KEY_Scan(void);
#endif

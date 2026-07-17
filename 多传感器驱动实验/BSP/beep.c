#include "beep.h"

void BEEP_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin   = BEEP_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_25MHz;
    gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(BEEP_PORT, &gpio);

    BEEP_OFF();  // 初始不响
}

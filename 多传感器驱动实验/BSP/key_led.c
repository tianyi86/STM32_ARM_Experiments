#include "key_led.h"

void KEY_LED_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = LED_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_25MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LED_PORT, &gpio);
    GPIO_SetBits(LED_PORT, LED_PIN);

    gpio.GPIO_Pin = KEY_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY_PORT, &gpio);
}

uint8_t KEY_Scan(void)
{
    if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == 0)
    {
        delay_ms(20);
        if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == 0)
        {
            while(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == 0);
            delay_ms(20);
            return 1;
        }
    }
    return 0;
}

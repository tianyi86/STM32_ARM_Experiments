#include "delay.h"

static uint32_t fac_us = 0;
static uint32_t fac_ms = 0;

void SysTick_Init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);  // 168MHz/8 = 21MHz
    fac_us = SystemCoreClock / 8000000;   // 21
    fac_ms = fac_us * 1000;               // 21000
}

void delay_us(uint16_t nus)
{
    uint32_t temp;
    SysTick->LOAD = (uint32_t)nus * fac_us;
    SysTick->VAL  = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL  = 0x00;
}

void delay_ms(uint16_t nms)
{
    while (nms > 0)
    {
        uint16_t seg = (nms > 500) ? 500 : nms;
        delay_us((uint16_t)(seg * 1000));
        nms -= seg;
    }
}

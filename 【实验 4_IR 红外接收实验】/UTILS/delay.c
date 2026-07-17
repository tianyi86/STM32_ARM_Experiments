#include "delay.h"

void systick_delay_us(uint32_t us){
    uint32_t ticks = 0;
    uint32_t told,tnow,reload,tcnt = 0;

    reload = SysTick->LOAD;
    ticks  = us * (SystemCoreClock / 1000000);
    told   = SysTick->VAL;

    while( 1 ){
        tnow = SysTick->VAL;
        if(tnow == told){
            continue ;
        }

        if(tnow < told){
            tcnt += told - tnow;
        }
        else {
            tcnt += reload - tnow + told;
        }

        told = tnow;
        if(tcnt >= ticks) { break; }
    }
}
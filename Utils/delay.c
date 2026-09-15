#include "delay.h"

void TimerInitialize(TimerHandle *t, TIM_TypeDef *TIMx)
{
    t->TIMx = TIMx;
}

void TimerDelayUs(TimerHandle *t, uint16_t us)
{
    uint16_t start = LL_TIM_GetCounter(t->TIMx);

    while ((uint16_t)(LL_TIM_GetCounter(t->TIMx) - start) < us) {
    }
}

void TimerDelayMs(TimerHandle *t, uint16_t ms)
{
    while (ms--) {
        TimerDelayUs(t, 1000);
    }
}
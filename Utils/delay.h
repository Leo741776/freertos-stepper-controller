#ifndef DELAY_H
#define DELAY_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    TIM_TypeDef *TIMx;
} TimerHandle;

void TimerInitialize(TimerHandle *t, TIM_TypeDef *TIMx);
void TimerDelayUs(TimerHandle *t, uint16_t us);
void TimerDelayMs(TimerHandle *t, uint16_t ms);

#endif
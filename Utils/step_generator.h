#ifndef STEP_GENERATOR_H
#define STEP_GENERATOR_H

#include "main.h"
#include "A4988.h"
#include "delay.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    TIM_TypeDef *TIMx;
    A4988Handle *motor;
    TimerHandle *delay_timer;
    volatile bool running;
    volatile uint32_t step_count;
} StepGeneratorHandle;

void StepGeneratorInitialize(
    StepGeneratorHandle *g,
    TIM_TypeDef *TIMx,
    A4988Handle *motor,
    TimerHandle *delay_timer
);

void StepGeneratorSetIntervalUs(
    StepGeneratorHandle *g,
    uint32_t interval_us
);

void StepGeneratorStart(StepGeneratorHandle *g);
void StepGeneratorStop(StepGeneratorHandle *g);
void StepGeneratorIRQHandler(StepGeneratorHandle *g);

#endif
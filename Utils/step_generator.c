#include "step_generator.h"

void StepGeneratorInitialize(
    StepGeneratorHandle *g,
    TIM_TypeDef *TIMx,
    A4988Handle *motor,
    TimerHandle *delay_timer
)
{
    g->TIMx = TIMx;
    g->motor = motor;
    g->delay_timer = delay_timer;
    g->running = false;
    g->step_count = 0;
}

void StepGeneratorSetIntervalUs(
    StepGeneratorHandle *g,
    uint32_t interval_us
)
{
    if (interval_us == 0) {
        return;
    }

    LL_TIM_SetAutoReload(g->TIMx, interval_us - 1);
}

void StepGeneratorStart(StepGeneratorHandle *g)
{
    if (!g->running) {
        LL_TIM_EnableIT_UPDATE(g->TIMx);
        LL_TIM_EnableCounter(g->TIMx);
        g->running = true;
    }
}

void StepGeneratorStop(StepGeneratorHandle *g)
{
    LL_TIM_DisableCounter(g->TIMx);
    LL_TIM_DisableIT_UPDATE(g->TIMx);
    g->running = false;
}

void StepGeneratorIRQHandler(StepGeneratorHandle *g)
{
    if (LL_TIM_IsActiveFlag_UPDATE(g->TIMx)) {
        LL_TIM_ClearFlag_UPDATE(g->TIMx);
        A4988Step(g->motor, g->delay_timer);
        g->step_count++;
    }
}
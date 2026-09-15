#include "A4988.h"

bool A4988Initialize(A4988Handle *a) {
    switch (a->direction) {
        case DIR_CW:  break;
        case DIR_CCW: break;
        default: return false;
    }

    switch (a->microstep) {
        case STEP_FULL:      break;
        case STEP_HALF:      break;
        case STEP_QUARTER:   break;
        case STEP_EIGHTH:    break;
        case STEP_SIXTEENTH: break;
        default: return false;
    }

    A4988SetDirection(a, a->direction);
    A4988SetMicrostep(a, a->microstep);
    a->stop_requested = false;

    return true;
}

bool A4988SetDirection(A4988Handle *a, A4988Direction direction_mode) {
    switch (direction_mode) {
        case DIR_CW:  LL_GPIO_SetOutputPin(a->direction_port, a->direction_pin);   break;
        case DIR_CCW: LL_GPIO_ResetOutputPin(a->direction_port, a->direction_pin); break;
        default: return false;
    }

    a->direction = direction_mode;

    return true;
}

bool A4988SetMicrostep(A4988Handle *a, A4988Microstep microstep_mode) {
    switch (microstep_mode) {
        case STEP_FULL:
            LL_GPIO_ResetOutputPin(a->ms1_port, a->ms1_pin);
            LL_GPIO_ResetOutputPin(a->ms2_port, a->ms2_pin);
            LL_GPIO_ResetOutputPin(a->ms3_port, a->ms3_pin);
            break;

        case STEP_HALF:
            LL_GPIO_SetOutputPin(a->ms1_port, a->ms1_pin);
            LL_GPIO_ResetOutputPin(a->ms2_port, a->ms2_pin);
            LL_GPIO_ResetOutputPin(a->ms3_port, a->ms3_pin);
            break;

        case STEP_QUARTER:
            LL_GPIO_ResetOutputPin(a->ms1_port, a->ms1_pin);
            LL_GPIO_SetOutputPin(a->ms2_port, a->ms2_pin);
            LL_GPIO_ResetOutputPin(a->ms3_port, a->ms3_pin);
            break;

        case STEP_EIGHTH:
            LL_GPIO_SetOutputPin(a->ms1_port, a->ms1_pin);
            LL_GPIO_SetOutputPin(a->ms2_port, a->ms2_pin);
            LL_GPIO_ResetOutputPin(a->ms3_port, a->ms3_pin);
            break;

        case STEP_SIXTEENTH:
            LL_GPIO_SetOutputPin(a->ms1_port, a->ms1_pin);
            LL_GPIO_SetOutputPin(a->ms2_port, a->ms2_pin);
            LL_GPIO_SetOutputPin(a->ms3_port, a->ms3_pin);
            break;

        default: return false;
    }

    a->microstep = microstep_mode;

    return true;
}

bool A4988SetSpeedRPM(A4988Handle *a, A4988SpeedRPM speed_rpm_mode) {
    switch (speed_rpm_mode) {
        case SPEED_60_RPM:  break;
        case SPEED_100_RPM: break;
        case SPEED_200_RPM: break;
        case SPEED_300_RPM: break;
        default: return false;
    }

    a->speed_rpm = speed_rpm_mode;

    return true;
}

void A4988Step(A4988Handle *a, TimerHandle *t) {
    LL_GPIO_SetOutputPin(a->step_port, a->step_pin);
    TimerDelayUs(t, 2);
    LL_GPIO_ResetOutputPin(a->step_port, a->step_pin);
}

uint32_t A4988GetStepIntervalUs(A4988Handle *a) {
    uint32_t steps_per_rev = 0;

    switch (a->microstep) {
        case STEP_FULL:      steps_per_rev = 200;  break;
        case STEP_HALF:      steps_per_rev = 400;  break;
        case STEP_QUARTER:   steps_per_rev = 800;  break;
        case STEP_EIGHTH:    steps_per_rev = 1600; break;
        case STEP_SIXTEENTH: steps_per_rev = 3200; break;
        default: return 0;
    }

    if (a->speed_rpm == 0) {
        return 0;
    }

    return 60000000UL / ((uint32_t)a->speed_rpm * steps_per_rev);
}
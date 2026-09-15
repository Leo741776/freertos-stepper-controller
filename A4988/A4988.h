#ifndef A4988_H
#define A4988_H

#include "delay.h"
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    DIR_CW  = 0,
    DIR_CCW = 1
} A4988Direction;

typedef enum {
    STEP_FULL      = 0b000,
    STEP_HALF      = 0b001,
    STEP_QUARTER   = 0b010,
    STEP_EIGHTH    = 0b011,
    STEP_SIXTEENTH = 0b111
} A4988Microstep;

typedef enum {
    SPEED_60_RPM  = 60,
    SPEED_100_RPM = 100,
    SPEED_200_RPM = 200,
    SPEED_300_RPM = 300
} A4988SpeedRPM;

typedef struct {
    GPIO_TypeDef   *step_port;
    GPIO_TypeDef   *direction_port;
    GPIO_TypeDef   *ms1_port;
    GPIO_TypeDef   *ms2_port;
    GPIO_TypeDef   *ms3_port;
    uint16_t        step_pin;
    uint16_t        direction_pin;
    uint16_t        ms1_pin;
    uint16_t        ms2_pin;
    uint16_t        ms3_pin;
    A4988Direction  direction;
    A4988Microstep  microstep;
    A4988SpeedRPM   speed_rpm;
    volatile bool   stop_requested;
} A4988Handle;

bool     A4988Initialize(A4988Handle *a);
bool     A4988SetDirection(A4988Handle *a, A4988Direction direction_mode);
bool     A4988SetMicrostep(A4988Handle *a, A4988Microstep microstep_mode);
bool     A4988SetSpeedRPM(A4988Handle *a, A4988SpeedRPM speed_rpm_mode);
void     A4988Step(A4988Handle *a, TimerHandle *t);
uint32_t A4988GetStepIntervalUs(A4988Handle *a);

#endif
#ifndef MOTION_COMMAND_H
#define MOTION_COMMAND_H

#include "A4988.h"
#include <stdint.h>

typedef struct {
    A4988Direction direction;
    A4988Microstep microstep;
    A4988SpeedRPM speed_rpm;
    uint32_t target_steps;
} MotionCommand;

#endif
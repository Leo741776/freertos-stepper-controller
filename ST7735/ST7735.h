#ifndef ST7735_H
#define ST7735_H

#include "motion_command.h"
#include "A4988.h"

void ST7735Init(void);

/* Update this prototype to 2 parameters */
void ST7735Update(const MotionCommand *cmd, A4988Handle *motor);

#endif
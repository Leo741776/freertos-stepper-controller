#ifndef CONTROLS_H
#define CONTROLS_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

bool button_pressed(GPIO_TypeDef *port, uint32_t pin, bool *was_pressed);

#endif
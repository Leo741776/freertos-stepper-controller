#include "controls.h"

bool button_pressed(GPIO_TypeDef *port, uint32_t pin, bool *was_pressed)
{
    bool is_pressed = (LL_GPIO_IsInputPinSet(port, pin) == 0);
    bool just_pressed = is_pressed && !(*was_pressed);

    *was_pressed = is_pressed;

    return just_pressed;
}
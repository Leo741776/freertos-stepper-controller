#ifndef INT_STRING_H
#define INT_STRING_H

#include <stdint.h>

void uint16_to_str(uint16_t value, char *buffer);
int uint32_to_str(uint32_t value, char *buffer);
void float_to_str(float value, char *buffer, int decimals);

#endif
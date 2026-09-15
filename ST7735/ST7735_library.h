#ifndef ST7735_LIBRARY_H
#define ST7735_LIBRARY_H

#include "stm32c0xx.h"
#include "stm32c0xx_ll_bus.h"
#include "stm32c0xx_ll_rcc.h"
#include "stm32c0xx_ll_gpio.h"
#include "stm32c0xx_ll_system.h"
#include "fonts.h"

#include <stdbool.h>

#define CS_PORT GPIOB
#define CS_PIN LL_GPIO_PIN_9

#define DC_PORT GPIOB
#define DC_PIN LL_GPIO_PIN_7

#define RST_PORT GPIOB
#define RST_PIN LL_GPIO_PIN_8

#define ST7735_X_OFFSET 2
#define ST7735_Y_OFFSET 1

#define ST7735_IS_160X128 1

#define ST7735_WIDTH 128
#define ST7735_HEIGHT 160

#define DELAY 0x80

#define ST7735_MADCTL_MY 0x80
#define ST7735_MADCTL_MX 0x40
#define ST7735_MADCTL_MV 0x20
#define ST7735_MADCTL_ML 0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_NOP 0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID 0x04
#define ST7735_RDDST 0x09
#define ST7735_SLPIN 0x10
#define ST7735_SLPOUT 0x11
#define ST7735_PTLON 0x12
#define ST7735_NORON 0x13
#define ST7735_INVOFF 0x20
#define ST7735_INVON 0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E
#define ST7735_PTLAR 0x30
#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6
#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5
#define ST7735_RDID1 0xDA
#define ST7735_RDID2 0xDB
#define ST7735_RDID3 0xDC
#define ST7735_RDID4 0xDD
#define ST7735_PWCTR6 0xFC
#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define color565(r, g, b) \
    (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

void ST7735_Init(uint8_t rotation);
void ST7735_SetRotation(uint8_t m);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_WriteString(
    uint16_t x,
    uint16_t y,
    const char *str,
    FontDef font,
    uint16_t color,
    uint16_t bgcolor
);
void ST7735_FillRectangle(
    uint16_t x,
    uint16_t y,
    uint16_t w,
    uint16_t h,
    uint16_t color
);
void ST7735_DrawImage(
    uint16_t x,
    uint16_t y,
    uint16_t w,
    uint16_t h,
    const uint16_t *data
);
void ST7735_InvertColors(bool invert);
void ST7735_draw_column(
    uint8_t x,
    uint8_t y_start,
    uint8_t y_end,
    uint16_t color
);

static inline void ST7735_Select(void)
{
    LL_GPIO_ResetOutputPin(CS_PORT, CS_PIN);
}

static inline void ST7735_Unselect(void)
{
    LL_GPIO_SetOutputPin(CS_PORT, CS_PIN);
}

#endif
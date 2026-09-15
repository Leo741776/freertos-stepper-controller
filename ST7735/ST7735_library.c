#include "ST7735_library.h"
#include <stddef.h>

int16_t _width, _height, cursor_x, cursor_y;
uint8_t rotation, _colstart, _rowstart, _xstart, _ystart;

static const uint8_t init_cmds1[] = {
    14,
    ST7735_SWRESET, DELAY, 150,
    ST7735_SLPOUT, DELAY, 255,
    ST7735_FRMCTR1, 3, 0x01, 0x2C, 0x2D,
    ST7735_FRMCTR2, 3, 0x01, 0x2C, 0x2D,
    ST7735_FRMCTR3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
    ST7735_INVCTR, 1, 0x07,
    ST7735_PWCTR1, 3, 0xA2, 0x02, 0x84,
    ST7735_PWCTR2, 1, 0xC5,
    ST7735_PWCTR3, 2, 0x0A, 0x00,
    ST7735_PWCTR4, 2, 0x8A, 0x2A,
    ST7735_PWCTR5, 2, 0x8A, 0xEE,
    ST7735_VMCTR1, 1, 0x0E,
    ST7735_INVOFF, 0,
    ST7735_COLMOD, 1, 0x05
};

#if defined(ST7735_IS_128X128)

static const uint8_t init_cmds2[] = {
    2,
    ST7735_CASET, 4, 0x00, 0x00, 0x00, 0x7F,
    ST7735_RASET, 4, 0x00, 0x00, 0x00, 0x7F
};

#elif defined(ST7735_IS_160X128)

static const uint8_t init_cmds2[] = {
    2,
    ST7735_CASET, 4, 0x00, 0x00, 0x00, 0x9F,
    ST7735_RASET, 4, 0x00, 0x00, 0x00, 0x7F
};

#elif defined(ST7735_IS_160X80)

static const uint8_t init_cmds2[] = {
    3,
    ST7735_CASET, 4, 0x00, 0x00, 0x00, 0x4F,
    ST7735_RASET, 4, 0x00, 0x00, 0x00, 0x9F,
    ST7735_INVON, 0
};

#endif

static const uint8_t init_cmds3[] = {
    4,
    ST7735_GMCTRP1, 16,
    0x02, 0x1c, 0x07, 0x12,
    0x37, 0x32, 0x29, 0x2d,
    0x29, 0x25, 0x2B, 0x39,
    0x00, 0x01, 0x03, 0x10,

    ST7735_GMCTRN1, 16,
    0x03, 0x1d, 0x07, 0x06,
    0x2E, 0x2C, 0x29, 0x2D,
    0x2E, 0x2E, 0x37, 0x3F,
    0x00, 0x00, 0x02, 0x10,

    ST7735_NORON, DELAY, 10,
    ST7735_DISPON, DELAY, 100
};

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile uint32_t j = 0; j < 4000; j++);
    }
}

void ST7735_Reset(void)
{
    LL_GPIO_ResetOutputPin(RST_PORT, RST_PIN);
    delay_ms(10);
    LL_GPIO_SetOutputPin(RST_PORT, RST_PIN);
    delay_ms(120);
}

static void SPI_TransmitByte(uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE));

    *(volatile uint8_t *)&SPI1->DR = data;
}

void ST7735_WriteCommand(uint8_t cmd)
{
    LL_GPIO_ResetOutputPin(DC_PORT, DC_PIN);

    SPI_TransmitByte(cmd);

    while (SPI1->SR & SPI_SR_BSY);
}

void ST7735_WriteData(uint8_t *buff, size_t size)
{
    LL_GPIO_SetOutputPin(DC_PORT, DC_PIN);

    for (size_t i = 0; i < size; i++) {
        SPI_TransmitByte(buff[i]);
    }

    while (SPI1->SR & SPI_SR_BSY);
}

void DisplayInit(const uint8_t *addr)
{
    uint8_t cmds = *addr++;

    while (cmds--) {
        uint8_t cmd = *addr++;

        ST7735_WriteCommand(cmd);

        uint8_t args = *addr++;
        uint16_t ms = args & DELAY;

        args &= ~DELAY;

        if (args) {
            ST7735_WriteData((uint8_t *)addr, args);
            addr += args;
        }

        if (ms) {
            ms = *addr++;
            delay_ms(ms == 255 ? 500 : ms);
        }
    }
}

void ST7735_SetAddressWindow(
    uint8_t x0,
    uint8_t y0,
    uint8_t x1,
    uint8_t y1
)
{
    uint8_t data[] = {
        0x00,
        x0 + _xstart,
        0x00,
        x1 + _xstart
    };

    ST7735_WriteCommand(ST7735_CASET);
    ST7735_WriteData(data, sizeof(data));

    ST7735_WriteCommand(ST7735_RASET);

    data[1] = y0 + _ystart;
    data[3] = y1 + _ystart;

    ST7735_WriteData(data, sizeof(data));

    ST7735_WriteCommand(ST7735_RAMWR);
}

void ST7735_Init(uint8_t rot)
{
    ST7735_Select();

    ST7735_Reset();
    DisplayInit(init_cmds1);
    DisplayInit(init_cmds2);
    DisplayInit(init_cmds3);

#if defined(ST7735_IS_160X80)

    _colstart = 24;
    _rowstart = 0;

    uint8_t data = 0xC0;

    ST7735_WriteCommand(ST7735_MADCTL);
    ST7735_WriteData(&data, 1);

#elif defined(ST7735_IS_128X128)

    _colstart = 2;
    _rowstart = 3;

#else

    _colstart = 0;
    _rowstart = 0;

#endif

    ST7735_SetRotation(rot);

    ST7735_Unselect();
}

void ST7735_SetRotation(uint8_t m)
{
    rotation = m % 4;

    uint8_t madctl = 0;

#if defined(ST7735_IS_160X80)

    const uint8_t madctl_tbl[] = {
        ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_BGR,
        ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR,
        ST7735_MADCTL_BGR,
        ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR
    };

    madctl = madctl_tbl[rotation];

#else

    const uint8_t rgb_flg = ST7735_MADCTL_RGB;

    switch (rotation) {
    case 0:
        madctl = ST7735_MADCTL_MX |
                 ST7735_MADCTL_MY |
                 rgb_flg;

        _width = ST7735_WIDTH;
        _height = ST7735_HEIGHT;
        _xstart = _colstart;
        _ystart = _rowstart;
        break;

    case 1:
        madctl = ST7735_MADCTL_MY |
                 ST7735_MADCTL_MV |
                 rgb_flg;

        _width = ST7735_HEIGHT;
        _height = ST7735_WIDTH;
        _ystart = _colstart;
        _xstart = _rowstart;
        break;

    case 2:
        madctl = rgb_flg;

        _width = ST7735_WIDTH;
        _height = ST7735_HEIGHT;
        _xstart = _colstart;
        _ystart = _rowstart;
        break;

    case 3:
        madctl = ST7735_MADCTL_MX |
                 ST7735_MADCTL_MV |
                 rgb_flg;

        _width = ST7735_HEIGHT;
        _height = ST7735_WIDTH;
        _ystart = _colstart;
        _xstart = _rowstart;
        break;
    }

#endif

    ST7735_Select();

    ST7735_WriteCommand(ST7735_MADCTL);
    ST7735_WriteData(&madctl, 1);

    ST7735_Unselect();
}

void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= _width || y >= _height) {
        return;
    }

    ST7735_Select();

    ST7735_SetAddressWindow(x, y, x + 1, y + 1);

    uint8_t data[] = {
        (uint8_t)(color >> 8),
        (uint8_t)(color & 0xFF)
    };

    ST7735_WriteData(data, sizeof(data));

    ST7735_Unselect();
}

void ST7735_WriteChar(
    uint16_t x,
    uint16_t y,
    char ch,
    FontDef font,
    uint16_t color,
    uint16_t bgcolor
)
{
    ST7735_SetAddressWindow(
        x,
        y,
        x + font.width - 1,
        y + font.height - 1
    );

    uint8_t fg[] = {
        (uint8_t)(color >> 8),
        (uint8_t)(color & 0xFF)
    };

    uint8_t bg[] = {
        (uint8_t)(bgcolor >> 8),
        (uint8_t)(bgcolor & 0xFF)
    };

    for (uint32_t i = 0; i < font.height; i++) {
        uint32_t b = font.data[(ch - 32) * font.height + i];

        for (uint32_t j = 0; j < font.width; j++) {
            ST7735_WriteData(
                ((b << j) & 0x8000) ? fg : bg,
                2
            );
        }
    }
}

void ST7735_WriteString(
    uint16_t x,
    uint16_t y,
    const char *str,
    FontDef font,
    uint16_t color,
    uint16_t bgcolor
)
{
    ST7735_Select();

    while (*str) {
        if (x + font.width >= _width) {
            x = 0;
            y += font.height;

            if (y + font.height >= _height) {
                break;
            }

            if (*str == ' ') {
                str++;
                continue;
            }
        }

        ST7735_WriteChar(
            x,
            y,
            *str,
            font,
            color,
            bgcolor
        );

        x += font.width;
        str++;
    }

    ST7735_Unselect();
}

void ST7735_FillRectangle(
    uint16_t x,
    uint16_t y,
    uint16_t w,
    uint16_t h,
    uint16_t color
)
{
    if (x >= _width || y >= _height) {
        return;
    }

    if (x + w - 1 >= _width) {
        w = _width - x;
    }

    if (y + h - 1 >= _height) {
        h = _height - y;
    }

    ST7735_Select();

    ST7735_SetAddressWindow(
        x,
        y,
        x + w - 1,
        y + h - 1
    );

    uint8_t data[] = {
        (uint8_t)(color >> 8),
        (uint8_t)(color & 0xFF)
    };

    LL_GPIO_SetOutputPin(DC_PORT, DC_PIN);

    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        SPI_TransmitByte(data[0]);
        SPI_TransmitByte(data[1]);
    }

    while (SPI1->SR & SPI_SR_BSY);

    ST7735_Unselect();
}

void ST7735_DrawImage(
    uint16_t x,
    uint16_t y,
    uint16_t w,
    uint16_t h,
    const uint16_t *data
)
{
    if (x >= _width ||
        y >= _height ||
        x + w - 1 >= _width ||
        y + h - 1 >= _height) {
        return;
    }

    ST7735_Select();

    ST7735_SetAddressWindow(
        x,
        y,
        x + w - 1,
        y + h - 1
    );

    ST7735_WriteData(
        (uint8_t *)data,
        sizeof(uint16_t) * w * h
    );

    ST7735_Unselect();
}

void ST7735_InvertColors(bool invert)
{
    ST7735_Select();

    ST7735_WriteCommand(
        invert ? ST7735_INVON : ST7735_INVOFF
    );

    ST7735_Unselect();
}

void ST7735_draw_column(
    uint8_t x,
    uint8_t y_start,
    uint8_t y_end,
    uint16_t color
)
{
    if (y_start > y_end) {
        return;
    }

    uint16_t height = (y_end - y_start) + 1;

    if (height > 128) {
        height = 128;
    }

    uint8_t buf[256];
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    for (uint16_t i = 0; i < height; i++) {
        buf[i * 2] = hi;
        buf[i * 2 + 1] = lo;
    }

    ST7735_Select();

    ST7735_SetAddressWindow(
        x,
        y_start,
        x,
        y_end
    );

    ST7735_WriteData(buf, height * 2);

    ST7735_Unselect();
}
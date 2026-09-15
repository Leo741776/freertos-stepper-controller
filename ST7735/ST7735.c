#include "ST7735.h"
#include "ST7735_library.h"
#include "int_string.h"
#include "step_generator.h"

#define ROT_0   0
#define ROT_90  1
#define ROT_180 2
#define ROT_270 3

#define START_X      10
#define START_Y      10
#define LABEL_OFFSET 85
#define Y_SPACING    15
#define NUM_DATA     5

#define BACKGROUND_START_X          0
#define BACKGROUND_START_Y          0
#define BACKGROUND_WIDTH_LANDSCAPE  160
#define BACKGROUND_HEIGHT_LANDSCAPE 128

extern FontDef Font_7x10;
extern StepGeneratorHandle step_generator;

void ST7735Init(void)
{
    ST7735_Init(ROT_90);

    ST7735_FillRectangle(
        BACKGROUND_START_X,
        BACKGROUND_START_Y,
        BACKGROUND_WIDTH_LANDSCAPE,
        BACKGROUND_HEIGHT_LANDSCAPE,
        BLACK
    );

    const char *labels[NUM_DATA] = {
        "STATUS:",
        "DIR:",
        "SPEED(RPM):",
        "REVS:",
        "RESOLUTION:"
    };

    for (int i = 0; i < NUM_DATA; i++) {
        ST7735_WriteString(
            START_X,
            START_Y + (i * Y_SPACING),
            labels[i],
            Font_7x10,
            YELLOW,
            BLACK
        );
    }
}

void ST7735Update(const MotionCommand *cmd, A4988Handle *motor)
{
    char string_speed_rpm[6];
    char string_rev_count[10];

    const char *status_str;
    uint16_t status_color;

    if (motor->stop_requested) {
        status_str = "ESTOP ";
        status_color = RED;
    } else if (step_generator.running) {
        status_str = "RUN   ";
        status_color = GREEN;
    } else {
        status_str = "STOP  ";
        status_color = WHITE;
    }

    const char *dir_str = (motor->direction == DIR_CW) ? "CW   " : "CCW  ";

    uint16_to_str(
        (uint16_t)cmd->speed_rpm,
        string_speed_rpm
    );

    if (cmd->speed_rpm < 100) {
        string_speed_rpm[2] = ' ';
        string_speed_rpm[3] = ' ';
        string_speed_rpm[4] = '\0';
    } else {
        string_speed_rpm[3] = ' ';
        string_speed_rpm[4] = '\0';
    }

    uint32_t steps_per_rev = 200;
    switch (motor->microstep) {
    case STEP_FULL:      steps_per_rev = 200;  break;
    case STEP_HALF:      steps_per_rev = 400;  break;
    case STEP_QUARTER:   steps_per_rev = 800;  break;
    case STEP_EIGHTH:    steps_per_rev = 1600; break;
    case STEP_SIXTEENTH: steps_per_rev = 3200; break;
    default:             steps_per_rev = 200;  break;
    }

    uint32_t total_revs = step_generator.step_count / steps_per_rev;

    uint32_to_str(
        total_revs,
        string_rev_count
    );

    const char *microstep_str;

    switch (motor->microstep) {
    case STEP_FULL:      microstep_str = "1/1  "; break;
    case STEP_HALF:      microstep_str = "1/2  "; break;
    case STEP_QUARTER:   microstep_str = "1/4  "; break;
    case STEP_EIGHTH:    microstep_str = "1/8  "; break;
    case STEP_SIXTEENTH: microstep_str = "1/16 "; break;
    default:             microstep_str = "?    "; break;
    }

    const char *lines[NUM_DATA];
    uint16_t colors[NUM_DATA];

    lines[0] = status_str;
    colors[0] = status_color;

    lines[1] = dir_str;
    colors[1] = WHITE;

    lines[2] = string_speed_rpm;
    colors[2] = WHITE;

    lines[3] = string_rev_count;
    colors[3] = WHITE;

    lines[4] = microstep_str;
    colors[4] = WHITE;

    for (int i = 0; i < NUM_DATA; i++) {
        ST7735_WriteString(
            START_X + LABEL_OFFSET,
            START_Y + (i * Y_SPACING),
            lines[i],
            Font_7x10,
            colors[i],
            BLACK
        );
    }
}
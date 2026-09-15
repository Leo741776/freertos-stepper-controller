/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "A4988.h"
#include "ST7735.h"
#include "delay.h"
#include "controls.h"
#include "motion_command.h"
#include "step_generator.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    A4988Handle *motor;
    TimerHandle *delay_timer;
} MotorTaskParams;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ESTOP_BIT       (1U << 0)
#define LIMIT_LEFT_BIT  (1U << 1)
#define LIMIT_RIGHT_BIT (1U << 2)

#define QUEUE_LENGTH 5
#define MOTOR_CONTROL_PERIOD_MS 10
#define RAMP_STEP_RPM 20

#define TOGGLE_DIR_PORT   GPIOA
#define TOGGLE_DIR_PIN    LL_GPIO_PIN_0
#define UP_RPM_PORT       GPIOA
#define UP_RPM_PIN        LL_GPIO_PIN_1
#define DOWN_RPM_PORT     GPIOA
#define DOWN_RPM_PIN      LL_GPIO_PIN_2
#define UP_STEPS_PORT     GPIOA
#define UP_STEPS_PIN      LL_GPIO_PIN_3
#define DOWN_STEPS_PORT   GPIOA
#define DOWN_STEPS_PIN    LL_GPIO_PIN_4

#define A4988_STEP_PORT       GPIOA
#define A4988_STEP_PIN        LL_GPIO_PIN_5
#define A4988_DIRECTION_PORT  GPIOA
#define A4988_DIRECTION_PIN   LL_GPIO_PIN_6
#define A4988_MS1_PORT        GPIOA
#define A4988_MS1_PIN         LL_GPIO_PIN_7
#define A4988_MS2_PORT        GPIOB
#define A4988_MS2_PIN         LL_GPIO_PIN_0
#define A4988_MS3_PORT        GPIOB
#define A4988_MS3_PIN         LL_GPIO_PIN_1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
TaskHandle_t xSafetyTaskHandle = NULL;
TaskHandle_t xMotorControlTaskHandle = NULL;
TaskHandle_t xUITaskHandle = NULL;
EventGroupHandle_t xSafetyEventGroup = NULL;
QueueHandle_t motion_queue = NULL;
SemaphoreHandle_t xSpiMutex = NULL;

A4988Handle motor;
TimerHandle delay_timer;
StepGeneratorHandle step_generator;

MotorTaskParams motor_task_params = {.motor = &motor, .delay_timer = &delay_timer};

static MotionCommand ui_state = {
    .direction = DIR_CW,
    .microstep = STEP_FULL,
    .speed_rpm = SPEED_60_RPM,
    .target_steps = 0
};
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void vSafetyTask(void *argument);
void vMotorControlTask(void *argument);
void vUITask(void *argument);
/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  TimerInitialize(&delay_timer, TIM3);
  StepGeneratorInitialize(&step_generator, TIM16, &motor, &delay_timer);

  motor.step_port = A4988_STEP_PORT;
  motor.step_pin = A4988_STEP_PIN;
  motor.direction_port = A4988_DIRECTION_PORT;
  motor.direction_pin = A4988_DIRECTION_PIN;
  motor.ms1_port = A4988_MS1_PORT;
  motor.ms1_pin = A4988_MS1_PIN;
  motor.ms2_port = A4988_MS2_PORT;
  motor.ms2_pin = A4988_MS2_PIN;
  motor.ms3_port = A4988_MS3_PORT;
  motor.ms3_pin = A4988_MS3_PIN;
    
  motor.direction = DIR_CW;
  motor.microstep = STEP_FULL;
  motor.speed_rpm = SPEED_60_RPM;

  if (!A4988Initialize(&motor)) {
    Error_Handler();
  }
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  xSpiMutex = xSemaphoreCreateMutex();
  if (xSpiMutex == NULL) {
    Error_Handler();
  }
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_QUEUES */
  motion_queue = xQueueCreate(QUEUE_LENGTH, sizeof(MotionCommand));
  if (motion_queue == NULL) {
    Error_Handler();
  }
  /* USER CODE END RTOS_QUEUES */

  /* USER CODE BEGIN RTOS_EVENTS */
  xSafetyEventGroup = xEventGroupCreate();
  if (xSafetyEventGroup == NULL) {
    Error_Handler();
  }
  /* USER CODE END RTOS_EVENTS */

  /* USER CODE BEGIN RTOS_THREADS */
  // Stack size set to 128 words (512 bytes) per task to save RAM on STM32C031 (6KB RAM limit)
  xTaskCreate(vSafetyTask, "Safety", 128, NULL, configMAX_PRIORITIES - 1, &xSafetyTaskHandle);
  xTaskCreate(vMotorControlTask, "Motor Control", 128, &motor_task_params, configMAX_PRIORITIES - 2, &xMotorControlTaskHandle);
  xTaskCreate(vUITask, "UI", 192, NULL, tskIDLE_PRIORITY + 1, &xUITaskHandle);
  /* USER CODE END RTOS_THREADS */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* EXTI Interrupt Callback for E-Stop / Limits */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xEventGroupSetBitsFromISR(xSafetyEventGroup, ESTOP_BIT, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static A4988SpeedRPM cycle_speed_up(A4988SpeedRPM current) {
    switch (current) {
        case SPEED_60_RPM:  return SPEED_100_RPM;
        case SPEED_100_RPM: return SPEED_200_RPM;
        case SPEED_200_RPM: return SPEED_300_RPM;
        case SPEED_300_RPM: return SPEED_300_RPM;
        default:            return SPEED_60_RPM;
    }
}

static A4988SpeedRPM cycle_speed_down(A4988SpeedRPM current) {
    switch (current) {
        case SPEED_300_RPM: return SPEED_200_RPM;
        case SPEED_200_RPM: return SPEED_100_RPM;
        case SPEED_100_RPM: return SPEED_60_RPM;
        case SPEED_60_RPM:  return SPEED_60_RPM;
        default:            return SPEED_60_RPM;
    }
}

static A4988Microstep cycle_steps_up(A4988Microstep current) {
    switch (current) {
        case STEP_FULL:      return STEP_HALF;
        case STEP_HALF:      return STEP_QUARTER;
        case STEP_QUARTER:   return STEP_EIGHTH;
        case STEP_EIGHTH:    return STEP_SIXTEENTH;
        case STEP_SIXTEENTH: return STEP_SIXTEENTH;
        default:             return STEP_FULL;
    }
}

static A4988Microstep cycle_steps_down(A4988Microstep current) {
    switch (current) {
        case STEP_SIXTEENTH: return STEP_EIGHTH;
        case STEP_EIGHTH:    return STEP_QUARTER;
        case STEP_QUARTER:   return STEP_HALF;
        case STEP_HALF:      return STEP_FULL;
        case STEP_FULL:      return STEP_FULL;
        default:             return STEP_FULL;
    }
}

void vSafetyTask(void *argument) {
    for (;;) {
        EventBits_t bits = xEventGroupWaitBits(
            xSafetyEventGroup, 
            ESTOP_BIT | LIMIT_LEFT_BIT | LIMIT_RIGHT_BIT, 
            pdTRUE, 
            pdFALSE, 
            portMAX_DELAY
        );

        if (bits & (ESTOP_BIT | LIMIT_LEFT_BIT | LIMIT_RIGHT_BIT)) {
            StepGeneratorStop(&step_generator);
            motor.stop_requested = true;
        }
    }
}

void vMotorControlTask(void *argument) {
    MotorTaskParams *params = (MotorTaskParams *)argument;
    A4988Handle *m = params->motor;

    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(MOTOR_CONTROL_PERIOD_MS);
    
    MotionCommand current_cmd = {0};
    uint32_t ramped_rpm = SPEED_60_RPM;

    xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        MotionCommand new_cmd;
        if (xQueueReceive(motion_queue, &new_cmd, 0) == pdPASS) {
            A4988SetDirection(m, new_cmd.direction);
            A4988SetMicrostep(m, new_cmd.microstep);
            current_cmd = new_cmd;
            m->stop_requested = false;
        }

        if (ramped_rpm < (uint32_t)current_cmd.speed_rpm) {
            ramped_rpm += RAMP_STEP_RPM;
            if (ramped_rpm > (uint32_t)current_cmd.speed_rpm) ramped_rpm = current_cmd.speed_rpm;
        } else if (ramped_rpm > (uint32_t)current_cmd.speed_rpm) {
            ramped_rpm -= RAMP_STEP_RPM;
            if (ramped_rpm < (uint32_t)current_cmd.speed_rpm) ramped_rpm = current_cmd.speed_rpm;
        }

        A4988SetSpeedRPM(m, (A4988SpeedRPM)ramped_rpm);

        uint32_t interval_us = A4988GetStepIntervalUs(m);
        if (interval_us > 0 && !m->stop_requested) {
            StepGeneratorSetIntervalUs(&step_generator, interval_us);
            StepGeneratorStart(&step_generator);
        } else {
            StepGeneratorStop(&step_generator);
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void vUITask(void *argument) {
    static bool dir_was_pressed = false;
    static bool up_rpm_was_pressed = false;
    static bool down_rpm_was_pressed = false;
    static bool up_steps_was_pressed = false;
    static bool down_steps_was_pressed = false;

    for (;;) {
        bool state_changed = false;

        if (button_pressed(TOGGLE_DIR_PORT, TOGGLE_DIR_PIN, &dir_was_pressed)) {
            ui_state.direction = (ui_state.direction == DIR_CW) ? DIR_CCW : DIR_CW;
            state_changed = true;
        }
        if (button_pressed(UP_RPM_PORT, UP_RPM_PIN, &up_rpm_was_pressed)) {
            ui_state.speed_rpm = cycle_speed_up(ui_state.speed_rpm);
            state_changed = true;
        }
        if (button_pressed(DOWN_RPM_PORT, DOWN_RPM_PIN, &down_rpm_was_pressed)) {
            ui_state.speed_rpm = cycle_speed_down(ui_state.speed_rpm);
            state_changed = true;
        }
        if (button_pressed(UP_STEPS_PORT, UP_STEPS_PIN, &up_steps_was_pressed)) {
            ui_state.microstep = cycle_steps_up(ui_state.microstep);
            state_changed = true;
        }
        if (button_pressed(DOWN_STEPS_PORT, DOWN_STEPS_PIN, &down_steps_was_pressed)) {
            ui_state.microstep = cycle_steps_down(ui_state.microstep);
            state_changed = true;
        }

        /* Only post to queue when user actually presses a button */
        if (state_changed) {
            xQueueSend(motion_queue, &ui_state, 0);
        }

        /* Render live screen data under SPI Mutex protection */
        if (xSemaphoreTake(xSpiMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            ST7735Update(&ui_state, &motor);
            xSemaphoreGive(xSpiMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
/* USER CODE END Application */
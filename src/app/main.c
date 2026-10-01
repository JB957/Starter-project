#include "main.h"

#include <stdbool.h>
#include <stdio.h>

#include "can.h"
#include "clock.h"
#include "gpio.h"
#include "adc.h"
#include "error_handler.h"
#include "core_config.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "rtt.h"

#include <stm32g4xx_hal.h>

#define HEIGHT 20
#define WIDTH 10

// Blinky Light Code

void heartbeat_task(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        core_GPIO_toggle_heartbeat();
        vTaskDelay(200 * portTICK_PERIOD_MS);
    }
}

// Keyboard

void Keys(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        char key = SEGGER_RTT_GetKey();
        if (key == "w")
        {
            rprintf("Up\n");
        }
        if (key == "a")
        {
            rprintf("Left\n");
        }
        if (key == "s")
        {
            rprintf("Down \n");
        }
        if (key == "d")
        {
            rprintf("Right\n");
        }
    }
}

void MakeBoard(void)
{
    uint8_t board[HEIGHT][WIDTH];

    rprintf("+----------+\n");

    for (int y = 0; y < HEIGHT; y++)
    {
        rprintf("|");

        for (int x = 0; x < WIDTH; x++)
        {
            if (board[y][x])
                rprintf("#");
            else
                rprintf(" ");
        }

        rprintf("|\n");
    }

    rprintf("+----------+\n");
}

//  Main Duh

int main(void)
{
    HAL_Init();

    if (!core_clock_init())
        error_handler();

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // hand control over to FreeRTOS
    vTaskStartScheduler();

    // we should not get here ever
    error_handler();
    return 1;
}

// Called when stack overflows from rtos
// Not needed in header, since included in FreeRTOS-Kernel/include/task.h
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    error_handler();
}

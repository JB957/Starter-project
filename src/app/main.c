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

#define PIECE_COUNT 7

uint8_t Board[HEIGHT][WIDTH];

uint8_t Block[4][4] =
    {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}};

typedef struct
{
    int x;
    int y;
    int rotation;
    int type;
} Tetromino;

Tetromino Piece;

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

void GameLoop(void *pvParameter)
{
    (void)pvParameter;
    while (true)
    {
        newblock();
        Update_Game();
        MakeBoard();
    }
}
void Get_Input(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        char key = SEGGER_RTT_GetKey();
        if (key == "w")
        {
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

void newblock(void)
{

    Piece.x = 3;
    Piece.y = 0;
    Piece.type = "t";
}

void Update_Game(void)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (Block[row][col] == 1)
            {
                int board_x = Piece.x + col;
                int board_y = Piece.y + row;

                Board[board_y][board_x] = 1;
            }
        }
    }
}

void MakeBoard(void)
{

    rprintf("+----------+\n");

    for (int y = 0; y < HEIGHT; y++)
    {
        rprintf("|");

        for (int x = 0; x < WIDTH; x++)
        {
            if (Board[y][x])
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

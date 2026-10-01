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

uint8_t Board[HEIGHT][WIDTH];

uint8_t Block[4][4][4] =
    {
        {{0, 1, 0, 0},
         {1, 1, 1, 0},
         {0, 0, 0, 0},
         {0, 0, 0, 0}},
        {{1, 0, 0, 0},
         {1, 1, 0, 0},
         {1, 0, 0, 0},
         {0, 0, 0, 0}},
        {{1, 1, 1, 0},
         {0, 1, 0, 0},
         {0, 0, 0, 0},
         {0, 0, 0, 0}},
        {{0, 1, 0, 0},
         {1, 1, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 0, 0}}};

typedef struct
{
    int x;
    int y;
    int rotation;

} Tetromino;

Tetromino Piece;

void NewBlock(void)
{
    Piece.x = 3;
    Piece.y = 0;
    Piece.rotation = 0;
}

bool CanMove(int newX, int newY)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (Block[Piece.rotation][y][x])
            {
                int boardX = newX + x;
                int boardY = newY + y;

                // hit walls/floor
                if (boardX < 0 || boardX >= WIDTH)
                    return false;

                if (boardY < 0 || boardY >= HEIGHT)
                    return false;

                // hit existing block
                if (Board[boardY][boardX])
                    return false;
            }
        }
    }

    return true;
}

void RotatePiece(void)
{
    int oldRotation = Piece.rotation;

    Piece.rotation++;
    if (Piece.rotation >= 4)
        Piece.rotation = 0;

    if (!CanMove(Piece.x, Piece.y))
        Piece.rotation = oldRotation;
}

void LockPiece(void)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (Block[Piece.rotation][y][x])
            {
                Board[Piece.y + y][Piece.x + x] = 1;
            }
        }
    }
}

void Render(void)
{
    rprintf("\033[2J"); // clear terminal

    rprintf("+----------+\n");

    for (int y = 0; y < HEIGHT; y++)
    {
        rprintf("|");

        for (int x = 0; x < WIDTH; x++)
        {
            bool draw = Board[y][x];

            // draw falling piece
            int localX = x - Piece.x;
            int localY = y - Piece.y;

            if (localX >= 0 && localX < 4 &&
                localY >= 0 && localY < 4)
            {
                if (Block[Piece.rotation][localY][localX])
                    draw = true;
            }

            if (draw)
                rprintf("#");
            else
                rprintf(".");
        }

        rprintf("|\n");
    }

    rprintf("+----------+\n");
}

void Get_Input(void)
{
    int key = SEGGER_RTT_GetKey();

    if (key == 'a')
    {
        if (CanMove(Piece.x - 1, Piece.y))
            Piece.x--;
    }

    if (key == 'd')
    {
        if (CanMove(Piece.x + 1, Piece.y))
            Piece.x++;
    }

    if (key == 's')
    {
        if (CanMove(Piece.x, Piece.y + 1))
            Piece.y++;
    }
    if (key == 'w')
    {
        RotatePiece();
    }
}

void Update_Game(void)
{
    static uint32_t timer = 0;

    timer++;

    // gravity every ~500ms
    if (timer > 25)
    {
        timer = 0;

        if (CanMove(Piece.x, Piece.y + 1))
        {
            Piece.y++;
        }
        else
        {
            LockPiece();

            NewBlock();
        }
    }
}

void GameLoop(void *pvParameter)
{
    (void)pvParameter;

    NewBlock();

    while (true)
    {
        Get_Input();

        Update_Game();

        Render();

        vTaskDelay(pdMS_TO_TICKS(20));
    }
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

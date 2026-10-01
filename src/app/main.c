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

#define PIECE_COUNT 7
#define ROTATIONS 4
#define PIECE_SIZE 4

enum PieceType
{
    PIECE_T,
    PIECE_I,
    PIECE_O,
    PIECE_L,
    PIECE_J,
    PIECE_S,
    PIECE_Z
};

uint8_t Pieces[PIECE_COUNT][ROTATIONS][PIECE_SIZE][PIECE_SIZE] =
    {
        // T
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
             {0, 0, 0, 0}}},

        // I
        {
            {{1, 1, 1, 1},
             {0, 0, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 0, 0, 0},
             {1, 0, 0, 0},
             {1, 0, 0, 0},
             {1, 0, 0, 0}},
            {{1, 1, 1, 1},
             {0, 0, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 0, 0, 0},
             {1, 0, 0, 0},
             {1, 0, 0, 0},
             {1, 0, 0, 0}}},

        // O
        {
            {{1, 1, 0, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 0, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 0, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 0, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}}},

        // L
        {
            {{1, 0, 0, 0},
             {1, 1, 1, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 0, 0},
             {1, 0, 0, 0},
             {1, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 1, 0},
             {0, 0, 1, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{0, 1, 0, 0},
             {0, 1, 0, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0}}},

        // J
        {
            {{0, 0, 1, 0},
             {1, 1, 1, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 0, 0, 0},
             {1, 0, 0, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 1, 0},
             {1, 0, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 0, 0},
             {0, 1, 0, 0},
             {0, 1, 0, 0},
             {0, 0, 0, 0}}},

        // S
        {
            {{0, 1, 1, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 0, 0, 0},
             {1, 1, 0, 0},
             {0, 1, 0, 0},
             {0, 0, 0, 0}},
            {{0, 1, 1, 0},
             {1, 1, 0, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 0, 0, 0},
             {1, 1, 0, 0},
             {0, 1, 0, 0},
             {0, 0, 0, 0}}},

        // Z
        {
            {{1, 1, 0, 0},
             {0, 1, 1, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{0, 1, 0, 0},
             {1, 1, 0, 0},
             {1, 0, 0, 0},
             {0, 0, 0, 0}},
            {{1, 1, 0, 0},
             {0, 1, 1, 0},
             {0, 0, 0, 0},
             {0, 0, 0, 0}},
            {{0, 1, 0, 0},
             {1, 1, 0, 0},
             {1, 0, 0, 0},
             {0, 0, 0, 0}}}};

typedef struct
{
    int x;
    int y;
    int rotation;
    int type;

} Tetromino;

Tetromino Piece = {.type = PIECE_COUNT - 1};

void NewBlock(void)
{
    Piece.x = 3;
    Piece.y = 0;
    Piece.rotation = 0;

    Piece.type++;
    if (Piece.type >= PIECE_COUNT)
        Piece.type = 0;
}

bool CanMove(int newX, int newY)
{
    for (int y = 0; y < PIECE_SIZE; y++)
    {
        for (int x = 0; x < PIECE_SIZE; x++)
        {
            if (Pieces[Piece.type][Piece.rotation][y][x])
            {
                int boardX = newX + x;
                int boardY = newY + y;

                // hit walls/floor
                if (boardX < 0 || boardX >= WIDTH)
                    return false;

                if (boardY >= HEIGHT)
                    return false;

                // hit existing block
                if (boardY >= 0 && Board[boardY][boardX])
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
    if (Piece.rotation >= ROTATIONS)
        Piece.rotation = 0;

    if (!CanMove(Piece.x, Piece.y))
        Piece.rotation = oldRotation;
}

void LockPiece(void)
{
    for (int y = 0; y < PIECE_SIZE; y++)
    {
        for (int x = 0; x < PIECE_SIZE; x++)
        {
            if (Pieces[Piece.type][Piece.rotation][y][x])
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

            if (localX >= 0 && localX < PIECE_SIZE &&
                localY >= 0 && localY < PIECE_SIZE)
            {
                if (Pieces[Piece.type][Piece.rotation][localY][localX])
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

    if (xTaskCreate(GameLoop, "Game", 1000, NULL, 4, NULL) != pdPASS)
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

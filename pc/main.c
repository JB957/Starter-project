#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define HEIGHT 20
#define WIDTH 10

static uint8_t board[HEIGHT][WIDTH] = {0};

#ifndef _WIN32
static struct termios original_terminal;
static bool terminal_changed = false;

static void restore_terminal(void)
{
    if (terminal_changed)
        tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal);
}

static void setup_terminal(void)
{
    if (!isatty(STDIN_FILENO) || tcgetattr(STDIN_FILENO, &original_terminal) != 0)
        return;

    struct termios raw = original_terminal;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO | ISIG);
    raw.c_iflag &= (tcflag_t)~(IXON | ICRNL);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0)
    {
        terminal_changed = true;
        atexit(restore_terminal);
    }
}
#endif

static int read_key(void)
{
#ifdef _WIN32
    return _getch();
#else
    return getchar();
#endif
}

static void make_board(void)
{
    puts("+----------+");

    for (int y = 0; y < HEIGHT; y++)
    {
        putchar('|');
        for (int x = 0; x < WIDTH; x++)
            putchar(board[y][x] ? '#' : ' ');
        puts("|");
    }

    puts("+----------+");
}

int main(void)
{
#ifndef _WIN32
    setup_terminal();
#endif

    make_board();
    puts("Press W/A/S/D to test input, Q to quit.");
    fflush(stdout);

    for (;;)
    {
        int key = read_key();
        if (key == EOF || key == 'q' || key == 'Q' || key == 3)
            break;

        switch (key)
        {
        case 'w':
        case 'W':
            puts("Up");
            break;
        case 'a':
        case 'A':
            puts("Left");
            break;
        case 's':
        case 'S':
            puts("Down");
            break;
        case 'd':
        case 'D':
            puts("Right");
            break;
        default:
            continue;
        }
        fflush(stdout);
    }

    return 0;
}

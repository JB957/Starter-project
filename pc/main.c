#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

#define HEIGHT 20
#define WIDTH 10

static uint8_t board[HEIGHT][WIDTH] = {0};
static const uint8_t block[4][4][4] = {
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
     {0, 0, 0, 0}},
};

typedef struct
{
    int x;
    int y;
    int rotation;
} Tetromino;

static Tetromino piece;
static unsigned int gravity_ticks;
static bool terminal_supports_ansi;

enum { NO_KEY = -2 };

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
    if (!_kbhit())
        return NO_KEY;
    return _getch();
#else
    fd_set input;
    FD_ZERO(&input);
    FD_SET(STDIN_FILENO, &input);
    struct timeval timeout = {0, 0};

    int ready = select(STDIN_FILENO + 1, &input, NULL, NULL, &timeout);
    if (ready < 0)
        return EOF;
    if (ready == 0)
        return NO_KEY;

    unsigned char key;
    return read(STDIN_FILENO, &key, 1) == 1 ? key : EOF;
#endif
}

static void wait_for_tick(void)
{
#ifdef _WIN32
    Sleep(20);
#else
    struct timeval delay = {0, 20000};
    select(0, NULL, NULL, NULL, &delay);
#endif
}

static void new_block(void)
{
    piece.x = 3;
    piece.y = 0;
    piece.rotation = 0;
}

static bool can_move(int new_x, int new_y)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (block[piece.rotation][y][x])
            {
                int board_x = new_x + x;
                int board_y = new_y + y;

                if (board_x < 0 || board_x >= WIDTH || board_y < 0 || board_y >= HEIGHT)
                    return false;

                if (board[board_y][board_x])
                    return false;
            }
        }
    }

    return true;
}

static void rotate_piece(void)
{
    int old_rotation = piece.rotation;
    piece.rotation = (piece.rotation + 1) % 4;

    if (!can_move(piece.x, piece.y))
        piece.rotation = old_rotation;
}

static void lock_piece(void)
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (block[piece.rotation][y][x])
            {
                int board_x = piece.x + x;
                int board_y = piece.y + y;
                board[board_y][board_x] = 1;
            }
        }
    }
}

static void get_input(int key)
{
    if ((key == 'a' || key == 'A') && can_move(piece.x - 1, piece.y))
        piece.x--;
    if ((key == 'd' || key == 'D') && can_move(piece.x + 1, piece.y))
        piece.x++;
    if ((key == 's' || key == 'S') && can_move(piece.x, piece.y + 1))
        piece.y++;
    if (key == 'w' || key == 'W')
        rotate_piece();
}

static void update_game(void)
{
    gravity_ticks++;
    if (gravity_ticks > 25)
    {
        gravity_ticks = 0;

        if (can_move(piece.x, piece.y + 1))
            piece.y++;
        else
        {
            lock_piece();
            new_block();
        }
    }
}

static void render(void)
{
    if (terminal_supports_ansi)
        fputs("\033[H\033[2J", stdout);

    puts("+----------+");

    for (int y = 0; y < HEIGHT; y++)
    {
        putchar('|');
        for (int x = 0; x < WIDTH; x++)
        {
            bool draw = board[y][x];
            int local_x = x - piece.x;
            int local_y = y - piece.y;

            if (local_x >= 0 && local_x < 4 && local_y >= 0 && local_y < 4)
                draw = draw || block[piece.rotation][local_y][local_x];

            putchar(draw ? '#' : '.');
        }
        puts("|");
    }

    puts("+----------+");
    puts("A/D: move  S: down  W: rotate  Q: quit");
    fflush(stdout);
}

int main(void)
{
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (output != INVALID_HANDLE_VALUE && GetConsoleMode(output, &mode))
        terminal_supports_ansi = SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
    setup_terminal();
    terminal_supports_ansi = isatty(STDOUT_FILENO);
#endif

    new_block();
    render();

    for (;;)
    {
        int key = read_key();
        if (key == EOF || key == 'q' || key == 'Q' || key == 3)
            break;

        get_input(key);
        update_game();
        render();
        wait_for_tick();
    }

    return 0;
}

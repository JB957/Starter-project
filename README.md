# STM32G4xx Project Template

## Overview
This is a Github template with everything needed to make a new STM32G4xx project from scratch.
This is not meant to be edited with production code.

## Setup
 1. Go to the Github website view of this repository, and in the top right corner, create a
    clone of this template with the "Use This Template" button.
 2. Clone the newly created repository to your computer.
 3. Run the setup script (`./scripts/setup.sh`) with the new name of the project as its
    argument.
 4. Confirm that the project works by running `make`

## Requirements
### Windows-Specific:
- [Windows Subsystem for Linux](https://learn.microsoft.com/en-us/windows/wsl/install)
- [Docker Desktop](https://docs.docker.com/desktop/install/windows-install/)

Not strictly required, but advised, is Github Desktop.

## Test the current Tetris scaffold on a PC

`pc/main.c` is a standalone terminal version of the T-piece board display and
WASD input handling in `src/app/main.c`. It does not need the STM32 toolchain or
a board.

On macOS or Linux, run from the project directory:

```sh
cc -std=c11 -Wall -Wextra -o pc/tetris pc/main.c
./pc/tetris
```

On Windows with MinGW, run:

```sh
gcc -std=c11 -Wall -Wextra -o pc/tetris.exe pc/main.c
.\pc\tetris.exe
```

Press W, A, S, or D to check the input messages, and Q to quit. In an
interactive terminal, each key is read immediately. When input is piped in,
the program reads the bytes until the pipe closes.

The program places a T piece at the top of the board before reading input.
Falling pieces, movement, line clearing, and scoring are not yet implemented.

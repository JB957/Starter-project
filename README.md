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

`pc/main.c` is a standalone terminal version of the T-piece game loop in
`src/app/main.c`. It does not need the STM32 toolchain or a board.

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

Press A or D to move the falling piece, S to move it down, W to rotate it,
and Q to quit. In an interactive terminal, each key is read immediately.
The piece also falls automatically about every half second. Empty cells
are shown as dots.

Pieces lock when they land and a new T piece appears at the top. Line
clearing and scoring are not yet implemented.

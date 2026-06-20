# Minesweeper

扫雷游戏 - Minesweeper game implemented in C with raylib

## Features
- Three difficulty levels: Beginner (9x9, 10 mines), Intermediate (16x16, 40 mines), Expert (16x30, 99 mines)
- Resizable window
- Sound effects
- Flag and chord support
- Timer and mine counter

## Build

```bash
gcc -o minesweeper.exe main.c minesweeper.c -I<raylib_path>/include -L<raylib_path>/lib -lraylib -lwinmm -lgdi32 -lopengl32 -static -lm
```
@echo off
if "%1"=="" (
    set RAYLIB_DIR=..\raylib
) else (
    set RAYLIB_DIR=%1
)

gcc -o minesweeper.exe main.c minesweeper.c -I%RAYLIB_DIR%\include -L%RAYLIB_DIR%\lib -lraylib -lwinmm -lgdi32 -lopengl32 -static -lm
if %errorlevel% equ 0 (
    echo Build successful: minesweeper.exe
) else (
    echo Build failed.
)
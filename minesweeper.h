#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <stdbool.h>

#define BEGINNER_ROWS   9
#define BEGINNER_COLS   9
#define BEGINNER_MINES  10

#define INTERMEDIATE_ROWS   16
#define INTERMEDIATE_COLS   16
#define INTERMEDIATE_MINES  40

#define EXPERT_ROWS   16
#define EXPERT_COLS   30
#define EXPERT_MINES  99

typedef enum {
    FACE_SMILE = 0,
    FACE_SURPRISE,
    FACE_WON,
    FACE_LOST
} FaceState;

typedef enum {
    DIFF_BEGINNER = 0,
    DIFF_INTERMEDIATE,
    DIFF_EXPERT
} Difficulty;

typedef struct {
    int rows;
    int cols;
    int total_mines;
} DifficultyConfig;

typedef struct {
    bool mine;
    bool revealed;
    bool flagged;
    int adjacent;
} Cell;

typedef struct {
    int rows;
    int cols;
    int total_mines;
    int flags_placed;
    int cells_revealed;
    bool first_click_done;
    bool game_over;
    bool won;
    int mouse_grid_x;
    int mouse_grid_y;
    double start_time;
    double elapsed;
    FaceState face;
    Difficulty difficulty;
    Cell *grid;
} Game;

void game_init(Game *g, Difficulty diff);
void game_reset(Game *g);
void game_destroy(Game *g);
Cell *game_cell(Game *g, int r, int c);
void game_place_mines(Game *g, int safe_r, int safe_c);
void game_reveal(Game *g, int r, int c);
void game_toggle_flag(Game *g, int r, int c);
void game_chord(Game *g, int r, int c);
void game_check_win(Game *g);
void game_update_elapsed(Game *g);

#endif
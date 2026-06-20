#include "minesweeper.h"
#include <stdlib.h>
#include <time.h>

#define NEIGHBOR_DIRS 8
static const int dr[NEIGHBOR_DIRS] = {-1, -1, -1,  0, 0,  1, 1, 1};
static const int dc[NEIGHBOR_DIRS] = {-1,  0,  1, -1, 1, -1, 0, 1};

static DifficultyConfig diffs[] = {
    { BEGINNER_ROWS,     BEGINNER_COLS,     BEGINNER_MINES     },
    { INTERMEDIATE_ROWS, INTERMEDIATE_COLS, INTERMEDIATE_MINES },
    { EXPERT_ROWS,       EXPERT_COLS,       EXPERT_MINES       }
};

static int count_adjacent_mines(Game *g, int r, int c)
{
    int i, count = 0;
    for (i = 0; i < NEIGHBOR_DIRS; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < g->rows && nc >= 0 && nc < g->cols) {
            if (game_cell(g, nr, nc)->mine) count++;
        }
    }
    return count;
}

static int count_adjacent_flags(Game *g, int r, int c)
{
    int i, count = 0;
    for (i = 0; i < NEIGHBOR_DIRS; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < g->rows && nc >= 0 && nc < g->cols) {
            if (game_cell(g, nr, nc)->flagged) count++;
        }
    }
    return count;
}

void game_init(Game *g, Difficulty diff)
{
    g->difficulty = diff;
    g->rows = diffs[diff].rows;
    g->cols = diffs[diff].cols;
    g->total_mines = diffs[diff].total_mines;
    g->grid = (Cell *)calloc(g->rows * g->cols, sizeof(Cell));
    game_reset(g);
}

void game_reset(Game *g)
{
    int i;
    for (i = 0; i < g->rows * g->cols; i++) {
        g->grid[i].mine = false;
        g->grid[i].revealed = false;
        g->grid[i].flagged = false;
        g->grid[i].adjacent = 0;
    }
    g->flags_placed = 0;
    g->cells_revealed = 0;
    g->first_click_done = false;
    g->game_over = false;
    g->won = false;
    g->elapsed = 0.0;
    g->face = FACE_SMILE;
}

void game_destroy(Game *g)
{
    if (g->grid != NULL) {
        free(g->grid);
        g->grid = NULL;
    }
}

Cell *game_cell(Game *g, int r, int c)
{
    return &g->grid[r * g->cols + c];
}

void game_place_mines(Game *g, int safe_r, int safe_c)
{
    int mines_to_place = g->total_mines;
    int i, j;

    srand((unsigned int)time(NULL));

    while (mines_to_place > 0) {
        int r = rand() % g->rows;
        int c = rand() % g->cols;
        if (game_cell(g, r, c)->mine) continue;

        /* skip safe zone around first click */
        if (abs(r - safe_r) <= 1 && abs(c - safe_c) <= 1) continue;

        game_cell(g, r, c)->mine = true;
        mines_to_place--;
    }

    /* calculate adjacent mine counts */
    for (i = 0; i < g->rows; i++) {
        for (j = 0; j < g->cols; j++) {
            if (!game_cell(g, i, j)->mine) {
                game_cell(g, i, j)->adjacent = count_adjacent_mines(g, i, j);
            }
        }
    }
}

/* forward declaration for flood fill */
static void flood_fill(Game *g, int r, int c);

void game_reveal(Game *g, int r, int c)
{
    Cell *cell;

    if (g->game_over) return;
    if (r < 0 || r >= g->rows || c < 0 || c >= g->cols) return;

    cell = game_cell(g, r, c);
    if (cell->revealed || cell->flagged) return;

    /* first click: place mines avoiding this cell */
    if (!g->first_click_done) {
        g->first_click_done = true;
        g->start_time = ((double)clock()) / CLOCKS_PER_SEC;
        game_place_mines(g, r, c);
    }

    if (cell->mine) {
        /* BANG! */
        cell->revealed = true;
        g->game_over = true;
        g->won = false;
        g->face = FACE_LOST;
        /* reveal all mines */
        {
            int i, j;
            for (i = 0; i < g->rows; i++) {
                for (j = 0; j < g->cols; j++) {
                    Cell *c2 = game_cell(g, i, j);
                    if (c2->mine && !c2->flagged) {
                        c2->revealed = true;
                    }
                }
            }
        }
        return;
    }

    cell->revealed = true;
    g->cells_revealed++;

    if (cell->adjacent == 0) {
        flood_fill(g, r, c);
    }

    game_check_win(g);
}

static void flood_fill(Game *g, int r, int c)
{
    int i;
    for (i = 0; i < NEIGHBOR_DIRS; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < g->rows && nc >= 0 && nc < g->cols) {
            Cell *cell = game_cell(g, nr, nc);
            if (!cell->revealed && !cell->flagged && !cell->mine) {
                cell->revealed = true;
                g->cells_revealed++;
                if (cell->adjacent == 0) {
                    flood_fill(g, nr, nc);
                }
            }
        }
    }
}

void game_toggle_flag(Game *g, int r, int c)
{
    Cell *cell;
    if (g->game_over) return;
    if (r < 0 || r >= g->rows || c < 0 || c >= g->cols) return;

    cell = game_cell(g, r, c);
    if (cell->revealed) return;

    cell->flagged = !cell->flagged;
    if (cell->flagged) {
        g->flags_placed++;
    } else {
        g->flags_placed--;
    }
}

void game_chord(Game *g, int r, int c)
{
    Cell *cell;
    int flags, i;

    if (g->game_over) return;
    if (r < 0 || r >= g->rows || c < 0 || c >= g->cols) return;

    cell = game_cell(g, r, c);
    if (!cell->revealed || cell->adjacent == 0) return;

    flags = count_adjacent_flags(g, r, c);
    if (flags != cell->adjacent) return;

    for (i = 0; i < NEIGHBOR_DIRS; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr >= 0 && nr < g->rows && nc >= 0 && nc < g->cols) {
            Cell *neighbor = game_cell(g, nr, nc);
            if (!neighbor->revealed && !neighbor->flagged) {
                game_reveal(g, nr, nc);
            }
        }
    }
}

void game_check_win(Game *g)
{
    int total_safe = g->rows * g->cols - g->total_mines;
    if (g->cells_revealed >= total_safe) {
        g->game_over = true;
        g->won = true;
        g->face = FACE_WON;
        /* flag remaining mines */
        {
            int i, j;
            for (i = 0; i < g->rows; i++) {
                for (j = 0; j < g->cols; j++) {
                    Cell *c = game_cell(g, i, j);
                    if (c->mine && !c->flagged) {
                        c->flagged = true;
                        g->flags_placed++;
                    }
                }
            }
        }
    }
}

void game_update_elapsed(Game *g)
{
    if (g->first_click_done && !g->game_over) {
        g->elapsed = ((double)clock()) / CLOCKS_PER_SEC - g->start_time;
        if (g->elapsed > 999.0) g->elapsed = 999.0;
    }
}

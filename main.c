#include "raylib.h"
#include "minesweeper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define BASE_CELL      32
#define BASE_PADDING   8
#define BASE_HEADER    48
#define BASE_TOP_PANEL   80
#define BASE_BOTTOM_BAR  36
#define MIN_CELL         22

static Color num_colors[9] = {
    {0,0,0,255},       /* 0 - unused */
    {0,0,255,255},     /* 1 - blue */
    {0,128,0,255},     /* 2 - green */
    {255,0,0,255},     /* 3 - red */
    {0,0,128,255},     /* 4 - dark blue */
    {128,0,0,255},     /* 5 - maroon */
    {0,128,128,255},   /* 6 - teal */
    {0,0,0,255},       /* 7 - black */
    {128,128,128,255}  /* 8 - gray */
};

static Color lighter(Color c, int amount)
{
    int r, g, b;
    r = c.r + amount; if (r > 255) r = 255;
    g = c.g + amount; if (g > 255) g = 255;
    b = c.b + amount; if (b > 255) b = 255;
    return Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
}

static Color darker(Color c, int amount)
{
    int r, g, b;
    r = c.r - amount; if (r < 0) r = 0;
    g = c.g - amount; if (g < 0) g = 0;
    b = c.b - amount; if (b < 0) b = 0;
    return Color{ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 };
}

static void draw_btn_raised(int x, int y, int w, int h)
{
    Color c = Color{ 208, 208, 208, 255 };
    DrawRectangle(x, y, w, h, c);
    DrawRectangle(x, y, w, 2, WHITE);
    DrawRectangle(x, y, 2, h, WHITE);
    DrawRectangle(x + 2, y + 2, w - 4, 1, lighter(c, 40));
    DrawRectangle(x + 2, y + 2, 1, h - 4, lighter(c, 40));
    DrawRectangle(x, y + h - 2, w, 2, GRAY);
    DrawRectangle(x + w - 2, y, 2, h, GRAY);
    DrawRectangle(x, y + h - 3, w, 1, darker(c, 40));
    DrawRectangle(x + w - 3, y, 1, h, darker(c, 40));
}

static void draw_btn_sunken(int x, int y, int w, int h)
{
    Color c = Color{ 170, 170, 170, 255 };
    DrawRectangle(x, y, w, h, c);
    DrawRectangle(x, y, w, 2, GRAY);
    DrawRectangle(x, y, 2, h, GRAY);
    DrawRectangle(x + 2, y + 2, w - 4, 1, DARKGRAY);
    DrawRectangle(x + 2, y + 2, 1, h - 4, DARKGRAY);
    DrawRectangle(x, y + h - 2, w, 2, WHITE);
    DrawRectangle(x + w - 2, y, 2, h, WHITE);
}

static void draw_cell_raised(int x, int y, int size)
{
    Color c = Color{ 208, 208, 208, 255 };
    DrawRectangle(x, y, size, size, c);
    DrawRectangle(x, y, size, 2, WHITE);
    DrawRectangle(x, y, 2, size, WHITE);
    DrawRectangle(x + 2, y + 2, size - 4, 1, lighter(c, 40));
    DrawRectangle(x + 2, y + 2, 1, size - 4, lighter(c, 40));
    DrawRectangle(x, y + size - 2, size, 2, GRAY);
    DrawRectangle(x + size - 2, y, 2, size, GRAY);
    DrawRectangle(x, y + size - 3, size, 1, darker(c, 40));
    DrawRectangle(x + size - 3, y, 1, size, darker(c, 40));
}

static void draw_cell_sunken(int x, int y, int size)
{
    Color c = Color{ 188, 188, 188, 255 };
    DrawRectangle(x, y, size, size, c);
    DrawRectangle(x, y, size, 2, GRAY);
    DrawRectangle(x, y, 2, size, GRAY);
    DrawRectangle(x + 2, y + 2, size - 4, 1, DARKGRAY);
    DrawRectangle(x + 2, y + 2, 1, size - 4, DARKGRAY);
    DrawRectangle(x, y + size - 2, size, 2, WHITE);
    DrawRectangle(x + size - 2, y, 2, size, WHITE);
}

static void draw_flag(int x, int y, int size)
{
    int cx, cy, fh, fw, top;
    cx = x + size / 2;
    cy = y + size / 2;
    fh = size * 5 / 8;
    fw = size * 3 / 8;
    if (fh < 6) fh = 6;
    if (fw < 4) fw = 4;
    top = cy - fh / 2;

    /* pole */
    DrawRectangle(cx - 1, top - 1, 2, fh + 3, BLACK);

    /* pennant triangle */
    {
        Vector2 vt, vb, vm;
        vt = Vector2{ (float)(cx - 1), (float)(top) };
        vb = Vector2{ (float)(cx - 1), (float)(top + fh) };
        vm = Vector2{ (float)(cx - fw - 1), (float)(cy) };
        DrawTriangle(vt, vm, vb, RED);
        DrawTriangleLines(vt, vm, vb, BLACK);
    }

    /* base */
    DrawRectangle(cx - 3, top + fh + 1, 8, 2, BLACK);
}

static void draw_mine(int x, int y, int size)
{
    int cx, cy, r, d;
    cx = x + size / 2;
    cy = y + size / 2;
    r = (size - 14) / 2;
    d = r * 7 / 10;
    DrawCircle(cx, cy, (float)r, BLACK);
    DrawCircle(cx, cy, (float)(r / 4), BLACK);
    DrawLine(cx - r + 2, cy, cx + r - 2, cy, WHITE);
    DrawLine(cx, cy - r + 2, cx, cy + r - 2, WHITE);
    DrawLine(cx - d, cy - d, cx + d, cy + d, WHITE);
    DrawLine(cx + d, cy - d, cx - d, cy + d, WHITE);
}

static void draw_smiley(Game *g, int x, int y, int size)
{
    int r, cx, cy;
    r = size / 2 - 1;
    cx = x + size / 2;
    cy = y + size / 2;

    DrawCircle(cx, cy, (float)r, YELLOW);
    DrawCircleLines(cx, cy, (float)r, BLACK);

    if (g->face == FACE_WON) {
        DrawRectangle(cx - r / 2 - 4, cy - r / 3 - 3, 8, 6, BLACK);
        DrawRectangle(cx + r / 2 - 4, cy - r / 3 - 3, 8, 6, BLACK);
        DrawLine(cx - r / 2 + 4, cy - r / 3, cx + r / 2 - 4, cy - r / 3, BLACK);
        DrawRectangle(cx - r / 3, cy + 2, r * 2 / 3, r / 2, BLACK);
        DrawRectangle(cx - r / 3 + 1, cy + 2 + 1, r * 2 / 3 - 2, r / 2 - 2, YELLOW);
    } else if (g->face == FACE_LOST) {
        DrawLine(cx - r / 2 - 3, cy - r / 3 - 3, cx - r / 2 + 3, cy - r / 3 + 3, BLACK);
        DrawLine(cx - r / 2 + 3, cy - r / 3 - 3, cx - r / 2 - 3, cy - r / 3 + 3, BLACK);
        DrawLine(cx + r / 2 - 3, cy - r / 3 - 3, cx + r / 2 + 3, cy - r / 3 + 3, BLACK);
        DrawLine(cx + r / 2 + 3, cy - r / 3 - 3, cx + r / 2 - 3, cy - r / 3 + 3, BLACK);
        DrawRectangle(cx - r / 3, cy - r / 3 - 3, r * 2 / 3, 3, BLACK);
    } else if (g->face == FACE_SURPRISE) {
        DrawCircle(cx - r / 3, cy - r / 3, (float)(r / 3), BLACK);
        DrawCircle(cx + r / 3, cy - r / 3, (float)(r / 3), BLACK);
        DrawCircle(cx, cy + r / 4, (float)(r / 3), BLACK);
    } else {
        DrawCircle(cx - r / 3, cy - r / 3, (float)(r / 5), BLACK);
        DrawCircle(cx + r / 3, cy - r / 3, (float)(r / 5), BLACK);
        {
            int mw, i;
            mw = r * 2 / 3;
            for (i = 0; i < 3; i++) {
                DrawRectangle(cx - mw / 2 + i, cy + i + 2, mw - i * 2, 1, BLACK);
            }
        }
    }
}

static Sound gen_tone(float freq, float duration, float vol)
{
    int sample_rate = 22050;
    int count = (int)(sample_rate * duration);
    short *buf;
    int i;

    if (count < 1) count = 1;
    buf = (short *)calloc((size_t)count, sizeof(short));
    if (buf == NULL) {
        Sound empty = { 0 };
        return empty;
    }
    for (i = 0; i < count; i++) {
        double t = (double)i / sample_rate;
        buf[i] = (short)(vol * 32767.0 * sin(2.0 * PI * freq * t));
    }
    {
        Wave w = { (unsigned int)count, (unsigned int)sample_rate, 16, 1, buf };
        Sound s = LoadSoundFromWave(w);
        UnloadWave(w);
        return s;
    }
}

static Sound gen_sweep(float freq_start, float freq_end, float duration, float vol)
{
    int sample_rate = 22050;
    int count = (int)(sample_rate * duration);
    short *buf;
    int i;

    if (count < 1) count = 1;
    buf = (short *)calloc((size_t)count, sizeof(short));
    if (buf == NULL) {
        Sound empty = { 0 };
        return empty;
    }
    for (i = 0; i < count; i++) {
        double t = (double)i / sample_rate;
        double ratio = t / duration;
        double freq = freq_start + (freq_end - freq_start) * ratio;
        buf[i] = (short)(vol * 32767.0 * sin(2.0 * PI * freq * t));
    }
    {
        Wave w = { (unsigned int)count, (unsigned int)sample_rate, 16, 1, buf };
        Sound s = LoadSoundFromWave(w);
        UnloadWave(w);
        return s;
    }
}

static Sound gen_arpeggio(float freqs[], int freq_count, float each_dur, float vol)
{
    int sample_rate = 22050;
    int per = (int)(sample_rate * each_dur);
    int count = per * freq_count;
    short *buf;
    int i, seg;

    if (count < 1) count = 1;
    buf = (short *)calloc((size_t)count, sizeof(short));
    if (buf == NULL) {
        Sound empty = { 0 };
        return empty;
    }
    for (seg = 0; seg < freq_count; seg++) {
        float freq = freqs[seg];
        int offset = seg * per;
        for (i = 0; i < per; i++) {
            double t = (double)i / sample_rate;
            float env = 1.0f;
            if (i < per / 8) env = (float)i / (per / 8);
            else if (i > per - per / 8) env = (float)(per - i) / (per / 8);
            buf[offset + i] = (short)(vol * env * 32767.0 * sin(2.0 * PI * freq * t));
        }
    }
    {
        Wave w = { (unsigned int)count, (unsigned int)sample_rate, 16, 1, buf };
        Sound s = LoadSoundFromWave(w);
        UnloadWave(w);
        return s;
    }
}

static int calc_cell_size(Game *g, int win_w, int win_h)
{
    int cs_w, cs_h, cs;
    float top_ratio = (float)BASE_TOP_PANEL / BASE_CELL;
    float bottom_ratio = (float)BASE_BOTTOM_BAR / BASE_CELL;

    cs_w = (win_w - BASE_PADDING * 2) / g->cols;
    cs_h = (int)((win_h - BASE_PADDING * 2) / (top_ratio + bottom_ratio + g->rows));
    cs = cs_w < cs_h ? cs_w : cs_h;
    if (cs < MIN_CELL) cs = MIN_CELL;
    return cs;
}

static int win_width_base(Game *g)
{
    return g->cols * BASE_CELL + BASE_PADDING * 2;
}

static int win_height_base(Game *g)
{
    return g->rows * BASE_CELL + BASE_TOP_PANEL + BASE_BOTTOM_BAR + BASE_PADDING * 2;
}

int main(void)
{
    Game game;
    Sound snd_reveal, snd_flag, snd_lose, snd_win;
    int screen_w, screen_h;
    float scale;
    int cell_size;
    int grid_x, grid_y, grid_w, grid_h;
    int header_y, header_h;
    int smiley_x, smiley_y, smiley_size;
    int counter_w, counter_h, counter_y;
    int db_y, db_h, db_w, db_gap;
    int bottom_bar_h;
    int restart_btn_x, restart_btn_y, restart_btn_w, restart_btn_h;
    int font_num, font_counter, font_diff;

    game_init(&game, DIFF_BEGINNER);

    screen_w = win_width_base(&game);
    screen_h = win_height_base(&game);

    InitWindow(screen_w, screen_h, "Minesweeper");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    InitAudioDevice();

    snd_reveal = gen_tone(800.0f, 0.025f, 0.6f);
    snd_flag   = gen_tone(600.0f, 0.030f, 0.5f);
    snd_lose   = gen_sweep(300.0f, 80.0f, 0.45f, 0.7f);
    {
        float wf[] = { 523.0f, 659.0f, 784.0f };
        snd_win = gen_arpeggio(wf, 3, 0.12f, 0.6f);
    }

    while (!WindowShouldClose()) {
        int mx, my, mr, mc;
        bool left_down, right_down;

        /* recalc layout each frame for responsive resize */
        {
            int win_w, win_h;
            win_w = GetScreenWidth();
            win_h = GetScreenHeight();
            cell_size = calc_cell_size(&game, win_w, win_h);
            scale = (float)cell_size / BASE_CELL;

            grid_w = game.cols * cell_size;
            grid_h = game.rows * cell_size;
            grid_x = (win_w - grid_w) / 2;
            if (grid_x < BASE_PADDING) grid_x = BASE_PADDING;
            grid_y = (int)(BASE_TOP_PANEL * scale) + BASE_PADDING;

            header_h = (int)(BASE_HEADER * scale);
            header_y = BASE_PADDING;

            smiley_size = header_h - 6;
            if (smiley_size < 20) smiley_size = 20;
            smiley_x = grid_x + grid_w / 2 - smiley_size / 2;
            smiley_y = header_y + (header_h - smiley_size) / 2;

            counter_w = (int)(50 * scale);
            counter_h = (int)(32 * scale);
            counter_y = header_y + (header_h - counter_h) / 2;

            db_h = (int)(22 * scale);
            if (db_h < 16) db_h = 16;
            db_w = (int)(84 * scale);
            if (db_w < 64) db_w = 64;
            db_y = header_y + header_h + 4;
            db_gap = (grid_w - 3 * db_w) / 4;
            if (db_gap < 2) db_gap = 2;

            bottom_bar_h = (int)(BASE_BOTTOM_BAR * scale);
            if (bottom_bar_h < 30) bottom_bar_h = 30;

            restart_btn_h = (int)(26 * scale);
            if (restart_btn_h < 18) restart_btn_h = 18;
            restart_btn_w = (int)(100 * scale);
            if (restart_btn_w < 70) restart_btn_w = 70;
            restart_btn_x = grid_x + (grid_w - restart_btn_w) / 2;
            restart_btn_y = grid_y + grid_h + (bottom_bar_h - restart_btn_h) / 2;

            font_num = (int)(18 * scale);
            if (font_num < 10) font_num = 10;
            font_counter = (int)(22 * scale);
            if (font_counter < 10) font_counter = 10;
            font_diff = (int)(14 * scale);
            if (font_diff < 8) font_diff = 8;
        }

        mx = GetMouseX();
        my = GetMouseY();
        mc = (mx - grid_x) / cell_size;
        mr = (my - grid_y) / cell_size;
        left_down = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        right_down = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);

        /* check if mouse is inside grid */
        if (mx > grid_x && mx < grid_x + grid_w &&
            my > grid_y && my < grid_y + grid_h &&
            mr >= 0 && mr < game.rows && mc >= 0 && mc < game.cols) {

            game.mouse_grid_x = mc;
            game.mouse_grid_y = mr;

            if (!game.game_over) {
                Cell *hover_cell;
                hover_cell = game_cell(&game, mr, mc);
                if ((left_down && !right_down && !hover_cell->revealed && !hover_cell->flagged) ||
                    (right_down && !left_down && !hover_cell->revealed) ||
                    (left_down && right_down)) {
                    game.face = FACE_SURPRISE;
                } else {
                    game.face = FACE_SMILE;
                }
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (right_down) {
                    game_chord(&game, mr, mc);
                    PlaySound(snd_reveal);
                } else {
                    game_reveal(&game, mr, mc);
                    if (game.game_over) {
                        PlaySound(game.won ? snd_win : snd_lose);
                    } else {
                        PlaySound(snd_reveal);
                    }
                }
            }
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                if (left_down) {
                    game_chord(&game, mr, mc);
                    PlaySound(snd_reveal);
                } else {
                    game_toggle_flag(&game, mr, mc);
                    PlaySound(snd_flag);
                }
            }
        } else {
            game.mouse_grid_x = -1;
            game.mouse_grid_y = -1;
            if (!game.game_over) {
                game.face = FACE_SMILE;
            }
        }

        if (!left_down && !right_down && !game.game_over) {
            game.face = FACE_SMILE;
        }

        /* smiley click */
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (mx >= smiley_x && mx <= smiley_x + smiley_size &&
                my >= smiley_y && my <= smiley_y + smiley_size) {
                game_reset(&game);
            }
        }

        /* difficulty switcher */
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int i;
            const char *labels[] = { "Beginner", "Intermed", "Expert" };
            for (i = 0; i < 3; i++) {
                int bx;
                bx = grid_x + db_gap + i * (db_w + db_gap);
                if (mx >= bx && mx <= bx + db_w &&
                    my >= db_y && my <= db_y + db_h) {
                    if (i != (int)game.difficulty) {
                        game_destroy(&game);
                        game_init(&game, (Difficulty)i);
                    }
                }
            }
            /* restart button */
            if (mx >= restart_btn_x && mx <= restart_btn_x + restart_btn_w &&
                my >= restart_btn_y && my <= restart_btn_y + restart_btn_h) {
                game_reset(&game);
            }
        }

        /* update */
        game_update_elapsed(&game);

        /* ---- render ---- */
        {
            int cur_w, cur_h;
            cur_w = GetScreenWidth();
            cur_h = GetScreenHeight();
            BeginDrawing();
            ClearBackground(Color{ 198, 198, 198, 255 });

            /* header panel */
            {
                int hx, hy, hw, hh;
                hx = grid_x;
                hy = header_y;
                hw = grid_w;
                hh = header_h;
                DrawRectangle(hx, hy, hw, hh, Color{ 212, 212, 212, 255 });
                DrawRectangle(hx, hy, hw, 2, WHITE);
                DrawRectangle(hx, hy, 2, hh, WHITE);
                DrawRectangle(hx, hy + hh - 2, hw, 2, GRAY);
                DrawRectangle(hx + hw - 2, hy, 2, hh, GRAY);

                /* mine counter (left) */
                {
                    int cx, cy, remaining;
                    char buf[8];
                    const char *label = "Mines:";
                    int label_w, label_h;

                    label_h = font_diff;
                    label_w = MeasureText(label, label_h);
                    cx = hx + 8 + label_w + 4;
                    cy = counter_y;
                    remaining = game.total_mines - game.flags_placed;
                    DrawRectangle(cx, cy, counter_w, counter_h, BLACK);
                    DrawRectangle(cx + 2, cy + 2, counter_w - 4, counter_h - 4,
                                 Color{ 40, 0, 0, 255 });
                    snprintf(buf, sizeof(buf), "%03d", remaining);
                    {
                        int tw;
                        tw = MeasureText(buf, font_counter);
                        DrawText(buf, cx + (counter_w - tw) / 2,
                                 cy + (counter_h - font_counter) / 2,
                                 font_counter, RED);
                    }
                    /* label */
                    DrawText(label, hx + 8, cy + (counter_h - label_h) / 2,
                             label_h, BLACK);
                }

                /* timer (right) */
                {
                    int cx, cy, elapsed_int;
                    char buf[8];
                    const char *label = "Time:";
                    int label_w, label_h;

                    label_h = font_diff;
                    label_w = MeasureText(label, label_h);
                    cx = hx + hw - 8 - counter_w;
                    cy = counter_y;
                    elapsed_int = (int)game.elapsed;
                    if (elapsed_int > 999) elapsed_int = 999;
                    DrawRectangle(cx, cy, counter_w, counter_h, BLACK);
                    DrawRectangle(cx + 2, cy + 2, counter_w - 4, counter_h - 4,
                                 Color{ 40, 0, 0, 255 });
                    snprintf(buf, sizeof(buf), "%03d", elapsed_int);
                    {
                        int tw;
                        tw = MeasureText(buf, font_counter);
                        DrawText(buf, cx + (counter_w - tw) / 2,
                                 cy + (counter_h - font_counter) / 2,
                                 font_counter, RED);
                    }
                    /* label to the left of the timer */
                    DrawText(label, cx - label_w - 4, cy + (counter_h - label_h) / 2,
                             label_h, BLACK);
                }
            }

            /* smiley */
            draw_smiley(&game, smiley_x, smiley_y, smiley_size);

            /* grid border */
            {
                DrawRectangle(grid_x, grid_y, grid_w, grid_h, Color{ 196, 196, 196, 255 });
                DrawRectangle(grid_x, grid_y, grid_w, 2, GRAY);
                DrawRectangle(grid_x, grid_y, 2, grid_h, GRAY);
                DrawRectangle(grid_x, grid_y + grid_h - 2, grid_w, 2, WHITE);
                DrawRectangle(grid_x + grid_w - 2, grid_y, 2, grid_h, WHITE);
            }

            /* cells */
            {
                int r, c;
                for (r = 0; r < game.rows; r++) {
                    for (c = 0; c < game.cols; c++) {
                        Cell *cell;
                        int cx, cy;
                        bool is_down, is_mouse_on;

                        cell = game_cell(&game, r, c);
                        cx = grid_x + c * cell_size;
                        cy = grid_y + r * cell_size;
                        is_mouse_on = (r == game.mouse_grid_y && c == game.mouse_grid_x);

                        if (cell->revealed) {
                            if (cell->mine && !game.won) {
                                DrawRectangle(cx, cy, cell_size, cell_size, RED);
                                draw_mine(cx, cy, cell_size);
                            } else {
                                draw_cell_sunken(cx, cy, cell_size);
                                if (cell->mine) {
                                    draw_mine(cx, cy, cell_size);
                                } else if (cell->adjacent > 0) {
                                    char num[2];
                                    int tw;
                                    num[0] = (char)('0' + cell->adjacent);
                                    num[1] = '\0';
                                    tw = MeasureText(num, font_num);
                                    DrawText(num, cx + (cell_size - tw) / 2,
                                             cy + (cell_size - font_num) / 2 + 1,
                                             font_num, num_colors[cell->adjacent]);
                                }
                            }
                        } else {
                            is_down = false;
                            if (is_mouse_on && !game.game_over) {
                                if (left_down && !right_down && !cell->flagged) {
                                    is_down = true;
                                } else if (right_down && !left_down) {
                                    is_down = true;
                                } else if (left_down && right_down) {
                                    is_down = true;
                                }
                            }

                            if (is_down) {
                                DrawRectangle(cx, cy, cell_size, cell_size,
                                             Color{ 176, 176, 176, 255 });
                                draw_cell_sunken(cx, cy, cell_size);
                                if (cell->flagged) {
                                    draw_flag(cx, cy, cell_size);
                                }
                            } else {
                                draw_cell_raised(cx, cy, cell_size);
                                if (cell->flagged) {
                                    draw_flag(cx, cy, cell_size);
                                }
                            }

                            /* wrong flag on game over */
                            if (game.game_over && game.won == false &&
                                cell->flagged && !cell->mine) {
                                int tw;
                                DrawRectangle(cx, cy, cell_size, cell_size,
                                             Color{ 188, 188, 188, 255 });
                                draw_cell_sunken(cx, cy, cell_size);
                                draw_flag(cx, cy, cell_size);
                                tw = MeasureText("X", font_num);
                                DrawText("X", cx + (cell_size - tw) / 2,
                                         cy + (cell_size - font_num) / 2 + 1,
                                         font_num, RED);
                            }
                        }
                    }
                }
            }

            /* difficulty buttons */
            {
                int i;
                const char *labels[] = { "Beginner", "Intermed", "Expert" };

                for (i = 0; i < 3; i++) {
                    int bx;
                    bx = grid_x + db_gap + i * (db_w + db_gap);

                    if (i == (int)game.difficulty) {
                        draw_btn_sunken(bx, db_y, db_w, db_h);
                    } else {
                        draw_btn_raised(bx, db_y, db_w, db_h);
                    }

                    {
                        int tw;
                        tw = MeasureText(labels[i], font_diff);
                        DrawText(labels[i], bx + (db_w - tw) / 2,
                                 db_y + (db_h - font_diff) / 2, font_diff, BLACK);
                    }
                }
            }

            /* restart button */
            draw_btn_raised(restart_btn_x, restart_btn_y, restart_btn_w, restart_btn_h);
            {
                int tw;
                const char *label = "New Game";
                tw = MeasureText(label, font_diff);
                DrawText(label, restart_btn_x + (restart_btn_w - tw) / 2,
                         restart_btn_y + (restart_btn_h - font_diff) / 2, font_diff, BLACK);
            }

            /* game-over overlay */
            if (game.game_over) {
                Color overlay;
                const char *title;
                Color title_color;
                int title_size, tw, y_center;
                char info_buf[64];
                int info_size, iw;

                overlay = Color{ 0, 0, 0, 160 };
                DrawRectangle(grid_x, grid_y, grid_w, grid_h, overlay);

                y_center = grid_y + grid_h / 2;
                title = game.won ? "YOU WIN!" : "GAME OVER";
                title_color = game.won ? GOLD : RED;
                title_size = (int)(cell_size * 1.2f);
                if (title_size < 20) title_size = 20;
                tw = MeasureText(title, title_size);
                DrawText(title, grid_x + (grid_w - tw) / 2,
                         y_center - title_size - 5, title_size, title_color);

                snprintf(info_buf, sizeof(info_buf), "Time: %d s", (int)game.elapsed);
                info_size = (int)(cell_size * 0.5f);
                if (info_size < 12) info_size = 12;
                iw = MeasureText(info_buf, info_size);
                DrawText(info_buf, grid_x + (grid_w - iw) / 2,
                         y_center + 5, info_size, WHITE);

                {
                    const char *hint = "Click face to restart";
                    int hint_size, hw;
                    hint_size = (int)(cell_size * 0.4f);
                    if (hint_size < 10) hint_size = 10;
                    hw = MeasureText(hint, hint_size);
                    DrawText(hint, grid_x + (grid_w - hw) / 2,
                             y_center + info_size + 12, hint_size, WHITE);
                }
            }

            EndDrawing();
        }
    }

    game_destroy(&game);
    UnloadSound(snd_reveal);
    UnloadSound(snd_flag);
    UnloadSound(snd_lose);
    UnloadSound(snd_win);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

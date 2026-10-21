#include "tetris.h"
#include "main.h"
#include "cmsis_os.h"
#include "oled.h"
#include "kb.h"
#include <stdint.h>
#include <stdio.h>

#define BW 10
#define BH 16
#define CELL 4
#define LEFT 2
#define TOP 0
#define FALL_MS 650U
#define FAST_MS 75U

static uint8_t board[BH][BW];
static int px, py, kind, rotation;
static uint32_t score, lines, seed;

typedef enum {
    SCREEN_MENU,
    SCREEN_PLAYING,
    SCREEN_PAUSED,
    SCREEN_FINISHED
} ScreenState;

static volatile ScreenState screen = SCREEN_MENU;
static uint32_t record_score; /* RAM only */
static uint32_t last_fall;
static osMessageQId key_queue;
static osMutexId game_mutex;
static osMutexId i2c_mutex;
static volatile uint8_t soft_drop_held;

static const uint16_t shapes[7] = {
    0x0F00, /* I */
    0x0660, /* O */
    0x0270, /* T */
    0x0360, /* S */
    0x0630, /* Z */
    0x0170, /* J */
    0x0470  /* L */
};

static uint8_t shape_at(int type, int rot, int x, int y)
{
    int sx = x, sy = y;
    switch (rot & 3) {
        case 1: sx = y;     sy = 3 - x; break;
        case 2: sx = 3 - x; sy = 3 - y; break;
        case 3: sx = 3 - y; sy = x;     break;
        default: break;
    }
    return (uint8_t)((shapes[type] >> (15 - (sy * 4 + sx))) & 1U);
}

static uint8_t fits(int nx, int ny, int nr)
{
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x) {
            if (!shape_at(kind, nr, x, y)) continue;
            int bx = nx + x, by = ny + y;
            if (bx < 0 || bx >= BW || by >= BH) return 0;
            if (by >= 0 && board[by][bx]) return 0;
        }
    return 1;
}

static void finish_game(void)
{
    if (score > record_score) record_score = score;
    soft_drop_held = 0;
    screen = SCREEN_FINISHED;
}

static void spawn(void)
{
    seed = seed * 1664525U + 1013904223U;
    kind = (int)((seed >> 16) % 7U);
    rotation = 0;
    px = 3;
    py = -1;
    if (!fits(px, py, rotation)) finish_game();
}

static void restart(void)
{
    for (int y = 0; y < BH; ++y)
        for (int x = 0; x < BW; ++x)
            board[y][x] = 0;
    soft_drop_held = 0;
    score = 0;
    lines = 0;
    spawn();
    last_fall = osKernelSysTick();
}

static uint8_t free_piece_id(void)
{
    uint8_t used[255] = {0};
    for (int y = 0; y < BH; ++y)
        for (int x = 0; x < BW; ++x)
            used[board[y][x]] = 1;
    for (int id = 1; id < 255; ++id)
        if (!used[id]) return (uint8_t)id;
    return 0;
}

static void lock_piece(void)
{
    uint8_t id = free_piece_id();
    if (!id) { finish_game(); return; }

    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            if (shape_at(kind, rotation, x, y) && py + y < 0) {
                finish_game();
                return;
            }

    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x) {
            if (!shape_at(kind, rotation, x, y)) continue;
            int bx = px + x, by = py + y;
            if (bx >= 0 && bx < BW && by >= 0 && by < BH)
                board[by][bx] = id;
        }

    int cleared = 0;
    for (int y = BH - 1; y >= 0; --y) {
        int full = 1;
        for (int x = 0; x < BW; ++x)
            if (!board[y][x]) { full = 0; break; }
        if (!full) continue;
        ++cleared;
        for (int row = y; row > 0; --row)
            for (int x = 0; x < BW; ++x)
                board[row][x] = board[row - 1][x];
        for (int x = 0; x < BW; ++x) board[0][x] = 0;
        ++y;
    }
    lines += (uint32_t)cleared;
    score += (uint32_t)(cleared * cleared * 100);
    spawn();
}

static void fall(void)
{
    if (fits(px, py + 1, rotation)) ++py;
    else lock_piece();
}

static void text_at(uint8_t x, uint8_t y, const char *text)
{
    oled_SetCursor(x, y);
    oled_WriteString((char *)text, Font_7x10, White);
}

static void render(void)
{
    static uint8_t picture[BH][BW];
    char text[24];
    oled_Fill(Black);

    if (screen == SCREEN_MENU) {
        text_at(42, 3, "TETRIS");
        text_at(22, 23, "5 - START");
        snprintf(text, sizeof(text), "BEST %lu", (unsigned long)record_score);
        text_at(10, 43, text);
    } else if (screen == SCREEN_PAUSED) {
        text_at(45, 1, "PAUSE");
        snprintf(text, sizeof(text), "SCORE %lu", (unsigned long)score);
        text_at(4, 14, text);
        snprintf(text, sizeof(text), "LINES %lu", (unsigned long)lines);
        text_at(4, 26, text);
        text_at(4, 40, "5 - RESUME");
        text_at(4, 52, "# - END");
    } else if (screen == SCREEN_FINISHED) {
        text_at(24, 1, "GAME OVER");
        snprintf(text, sizeof(text), "SCORE %lu", (unsigned long)score);
        text_at(4, 14, text);
        snprintf(text, sizeof(text), "LINES %lu", (unsigned long)lines);
        text_at(4, 26, text);
        snprintf(text, sizeof(text), "BEST %lu", (unsigned long)record_score);
        text_at(4, 38, text);
        text_at(4, 52, "# - MENU");
    } else {
        for (int y = 0; y < BH; ++y)
            for (int x = 0; x < BW; ++x)
                picture[y][x] = board[y][x];

        for (int y = 0; y < 4; ++y)
            for (int x = 0; x < 4; ++x) {
                if (!shape_at(kind, rotation, x, y)) continue;
                int bx = px + x, by = py + y;
                if (bx >= 0 && bx < BW && by >= 0 && by < BH)
                    picture[by][bx] = 255U;
            }


        for (int y = 0; y < BH; ++y)
            for (int x = 0; x < BW; ++x) {
                int sx = LEFT + x * CELL, sy = TOP + y * CELL;
                uint8_t id = picture[y][x];
                if (!id) {
                    oled_DrawPixel((uint8_t)(sx + 1), (uint8_t)(sy + 1), White);
                    continue;
                }
                for (int dy = 0; dy < CELL - 1; ++dy)
                    for (int dx = 0; dx < CELL - 1; ++dx)
                        oled_DrawPixel((uint8_t)(sx + dx),
                                       (uint8_t)(sy + dy), White);
                if (x + 1 < BW && picture[y][x + 1] == id)
                    for (int dy = 0; dy < CELL - 1; ++dy)
                        oled_DrawPixel((uint8_t)(sx + CELL - 1),
                                       (uint8_t)(sy + dy), White);
                if (y + 1 < BH && picture[y + 1][x] == id)
                    for (int dx = 0; dx < CELL - 1; ++dx)
                        oled_DrawPixel((uint8_t)(sx + dx),
                                       (uint8_t)(sy + CELL - 1), White);
                if (x + 1 < BW && y + 1 < BH &&
                    picture[y][x + 1] == id &&
                    picture[y + 1][x] == id &&
                    picture[y + 1][x + 1] == id)
                    oled_DrawPixel((uint8_t)(sx + CELL - 1),
                                   (uint8_t)(sy + CELL - 1), White);
            }

        for (int y = 0; y < 64; ++y)
            oled_DrawPixel((uint8_t)(LEFT + BW * CELL + 1), (uint8_t)y, White);

        text_at(47, 0, "TETRIS");
        snprintf(text, sizeof(text), "S:%lu", (unsigned long)score);
        text_at(47, 11, text);
        snprintf(text, sizeof(text), "L:%lu", (unsigned long)lines);
        text_at(47, 22, text);
        text_at(47, 33, "4/6 MOVE");
        text_at(47, 44, "5ROT 8FAST");
        text_at(47, 54, "*PAUSE #END");
    }
    oled_UpdateScreen();
}

void Tetris_Init(void)
{
    osMessageQDef(tetrisKeys, 16, uint32_t);
    key_queue = osMessageCreate(osMessageQ(tetrisKeys), NULL);
    osMutexDef(tetrisLock);
    game_mutex = osMutexCreate(osMutex(tetrisLock));
    osMutexDef(tetrisI2C);
    i2c_mutex = osMutexCreate(osMutex(tetrisI2C));
    seed = 12345U;
    restart();
    screen = SCREEN_MENU;
}

void Tetris_InputTask(void const *argument)
{
    (void)argument;
    char previous = 0;
    if (i2c_mutex && osMutexWait(i2c_mutex, osWaitForever) == osOK) {
        Set_Keyboard();
        osMutexRelease(i2c_mutex);
    }
    for (;;) {
        char key = 0;
        if (i2c_mutex && osMutexWait(i2c_mutex, osWaitForever) == osOK) {
            key = Get_Key();
            osMutexRelease(i2c_mutex);
        }
        soft_drop_held = (uint8_t)(key == '8' && screen == SCREEN_PLAYING);
        if (key != previous && key != 0 && key_queue)
            (void)osMessagePut(key_queue, (uint32_t)(uint8_t)key, 0);
        previous = key;
        osDelay(15);
    }
}

void Tetris_GameTask(void const *argument)
{
    (void)argument;
    uint8_t previous_fast = 0;
    for (;;) {
        uint32_t now = osKernelSysTick();
        if (game_mutex && osMutexWait(game_mutex, osWaitForever) == osOK) {
            osEvent event;
            while (key_queue && (event = osMessageGet(key_queue, 0)).status == osEventMessage) {
                char key = (char)event.value.v;
                seed += (uint32_t)(uint8_t)key + now;
                if (screen == SCREEN_MENU) {
                    if (key == '5') { restart(); screen = SCREEN_PLAYING; }
                } else if (screen == SCREEN_PLAYING) {
                    if (key == '#') finish_game();
                    else if (key == '*') screen = SCREEN_PAUSED;
                    else if (key == '4' && fits(px - 1, py, rotation)) --px;
                    else if (key == '6' && fits(px + 1, py, rotation)) ++px;
                    else if (key == '5') {
                        int nr = (rotation + 1) & 3;
                        if (fits(px, py, nr)) rotation = nr;
                        else if (fits(px - 1, py, nr)) { --px; rotation = nr; }
                        else if (fits(px + 1, py, nr)) { ++px; rotation = nr; }
                    }
                } else if (screen == SCREEN_PAUSED) {
                    if (key == '5' || key == '*') {
                        screen = SCREEN_PLAYING;
                        last_fall = now;
                    }
                    else if (key == '#') finish_game();
                } else if (screen == SCREEN_FINISHED) {
                    if (key == '#') screen = SCREEN_MENU;
                }
            }
            if (screen == SCREEN_PLAYING) {
                uint8_t fast = soft_drop_held;
                if (fast != previous_fast) last_fall = now;
                if ((uint32_t)(now - last_fall) >=
                    (fast ? FAST_MS : FALL_MS)) {
                    fall();
                    last_fall = now;
                }
                previous_fast = fast;
            } else {
                last_fall = now;
                previous_fast = 0;
            }
            osMutexRelease(game_mutex);
        }
        osDelay(10);
    }
}

void Tetris_DisplayTask(void const *argument)
{
    (void)argument;
    if (i2c_mutex && osMutexWait(i2c_mutex, osWaitForever) == osOK) {
        oled_Init();
        osMutexRelease(i2c_mutex);
    }
    for (;;) {
        if (game_mutex && osMutexWait(game_mutex, osWaitForever) == osOK) {
            if (i2c_mutex && osMutexWait(i2c_mutex, osWaitForever) == osOK) {
                render();
                osMutexRelease(i2c_mutex);
            }
            osMutexRelease(game_mutex);
        }
        osDelay(150);
    }
}

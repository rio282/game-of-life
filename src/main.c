#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnusedParameter"
#pragma ide diagnostic ignored "OCUnusedMacroInspection"

#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>
#include <time.h>
#include "generators/noise.h"


#define MAX_COLS 32
#define MAX_ROWS 24
#define CELL_SIZE 32
#define GRID_SIZE (MAX_ROWS * MAX_COLS)
#define STROKE_WIDTH 1
#define REAL_CELL_SIZE (CELL_SIZE + STROKE_WIDTH)

#define WIN_TITLE "Game of Life"
#define WIN_WIDTH (MAX_COLS * CELL_SIZE + MAX_COLS * STROKE_WIDTH)
#define WIN_HEIGHT (MAX_ROWS * CELL_SIZE + MAX_ROWS * STROKE_WIDTH)
#define TICK_RATE 30
#define TICK_DELAY (1000/TICK_RATE)


static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static char title[256] = WIN_TITLE;
static bool paused = false;
static bool draw_grid = true;

bool cells[GRID_SIZE] = {0};
bool next[GRID_SIZE] = {0};

Uint32 generation;
size_t population;
char generation_str[256] = {0};
char population_str[256] = {0};
char tickrate_str[256] = {0};

const int n_directions[8][2] = {
        {-1, -1}, // Top-left
        {-1, 0},  // Top
        {-1, 1},  // Top-right
        {0,  -1}, // Left
        {0,  1},  // Right
        {1,  -1}, // Bottom-left
        {1,  0},  // Bottom
        {1,  1}   // Bottom-right
};

void GetNeighbours(bool *neighbours[8], size_t idx) {
    size_t row = idx / MAX_COLS;
    size_t col = idx % MAX_COLS;

    for (int i = 0, n = 0; i < 8; ++i) {
        size_t new_row = row + n_directions[i][0];
        size_t new_col = col + n_directions[i][1];
        neighbours[n++] = &cells[new_row * MAX_COLS + new_col];
    }
}

char GetAliveCountNeighbours(size_t idx) {
    // get neighbours
    bool *neighbours[8] = {0};
    GetNeighbours(neighbours, idx);

    // count alive
    char alive = 0;
    for (int i = 0; i < 8; ++i) if (neighbours[i] && *neighbours[i]) ++alive;
    return alive;
}

void UpdateCells() {

    size_t idx;
    char n_alive;
    population = 0;

    for (size_t row = 0; row < MAX_ROWS; ++row) {
        for (size_t col = 0; col < MAX_COLS; ++col) {

            idx = row * MAX_COLS + col;
            n_alive = GetAliveCountNeighbours(idx);

            if (cells[idx]) {
                /*
                 * Any live cell with fewer than two live neighbours dies, as if by underpopulation.
                 * Any live cell with two or three live neighbours lives on to the next generation.
                 * Any live cell with more than three live  neighbours dies, as if by overpopulation.
                 */
                next[idx] = !(n_alive < 2 || n_alive > 3);
            } else {
                /* Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction. */
                next[idx] = n_alive == 3;
            }

            if (next[idx]) population++;

        }
    }

    // move next into current
    for (size_t i = 0; i < GRID_SIZE; ++i) {
        cells[i] = next[i];
    }

    // update generation
    generation++;

}

void DrawGrid() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_FRect rect;
    for (size_t row = 0; row < MAX_ROWS; ++row) {
        for (size_t col = 0; col < MAX_COLS; ++col) {
            rect = (SDL_FRect) {
                    .x = (float) col * (CELL_SIZE + STROKE_WIDTH),
                    .y = (float) row * (CELL_SIZE + STROKE_WIDTH),
                    .w = CELL_SIZE,
                    .h = CELL_SIZE
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void DrawCells() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FRect rect;
    for (size_t row = 0; row < MAX_ROWS; ++row) {
        for (size_t col = 0; col < MAX_COLS; ++col) {
            if (cells[row * MAX_COLS + col]) {
                rect = (SDL_FRect) {
                        .x = (float) col * REAL_CELL_SIZE,
                        .y = (float) row * REAL_CELL_SIZE,
                        .w = (float) CELL_SIZE,
                        .h = (float) CELL_SIZE
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void DrawInformationText() {

    if (paused) {
        SDL_RenderDebugText(renderer, CELL_SIZE / 2.0, CELL_SIZE / 2.0, "Paused");
        return;
    }

    snprintf(tickrate_str, sizeof(tickrate_str), "Tickrate: %d", TICK_RATE);
    snprintf(generation_str, sizeof(generation_str), "Generation: %d", generation);
    snprintf(population_str, sizeof(population_str), "Population: %zu", population);

    SDL_RenderDebugText(renderer, CELL_SIZE / 2.0, CELL_SIZE / 2.0, tickrate_str);
    SDL_RenderDebugText(renderer, CELL_SIZE / 2.0, CELL_SIZE, generation_str);
    SDL_RenderDebugText(renderer, CELL_SIZE / 2.0, CELL_SIZE + (CELL_SIZE / 2.0), population_str);

}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc51-cpp"

void GenerateRandomPattern() {
    float threshold = 0.0f;

    srand(time(NULL));
    InitPermutationTable();

    // keep generating until we get a non-empty map, or we're reached the max amount of attempts
    for (unsigned char attempts = 0, max_attempts = 255; attempts < max_attempts; ++attempts) {
        GenerateBinaryNoise(MAX_COLS, MAX_ROWS, cells, threshold);
        for (size_t i = 0; i < GRID_SIZE; ++i) if (cells[i]) return;
    }
    SDL_Log("Failed to generate a map... Please try again by pressing [R].");
}

#pragma clang diagnostic pop

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't initialize SDL!", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
            title, WIN_WIDTH, WIN_HEIGHT,
            SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS,
            &window, &renderer)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create window/renderer!", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    GenerateRandomPattern();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_UP) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                return SDL_APP_SUCCESS;
            case SDL_SCANCODE_SPACE:
                paused = !paused;
                snprintf(title, sizeof(title), "%s%s", WIN_TITLE, paused ? " (Paused)" : "");
                SDL_SetWindowTitle(window, title);
                break;
            case SDL_SCANCODE_R:
                GenerateRandomPattern();
                generation = 0;
                break;
            case SDL_SCANCODE_C:
                for (size_t i = 0; i < GRID_SIZE; ++i) cells[i] = false;
                generation = 0;
                break;
            case SDL_SCANCODE_G:
                draw_grid = !draw_grid;
                break;
            default:
                break;
        }
    }

    if (paused && event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
        float x, y;
        SDL_GetMouseState(&x, &y);

        const size_t col = (size_t) (x / REAL_CELL_SIZE);
        const size_t row = (size_t) (y / REAL_CELL_SIZE);

        if (col < MAX_COLS && row < MAX_ROWS) {
            const size_t idx = row * MAX_COLS + col;
            cells[idx] = !cells[idx];
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    // update
    if (!paused) {
        UpdateCells();
    }

    // render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (draw_grid) DrawGrid();
    DrawCells();
    DrawInformationText();

    SDL_RenderPresent(renderer);
    SDL_Delay(TICK_DELAY);

    // continue happily
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    // nothing rn...
}

#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnusedParameter"
#pragma ide diagnostic ignored "OCUnusedMacroInspection"

#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


#define MAX_COLS 32
#define MAX_ROWS 24
#define CELL_SIZE 32
#define GRID_SIZE (MAX_ROWS * MAX_COLS)
#define STROKE_WIDTH 1
#define REAL_CELL_SIZE (CELL_SIZE + STROKE_WIDTH)

#define WIN_TITLE "Game of Life"
#define WIN_WIDTH (MAX_COLS * CELL_SIZE + MAX_COLS * STROKE_WIDTH)
#define WIN_HEIGHT (MAX_ROWS * CELL_SIZE + MAX_ROWS * STROKE_WIDTH)
#define TICK_RATE 60
#define TICK_DELAY (1.0/TICK_RATE)

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static bool paused = false;

bool cells[GRID_SIZE] = {0};


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


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't initialize SDL!", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
            WIN_TITLE, WIN_WIDTH, WIN_HEIGHT,
            SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS,
            &window, &renderer)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create window/renderer!", SDL_GetError(), NULL);
        return SDL_APP_FAILURE;
    }

    cells[0] = 1;
    cells[1] = 1;
    cells[2] = 1;

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
                SDL_Log("Paused: %s", paused ? "true" : "false");
                break;
            default:
                break;
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && event->button.button == SDL_BUTTON_LEFT) {
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
    if (paused) return SDL_APP_CONTINUE;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    DrawGrid();
    DrawCells();

    SDL_RenderPresent(renderer);
    SDL_Delay(TICK_DELAY);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    // nothing rn...
}

#pragma clang diagnostic pop
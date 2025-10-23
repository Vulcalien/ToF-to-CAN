/* Copyright 2025 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "display.h"

#include <SDL.h>
#include <SDL_ttf.h>

#define CELL_WIDTH 80
#define CELL_HEIGHT CELL_WIDTH
#define WINDOW_WIDTH  (CELL_WIDTH * 8)
#define WINDOW_HEIGHT (CELL_HEIGHT * 8)

#define GRID_XOFF 0
#define GRID_YOFF 0

static SDL_Window *window;
static SDL_Renderer *renderer;

static TTF_Font *font;

int display_init(void) {
    if(SDL_Init(SDL_INIT_VIDEO)) {
        printf("[Display] SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    if(TTF_Init()) {
        printf("[Display] TTF_Init: %s\n", TTF_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "ToF Visualizer",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN
    );
    if(!window) {
        printf("[Display] SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        printf("[Display] SDL_CreateRenderer: %s\n", SDL_GetError());
        return 1;
    }

    // automatically scale-and-fit
    SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    font = TTF_OpenFont("res/DejaVuLGCSansMono.ttf", 20);
    if(!font) {
        printf("[Display] TTF_OpenFont: %s\n", TTF_GetError());
        return 1;
    }

    SDL_ShowWindow(window);
    return 0;
}

int display_tick(void) {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT)
            return 1;
    }
    return 0;
}

void display_refresh(void) {
    SDL_RenderPresent(renderer);

    // clear window
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

static SDL_Color choose_fg_color(SDL_Color bg) {
    double bg_l = (bg.r * 0.299 + bg.g * 0.587 + bg.b * 0.114);
    if(bg_l > 128)
        return (SDL_Color) { 0, 0, 0 };
    else
        return (SDL_Color) { 255, 255, 255 };
}

static void write_value(int val, int bg_color, int x, int y) {
    char text[16];
    snprintf(text, sizeof(text), "%d", val);

    SDL_Color bg = { bg_color >> 16, bg_color >> 8, bg_color };
    SDL_Color fg = choose_fg_color(bg);

    SDL_Surface *surface = TTF_RenderText_Shaded(font, text, fg, bg);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {
        x - surface->w / 2,
        y - surface->h / 2,
        surface->w,
        surface->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int display_update(int16_t values[64], int32_t colors[64]) {
    for(int i = 0; i < 64; i++) {
        int val = values[i];
        int col = colors[i];

        int x = i % 8;
        int y = i / 8;

        // draw rectangle
        SDL_SetRenderDrawColor(renderer, col >> 16, col >> 8, col, 255);
        SDL_Rect rect = {
            GRID_XOFF + x * CELL_WIDTH,
            GRID_YOFF + y * CELL_HEIGHT,
            CELL_WIDTH,
            CELL_HEIGHT
        };
        SDL_RenderFillRect(renderer, &rect);

        // write value number
        write_value(
            val, col,
            GRID_XOFF + x * CELL_WIDTH  + CELL_WIDTH  / 2,
            GRID_YOFF + y * CELL_HEIGHT + CELL_HEIGHT / 2
        );
    }

    // draw grid lines
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
    for(int i = 0; i < 8; i++) {
        SDL_Rect vertical = {
            GRID_XOFF + i * CELL_WIDTH,
            GRID_YOFF,
            1,
            CELL_HEIGHT * 8
        };

        SDL_Rect horizontal = {
            GRID_XOFF,
            GRID_YOFF + i * CELL_HEIGHT,
            CELL_WIDTH * 8,
            1
        };

        SDL_RenderFillRect(renderer, &vertical);
        SDL_RenderFillRect(renderer, &horizontal);
    }
    return 0;
}

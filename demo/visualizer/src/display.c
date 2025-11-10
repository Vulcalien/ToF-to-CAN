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

#include "view.h"

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
        DISPLAY_WIDTH, DISPLAY_HEIGHT,
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
    SDL_RenderSetLogicalSize(renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    font = TTF_OpenFont("res/DejaVuLGCSansMono.ttf", 19);
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

void display_update(void) {
    if(view->update(renderer, font)) {
        SDL_RenderPresent(renderer);

        // clear window
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }
}

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
    if(view->update(renderer)) {
        SDL_RenderPresent(renderer);

        // clear window
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }
}

static inline SDL_Color choose_fg_color(SDL_Color bg) {
    double bg_l = (bg.r * 0.299 + bg.g * 0.587 + bg.b * 0.114);
    if(bg_l > 128)
        return (SDL_Color) { 0, 0, 0 };
    else
        return (SDL_Color) { 255, 255, 255 };
}

void display_write(const char *text, int bg_color, int xc, int yc) {
    SDL_Color bg = { bg_color >> 16, bg_color >> 8, bg_color };
    SDL_Color fg = choose_fg_color(bg);

    SDL_Surface *surface = TTF_RenderText_Shaded(font, text, fg, bg);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {
        xc - surface->w / 2,
        yc - surface->h / 2,
        surface->w,
        surface->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

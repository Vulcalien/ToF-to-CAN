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
#include "view.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include "libtofcan.h"
#include "can-io.h"

#define CELL_WIDTH 75
#define CELL_HEIGHT CELL_WIDTH

#define GRID_XOFF 0
#define GRID_YOFF 0

static int shown_sensor;

static int grid_init(void) {
    shown_sensor = 0;

    return 0;
}

static inline int cell_color(int value, int min, int max) {
    int gray = 255 - (255 * (value - min) / max);
    return (gray << 16 | gray << 8 | gray);
}

static SDL_Color choose_fg_color(SDL_Color bg) {
    double bg_l = (bg.r * 0.299 + bg.g * 0.587 + bg.b * 0.114);
    if(bg_l > 128)
        return (SDL_Color) { 0, 0, 0 };
    else
        return (SDL_Color) { 255, 255, 255 };
}

static void write_value(SDL_Renderer *renderer, TTF_Font *font,
                        int val, int bg_color, int x, int y) {
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

static bool grid_update(SDL_Renderer *renderer, TTF_Font *font) {
    int sensor;
    struct libtofcan_batch batch;
    if(can_io_get_data(&sensor, &batch))
        return false;

    // get min and max distances
    int min = batch.data[0];
    int max = min;
    for(int i = 1; i < 64; i++) {
        int point = batch.data[i];
        if(min > point)
            min = point;
        if(max < point)
            max = point;
    }

    // draw cells
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            int value = batch.data[x + y * 8];
            int color = cell_color(value, min, max);

            // draw rectangle
            SDL_SetRenderDrawColor(
                renderer, color >> 16, color >> 8, color, 255
            );
            SDL_Rect rect = {
                GRID_XOFF + x * CELL_WIDTH,
                GRID_YOFF + y * CELL_HEIGHT,
                CELL_WIDTH,
                CELL_HEIGHT
            };
            SDL_RenderFillRect(renderer, &rect);

            // write value number
            write_value(
                renderer, font, value, color,
                GRID_XOFF + x * CELL_WIDTH  + CELL_WIDTH  / 2,
                GRID_YOFF + y * CELL_HEIGHT + CELL_HEIGHT / 2
            );
        }
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

    return true;
}

const struct View view_grid = {
    .init = grid_init,
    .update = grid_update
};

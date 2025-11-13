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
#pragma once

#include "visualizer.h"

#include <SDL.h>

struct View {
    int (*init)(void);

    // returns true if the display should be refreshed
    bool (*update)(SDL_Renderer *renderer);
};

extern const struct View *view;

extern const struct View
    view_grid,
    view_ring;

extern int view_set(const struct View *new_view);

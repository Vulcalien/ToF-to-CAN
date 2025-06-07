// SPDX-License-Identifier: GPL-3.0-or-later

/* Copyright 2025 Rocco Rusponi
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

#include "main.h"

#include <pthread.h>
#include <semaphore.h>

#define PROCESSING_DATA_MAX_LENGTH 64

extern pthread_mutex_t processing_data_mutex;
extern sem_t           processing_data_available;
extern int             processing_data_length;

extern int16_t processing_data[PROCESSING_DATA_MAX_LENGTH];
extern bool    processing_below_threshold;
extern bool    processing_threshold_event;

extern int processing_init(void);
extern int processing_start(void);

extern void processing_pause(void);
extern void processing_resume(void);

extern int processing_set_mode(int mode);
extern int processing_set_threshold(int threshold);
extern int processing_set_threshold_delay(int delay);

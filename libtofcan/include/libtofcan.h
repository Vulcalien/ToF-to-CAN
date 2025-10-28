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

#include <stdint.h>
#include <stdbool.h>

struct tofcan_sample {
    int16_t distance;
    bool below_threshold;
}

struct tofcan_batch {
    int16_t data[64];
    int data_length;
    int batch_id;
};

struct tofcan_driver {
    int (*init)(void);
    int (*read)(uint32_t *id, bool *rtr, void *data, int *len);
    int (*write)(uint32_t id, bool rtr, void *data, int len);
};

extern const struct tofcan_driver_linux;
extern const struct tofcan_driver_nuttx;

/*
 * int tofcan_init(const struct tofcan_driver *driver)
 *
 * Initialize the library and select the specified CAN driver.
 */
extern int tofcan_init(const struct tofcan_driver *driver);

/*
 * void tofcan_request_sample(int sensor_id)
 *
 * Requests a sample or data batch from the specified sensor.
 */
extern void tofcan_request_sample(int sensor_id);

/*
 * void *tofcan_receiver(void *)
 *
 * Main function of the receiver thread. This function never returns.
 * When a sample or data batch is received, the configured callback
 * functions are called.
 */
extern void *tofcan_receiver(void *);

/*
 * void tofcan_set_callbacks(int (*sample)(...), int (*batch)(...))
 *
 * Sets the callback functions that are called when the receiver obtains
 * a sample or full data batch. If a callback function is NULL, no
 * action is performed.
 *
 * Note that the data pointer's lifetime expires when the callback
 * function returns. DO NOT dereference the pointer outside the callback
 * function's scope. Copy the data instead.
 */
extern void tofcan_set_callbacks(
    int (*sample)(int sensor_id, struct tofcan_sample *data),
    int (*batch) (int sensor_id, struct tofcan_batch  *data)
);

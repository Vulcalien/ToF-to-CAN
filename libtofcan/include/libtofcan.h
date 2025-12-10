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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "tof2can.h"

struct libtofcan_sample {
    int16_t distance;
    bool below_threshold;
};

struct libtofcan_batch {
    int16_t data[64];
    int data_length;

    int batch_id;
    int packets_received;
    int packets_expected;
};

/*
 * Description of a CAN message.
 */
struct libtofcan_msg {
    uint32_t id;
    bool     rtr;
    int      len;
    uint8_t  data[8];
};

/*
 * Sets the callback functions to be called when the receiver obtains a
 * sample or data batch. If the corresponding callback function is NULL,
 * data will instead be discarded.
 *
 * Note that the data pointer's lifetime expires when the callback
 * function returns. DO NOT dereference the pointer outside the callback
 * function's scope. Copy the data instead.
 */
extern void libtofcan_set_callbacks(
    void (*sample)(int sensor, struct libtofcan_sample *data),
    void (*batch)(int sensor, struct libtofcan_batch *data, bool valid)
);

/*
 * Prepares a CAN message to configure the sensor with the specified ID.
 */
extern void libtofcan_config(int sensor, struct libtofcan_msg *msg,
                             struct tof2can_config *config);

/*
 * Prepares a CAN message to request a sample or data batch from the
 * sensor with the specified ID.
 */
extern void libtofcan_request(int sensor, struct libtofcan_msg *msg);

/*
 * Handles a CAN message coming from a ToF sensor. If the message does
 * not come from a ToF sensor, no action is performed.
 */
extern void libtofcan_receive(const struct libtofcan_msg *msg);

/*
 * Generates a human-readable string describing the given configuration.
 * The string's maximum length should be at least 256.
 */
extern void libtofcan_config_string(struct tof2can_config *config,
                                    char *str, int maxlen);

#ifdef __cplusplus
}
#endif

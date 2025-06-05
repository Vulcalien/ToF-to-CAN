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
#include "tof.h"

#include <stdio.h>
#include <unistd.h>

#include "vl53l5cx_api.h"

int tof_matrix_width;
static int tof_resolution;

static VL53L5CX_Configuration config;
static VL53L5CX_ResultsData results;
static bool is_ranging = false;

extern void set_i2c_rst(bool on);
extern void set_LPn(bool on);

static inline void tof_reset(void) {
    set_LPn(0);
    usleep(2000);
    set_LPn(1);

    set_i2c_rst(0);
    usleep(2000);
    set_i2c_rst(1);
    usleep(2000);
    set_i2c_rst(0);
    usleep(10000);
}

static inline int get_resolution_sqrt(void) {
    switch(tof_resolution) {
        case VL53L5CX_RESOLUTION_4X4: return 4;
        case VL53L5CX_RESOLUTION_8X8: return 8;
    }
    return -1;
}

int tof_init(void) {
    config.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;

    printf("[ToF] resetting sensor\n");
    board_userled(BOARD_RED_LED,   true);
    board_userled(BOARD_GREEN_LED, true);
    tof_reset();

    // check if sensor is alive
    uint8_t is_alive;
    if(vl53l5cx_is_alive(&config, &is_alive) || !is_alive) {
        printf(
            "[ToF] sensor not detected at address 0x%x\n",
            config.platform.address
        );
        return 1;
    }
    printf("[ToF] sensor detected\n");

    // initialize sensor
    if(vl53l5cx_init(&config)) {
        printf("[ToF] error initializing sensor\n");
        return 1;
    }
    printf("[ToF] initialization complete\n");

    // set sensor's ranging mode to continuous
    if(vl53l5cx_set_ranging_mode(&config, VL53L5CX_RANGING_MODE_CONTINUOUS)) {
        printf("[ToF] error setting ranging mode\n");
        return 1;
    }

    // get default resolution
    uint8_t resolution;
    if(vl53l5cx_get_resolution(&config, &resolution)) {
        printf("[ToF] error getting default resolution\n");
        return 1;
    }

    tof_resolution   = resolution;
    tof_matrix_width = get_resolution_sqrt();

    board_userled(BOARD_RED_LED,   false);
    board_userled(BOARD_GREEN_LED, false);
    return 0;
}

void tof_start_ranging(void) {
    // if already ranging, do nothing
    if(is_ranging)
        return;

    while(vl53l5cx_start_ranging(&config)) {
        printf("[ToF] error trying to start ranging, retrying\n");
        usleep(1000); // wait 1ms
    }
    is_ranging = true;
}

void tof_stop_ranging(void) {
    // if already not ranging, do nothing
    if(!is_ranging)
        return;

    while(vl53l5cx_stop_ranging(&config)) {
        printf("[ToF] error trying to stop ranging, retrying\n");
        usleep(1000); // wait 1ms
    }
    is_ranging = false;
}

int tof_set_resolution(int resolution) {
    tof_resolution   = resolution;
    tof_matrix_width = get_resolution_sqrt();

    int err = vl53l5cx_set_resolution(&config, resolution);
    printf(
        "[ToF] setting resolution to %d (err=%d)\n",
        resolution, err
    );
    return err;
}

int tof_read_data(int16_t **matrix, uint8_t **status_matrix) {
    uint8_t is_ready;

    // check if data is ready
    if(vl53l5cx_check_data_ready(&config, &is_ready)) {
        printf("[ToF] error in vl53l5cx_check_data_ready\n");
        return 1;
    }

    // if data is not ready, return
    if(!is_ready)
        return 1;

    // read ranging data
    if(vl53l5cx_get_ranging_data(&config, &results)) {
        printf("[ToF] error in vl53l5cx_get_ranging_data\n");
        return 1;
    }

    #if VL53L5CX_NB_TARGET_PER_ZONE == 1
        *matrix = results.distance_mm;
        *status_matrix = results.target_status;
    #else
        static int16_t _matrix[64];
        static uint8_t _status_matrix[64];

        for(int i = 0; i < tof_resolution; i++) {
            const int results_index = i * VL53L5CX_NB_TARGET_PER_ZONE;

            _matrix[i] = results.distance_mm[results_index];
            _status_matrix[i] = results.target_status[results_index];
        }

        *matrix = _matrix;
        *status_matrix = _status_matrix;
    #endif

    return 0;
}

int tof_set_frequency(int frequency_hz) {
    int err = vl53l5cx_set_ranging_frequency_hz(&config, frequency_hz);
    printf(
        "[ToF] setting frequency to %dHz (err=%d)\n",
        frequency_hz, err
    );
    return err;
}

int tof_set_sharpener(int sharpener_percent) {
    int err = vl53l5cx_set_sharpener_percent(&config, sharpener_percent);
    printf(
        "[ToF] setting sharpener to %d%% (err=%d)\n",
        sharpener_percent, err
    );
    return err;
}

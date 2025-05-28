#include "tof.h"

#include <stdio.h>
#include <unistd.h>

#include "vl53l5cx_api.h"

int tof_resolution   = 16; // default value
int tof_matrix_width = 4;  // default value

static VL53L5CX_Configuration config;
static VL53L5CX_ResultsData results;
static bool is_ranging = false;

extern void set_i2c_rst(bool on);
extern void set_LPn(bool on);
extern void board_userled(int led, bool ledon);

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

int tof_init(void) {
    config.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;

    printf("[ToF] resetting sensor\n");
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
    tof_resolution = resolution;
    if(resolution == VL53L5CX_RESOLUTION_4X4)
        tof_matrix_width = 4;
    else if(resolution == VL53L5CX_RESOLUTION_8X8)
        tof_matrix_width = 8;

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

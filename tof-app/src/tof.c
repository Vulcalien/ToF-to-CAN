#include "tof.h"

#include <stdio.h>
#include <unistd.h>

#include "vl53l5cx_api.h"

int tof_resolution   = 16; // default value
int tof_matrix_width = 4;  // default value

static VL53L5CX_Configuration config;
static bool ranging = false;

int tof_init(void) {
    config.platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;

    uint8_t is_alive;
    if(vl53l5cx_is_alive(&config, &is_alive) || !is_alive) {
        printf(
            "ToF: sensor not detected at address %x\n",
            config.platform.address
        );
        return 1;
    }

    if(vl53l5cx_init(&config)) {
        printf("ToF: error initializing sensor\n");
        return 1;
    }
    return 0;
}

int tof_start_ranging(void) {
    if(ranging)
        return 0;
    return vl53l5cx_start_ranging(&config);
}

int tof_stop_ranging(void) {
    if(!ranging)
        return 0;
    return vl53l5cx_stop_ranging(&config);
}

int tof_set_resolution(int resolution) {
    tof_resolution = resolution;
    if(resolution == VL53L5CX_RESOLUTION_4X4)
        tof_matrix_width = 4;
    else if(resolution == VL53L5CX_RESOLUTION_8X8)
        tof_matrix_width = 8;

    int err = vl53l5cx_set_resolution(&config, resolution);
    printf(
        "ToF: setting resolution to %d (err=%d)\n",
        resolution, err
    );
    return err;
}

int tof_read_data(int16_t *matrix, uint8_t *status_matrix) {
    int err = 0;
    VL53L5CX_ResultsData results;

    // wait for data to be ready
    uint8_t is_ready = false;
    while(!is_ready) {
        err = vl53l5cx_check_data_ready(&config, &is_ready);
        if(err) {
            printf("ToF: error in vl53l5cx_check_data_ready\n");
            return err;
        }
        usleep(1000); // wait 1ms
    }

    // read ranging data
    err = vl53l5cx_get_ranging_data(&config, &results);
    if(err) {
        printf("ToF: error in vl53l5cx_get_ranging_data\n");
        return err;
    }

    // copy ranging data
    for(int i = 0; i < tof_resolution; i++) {
        const int results_index = i * VL53L5CX_NB_TARGET_PER_ZONE;

        matrix[i] = results.distance_mm[results_index];
        status_matrix[i] = results.target_status[results_index];
    }
    return err;
}

int tof_set_ranging_frequency(int frequency_hz) {
    int err = vl53l5cx_set_ranging_frequency_hz(&config, frequency_hz);
    printf(
        "ToF: setting ranging frequency to %dHz (err=%d)\n",
        frequency_hz, err
    );
    return err;
}

int tof_set_ranging_mode(int mode) {
    int err = vl53l5cx_set_ranging_mode(&config, mode);

    const char *setting_name;
    if(mode == 0)
        setting_name = "Autonomous";
    else if(mode == 1)
        setting_name = "Continuous";
    else
        setting_name = "Unknown";

    printf(
        "ToF: setting ranging mode to '%s' (err=%d)\n",
        setting_name, err
    );
    return err;
}

int tof_set_sharpener(int sharpener_percent) {
    int err = vl53l5cx_set_sharpener_percent(&config, sharpener_percent);
    printf(
        "ToF: setting sharpener to %d%% (err=%d)\n",
        sharpener_percent, err
    );
    return err;
}

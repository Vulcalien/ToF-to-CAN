#include "tof.h"

#include <stdio.h>

#include "vl53l5cx_api.h"

static VL53L5CX_Configuration config;

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
    return vl53l5cx_start_ranging(&config);
}

int tof_stop_ranging(void) {
    return vl53l5cx_stop_ranging(&config);
}

int tof_set_resolution(int resolution) {
    int err = vl53l5cx_set_resolution(&config, resolution);
    printf(
        "ToF: setting resolution to %d (err=%d)\n",
        resolution, err
    );
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

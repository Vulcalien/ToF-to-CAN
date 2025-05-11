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

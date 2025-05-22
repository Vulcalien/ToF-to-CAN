#include "main.h"

#include "distance-sensor.h"

// Compile-time checks

_Static_assert(
    sizeof(struct distance_sensor_can_config) == DISTANCE_SENSOR_CAN_CONFIG_SIZE,
    "size of struct distance_sensor_can_config is incorrect"
);

_Static_assert(
    sizeof(struct distance_sensor_can_sample) == DISTANCE_SENSOR_CAN_SAMPLE_SIZE,
    "size of struct distance_sensor_can_sample is incorrect"
);

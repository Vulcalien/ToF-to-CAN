#include "main.h"

#include "tof2can.h"

// Compile-time checks

_Static_assert(
    sizeof(struct tof2can_config) == TOF2CAN_CONFIG_SIZE,
    "size of struct tof2can_config is incorrect"
);

_Static_assert(
    sizeof(struct tof2can_sample) == TOF2CAN_SAMPLE_SIZE,
    "size of struct tof2can_sample is incorrect"
);

_Static_assert(
    sizeof(struct tof2can_data_packet) == TOF2CAN_DATA_PACKET_SIZE,
    "size of struct tof2can_data_packet is incorrect"
);

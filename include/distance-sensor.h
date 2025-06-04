#ifndef DISTANCE_SENSOR_CAN_INTERFACE
#define DISTANCE_SENSOR_CAN_INTERFACE

#include <stdint.h>
#include <stdbool.h>

#define DISTANCE_SENSOR_CAN_CONFIG_SIZE 8
struct distance_sensor_can_config {
    // ToF settings
    uint8_t resolution; // 16 or 64
    uint8_t frequency;  // 1...60Hz
    uint8_t sharpener;  // 0...99%

    // processing settings
    uint8_t  processing_mode; // see below
    uint16_t threshold;       // 0...4000mm
    uint8_t  threshold_delay; // 0...255

    // data transmission
    uint8_t transmit_timing    : 1; // 0=on-demand, 1=continuous
    uint8_t transmit_condition : 2; // see below
};

// processing_mode:
//
// Bits 6-7 identify the area to process:
//     0=matrix, 1=column, 2=row, 3=point
// Other bits are treated differently based on the specified area.
//
// if area == matrix:
//   + Bit +- Function --------+- Values ------------------------+
//   | 0-3 |  < unused >       |                                 |
//   | 4-5 |  result selector  |  0=min, 1=max, 2=average, 3=all |
//   | 6-7 |  area             |  0=matrix                       |
//   +-----+-------------------+---------------------------------+
//
// if (area == column) or (area == row):
//   + Bit +- Function --------+- Values ------------------------+
//   | 0-2 |  column/row       |  0...7                          |
//   | 3   |  < unused >       |                                 |
//   | 4-5 |  result selector  |  0=min, 1=max, 2=average, 3=all |
//   | 6-7 |  area             |  1=column, 2=row                |
//   +-----+-------------------+---------------------------------+
//
// if area == point:
//   + Bit +- Function --------+- Values ------------------------+
//   | 0-2 |  x coordinate     |  0...7                          |
//   | 3-5 |  y coordinate     |  0...7                          |
//   | 6-7 |  area             |  3=point                        |
//   +-----+-------------------+---------------------------------+

// transmit_condition:
// - 0=always true
// - 1=below threshold event
// - 2=above threshold event
// - 3=any threshold event

#define DISTANCE_SENSOR_CAN_SAMPLE_SIZE 4
struct distance_sensor_can_sample {
    int16_t distance;
    bool below_threshold;

    char _padding[1];
};

// number of distinct sensor IDs (ID=0 is broadcast)
#define DISTANCE_SENSOR_MAX_COUNT 32

#define DISTANCE_SENSOR_CAN_CONFIG_MASK_ID 0x6c0 // 0x6c0...0x6df
#define DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID 0x6e0 // 0x6e0...0x6ff

#endif // DISTANCE_SENSOR_CAN_INTERFACE

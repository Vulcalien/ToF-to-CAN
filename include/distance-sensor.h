#ifndef DISTANCE_SENSOR_CAN_INTERFACE
#define DISTANCE_SENSOR_CAN_INTERFACE

#define DISTANCE_SENSOR_CAN_CONFIG_SIZE 8
struct distance_sensor_can_config {
    // ToF settings
    unsigned int resolution        : 8; // 16 or 64
    unsigned int ranging_frequency : 8; // 1...60Hz
    unsigned int sharpener         : 8; // 0...99%

    // processing settings
    unsigned int processing_mode : 8;  // see below
    unsigned int threshold       : 16; // 0...4000mm
    unsigned int threshold_delay : 8;  // 0...255

    // data transmission
    unsigned int transmit_timing    : 1; // 0=on-demand, 1=continuous
    unsigned int transmit_condition : 2; // see below
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
    short distance;
    bool below_threshold;

    char _padding[1];
};

// number of distinct sensor IDs (ID=0 is broadcast)
#define DISTANCE_SENSOR_MAX_COUNT 32

#define DISTANCE_SENSOR_CAN_CONFIG_MASK_ID 0x6c0 // 0x6c0...0x6df
#define DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID 0x6e0 // 0x6e0...0x6ff

#endif // DISTANCE_SENSOR_CAN_INTERFACE

#ifndef DISTANCE_SENSOR_CAN_INTERFACE
#define DISTANCE_SENSOR_CAN_INTERFACE

struct distance_sensor_can_config {
    // ToF settings
    unsigned int resolution        : 8; // 16 or 64
    unsigned int ranging_frequency : 8; // 1...60Hz
    unsigned int ranging_mode      : 1; // 0=autonomous, 1=continuous
    unsigned int sharpener         : 7; // 0...99%

    // processing settings
    unsigned int processing_mode : 8;  // TODO explain this field
    unsigned int threshold       : 16; // 0...4000mm
    unsigned int threshold_delay : 8;  // 0...255

    // data transmission
    // TODO
};

struct distance_sensor_can_sample {
    unsigned short distance;
    bool below_threshold;
};

// number of distinct sensor IDs (ID=0 is broadcast)
#define DISTANCE_SENSOR_MAX_COUNT 32

#define DISTANCE_SENSOR_CAN_CONFIG_MASK_ID 0x6c0 // 0x6c0...0x6df
#define DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID 0x6e0 // 0x6e0...0x6ff

#endif // DISTANCE_SENSOR_CAN_INTERFACE

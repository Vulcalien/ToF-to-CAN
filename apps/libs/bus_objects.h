/*
 * bus_objects.h
 */

#ifndef __BUS_OBJECTS
#define __BUS_OBJECTS


// -------------- DISTANCE_SENSORS ---------------------

#define DISTANCE_SENSOR_CAN_SAMPLE_ID             0x67f

typedef struct {
    unsigned short threshold;
    unsigned short count;
    char padding[4];
}  __attribute__((packed)) t_can_distance_sensor_sample;

#define DISTANCE_SENSOR_CAN_MASK_ID                 0x670
#define DISTANCE_SENSOR_DATA_CAN_MASK_ID            0x6f0

typedef struct {
    unsigned char sensor;
    unsigned short distance;
    unsigned char alarm;
    char padding[4];
}  __attribute__((packed)) t_can_distance_sensor_data;

#define DISTANCE_SENSOR_CAN_NOTIFY_ID             0x66f
#define DISTANCE_SENSOR_CAN_STREAM_ID             0x66e
#define DISTANCE_SENSOR_CAN_STREAM_OFF_ID         0x66d


#endif

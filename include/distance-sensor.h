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

/*
 * Definition of a packet-based protocol for transmitting multiple
 * samples, with support for out-of-order packets and loss-tolerance.
 *
 *
 * To transmit all the data it needs to, the sensor sends multiple
 * packets organized in batches.
 *
 * A batch is assigned a batch ID, and all packets within it share the
 * same batch ID. Packets within a batch are assigned a sequence number
 * from 0 to N-1, with N being the total number of packets in the batch.
 *
 * Since N is not known beforehand, the last packet of a batch has the
 * 'last of batch' flag set. The receiver should set N to the last
 * packet's sequence number plus one.
 *
 * Packets may contain from 1 to 3 samples: the 'data length' attribute
 * specifies this information. All packets, except the last one, are
 * required to contain 3 samples.
 *
 *
 * Handling out-of-order packets:
 *   the receiver should buffer the samples received, offset by
 *   (sequence_number * 3), regardless of the order of arrival.
 *
 *   Note that the last packet of a batch may arrive before other
 *   packets within that batch: use the 'last of batch' flag to obtain
 *   the total number of packets, then wait for all of them to arrive.
 *
 * Handling packet loss:
 *   first, note that packets are never retransmitted, so there is no
 *   way to recover lost data. To avoid waiting indefinitely for lost
 *   packets, the receiver should keep track of the batch ID. Whenever
 *   the batch ID changes (i.e. a new batch is being transmitted), the
 *   previous batch should be considered incomplete.
 */

#define DISTANCE_SENSOR_CAN_DATA_PACKET_SIZE 8
struct distance_sensor_can_data_packet {
    uint8_t sequence_number;

    uint8_t data_length   : 2;
    uint8_t batch_id      : 5;
    uint8_t last_of_batch : 1;

    int16_t data[3];
};

// number of distinct sensor IDs (ID=0 is broadcast)
#define DISTANCE_SENSOR_MAX_COUNT 32

#define DISTANCE_SENSOR_CAN_CONFIG_MASK_ID      0x6c0 // 0x6c0...0x6df
#define DISTANCE_SENSOR_CAN_SAMPLE_MASK_ID      0x6e0 // 0x6e0...0x6ff
#define DISTANCE_SENSOR_CAN_DATA_PACKET_MASK_ID 0x700 // 0x700...0x71f

#endif // DISTANCE_SENSOR_CAN_INTERFACE

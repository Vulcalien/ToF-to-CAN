#ifndef DISTANCE_SENSOR_CAN_INTERFACE
#define DISTANCE_SENSOR_CAN_INTERFACE

#include <stdint.h>
#include <stdbool.h>

/*
 * struct distance_sensor_can_config (size = 8)
 *
 * Sent by the user device to the distance sensor to configure it. After
 * powering up, the sensor remains idle until being configured.
 *
 * resolution
 *     Number of sampled data points. Allowed values:
 *     - 16 (4x4)
 *     - 64 (8x8)
 *
 * frequency:
 *     How frequently the sensor obtains new data, measured in Hz.
 *     The allowed range depends on the *resolution*:
 *     - if resolution = 16 (4x4), the range is [1, 60]
 *     - if resolution = 64 (8x8), the range is [1, 15]
 *
 * sharpener:
 *     TODO
 *
 * processing_mode:
 *     Settings determining which area of the sensor's data is
 *     processed, and which quantity is returned.
 *
 *     Bits 6-7 identify the area: 0=matrix, 1=column, 2=row, 3=point.
 *     Other bits are treated differently based on the specified area.
 *
 *     if area == matrix:
 *       + Bit +- Function --------+- Values ------------------------+
 *       | 0-3 |  < unused >       |                                 |
 *       | 4-5 |  result selector  |  0=min, 1=max, 2=average, 3=all |
 *       | 6-7 |  area             |  0=matrix                       |
 *       +-----+-------------------+---------------------------------+
 *
 *     if (area == column) or (area == row):
 *       + Bit +- Function --------+- Values ------------------------+
 *       | 0-2 |  column/row       |  0...7                          |
 *       | 3   |  < unused >       |                                 |
 *       | 4-5 |  result selector  |  0=min, 1=max, 2=average, 3=all |
 *       | 6-7 |  area             |  1=column, 2=row                |
 *       +-----+-------------------+---------------------------------+
 *
 *     if area == point:
 *       + Bit +- Function --------+- Values ------------------------+
 *       | 0-2 |  x coordinate     |  0...7                          |
 *       | 3-5 |  y coordinate     |  0...7                          |
 *       | 6-7 |  area             |  3=point                        |
 *       +-----+-------------------+---------------------------------+
 *
 * threshold:
 *     The distance to use as threshold value for calculating the
 *     *below_threshold* value. Ignored if *processing_mode* specifies
 *     multiple distance samples as result.
 *
 * threshold_delay:
 *     Number of internal iterations that the distance needs to be
 *     consistently below or above the threshold before updating the
 *     *below_threshold* value. The corresponding amount of time is
 *     equal to (threshold_delay / frequency) seconds. Ignored if
 *     *processing_mode* specifies multiple distance samples as result.
 *
 * transmit_timing:
 *     When the sensor should transmit new data.
 *     If set to on-demand mode (0), the sensor will wait for the user
 *     device to send a *Remote Transmit Request* message.
 *     If set to continuous mode (1), the sensor will transmit as soon
 *     as new data is available.
 *
 * transmit_condition:
 *     The condition that needs to be satisfied for the sensor to
 *     transmit data. Allowed values:
 *     - 0=always true
 *     - 1=below threshold event
 *     - 2=above threshold event
 *     - 3=any threshold event
 *
 *     Threshold events happen when the *below_threshold* value changes:
 *     - if below_threshold = true, a 'below threshold event' happens
 *     - if below_threshold = false, an 'above threshold event' happens
 *
 *     Ignored if *processing_mode* specifies multiple distance samples
 *     as result.
 */

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

/*
 * struct distance_sensor_can_sample (size = 4)
 *
 * Sent by the distance sensor when configured to obtain a single
 * distance sample.
 *
 * distance:
 *     The sample distance, in millimeters.
 *
 * below_threshold:
 *     An indication of whether the distance is below the configured
 *     *threshold* or not. Note that the sensor only updates this value
 *     if the distance has been consistently below or above the
 *     threshold for at least *threshold_delay* internal iterations.
 */

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

/*
 * struct distance_sensor_can_data_packet (size = 8)
 *
 * Sent by the distance sensor, as part of a batch of packets, when
 * configured to obtain multiple distance samples.
 *
 * sequence_number:
 *     Identifier of the packet within its batch.
 *
 * data_length:
 *     Number of data samples carried by the packet, from 1 to 3. Only
 *     the last packet of a batch is allowed to contain less than 3.
 *
 * batch_id:
 *     Identifier of the packet's batch.
 *
 * last_of_batch:
 *     True if the packet is the last of its batch, false otherwise.
 *
 * data:
 *     Array of samples carried by the packet, measured in millimeters.
 *     Invalid samples are represented by a negative value.
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

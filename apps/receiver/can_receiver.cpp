/*
 * can_receiver.cpp
 */


#include "can_receiver.h"
#include "can_utils.h"
#include "bus_objects.h"

#include <nuttx/can/can.h>

#include "geometry.h"
#include "can_sender.h"
#include "tof_processing.h"

extern MC_CanSender         *can_sender;
extern TOFProcessor         * processor;

CANReceiver::CANReceiver(PeriodicTaskScheduler &sched)
    : PeriodicTask(sched, "can_receiver", 1)
{
    fd = CAN::open(O_RDONLY);
    if (fd < 0) {
        perror("Opening CAN");
    }
    //printf("CANReceiver STARTED\n");
    struct canioc_bittiming_s bt;
    int ret = ioctl(fd, CANIOC_GET_BITTIMING, (unsigned long)((uintptr_t)&bt));
    if (ret < 0) {
        printf("Bit timing not available: %d\n", errno);
    }
    else {
        printf("Bit timing:\n");
        printf("   Baud: %lu\n", (unsigned long)bt.bt_baud);
        printf("  TSEG1: %u\n", bt.bt_tseg1);
        printf("  TSEG2: %u\n", bt.bt_tseg2);
        printf("    SJW: %u\n", bt.bt_sjw);
    }

    DDS::insert("alarm", 300);
    alarm_handle = DDS::get_handle("alarm");
    bool alval = false;
    DDS::publish_handle(alarm_handle, (char *)&alval, sizeof(bool));
    DDS::insert("sample", 250);
    sample_handle = DDS::get_handle("sample");
    DDS::insert("threshold", 1000);
    threshold_handle = DDS::get_handle("threshold");
}


void CANReceiver::run(float delta_t)
{
    int              msgsize;
    struct can_msg_s rxmsg;

    msgsize        = sizeof(struct can_msg_s);
    ssize_t nbytes = read(fd, &rxmsg, msgsize);
    if (nbytes < (ssize_t)CAN_MSGLEN(0) || nbytes > msgsize) {
        printf("ERROR: CAN_READ(%ld) returned %ld\n", (long)msgsize, (long)nbytes);
        return;
    }

    // if (m_canlog)
    //     printf("[CANTASK] id=%x\n", rxmsg.cm_hdr.ch_id);
    // if (m_filter_id != 0) {
    //     if (m_filter_id == rxmsg.cm_hdr.ch_id)
    //         printf("[CANTASK] id=%x\n", rxmsg.cm_hdr.ch_id);
    // }
    switch (rxmsg.cm_hdr.ch_id) {
    case DISTANCE_SENSOR_CAN_SAMPLE_ID:
        {
            const t_can_distance_sensor_sample *m =
                (const t_can_distance_sensor_sample *)rxmsg.cm_data;

            printf("[CANReceiver] received DISTANCE_SENSOR_CAN_SAMPLE, threshold = %d, count = %d\n", m->threshold, m->count);

            processor->set_threshold(m->threshold);
            processor->set_alarm_max(m->count);

            bool do_sample = true;
            DDS::publish_handle(sample_handle, (void *)&do_sample, sizeof(bool));
        }
        break;
    case DISTANCE_SENSOR_CAN_NOTIFY_ID:
        {
            printf("[CANReceiver] received DISTANCE_SENSOR_CAN_NOTIFY_ID\n");
            bool alarm = false;
            DDS::publish_handle(alarm_handle, (void *)&alarm, sizeof(bool));
            bool do_sample = true;
            DDS::publish_handle(sample_handle, (void *)&do_sample, sizeof(bool));
        }
        break;
    case DISTANCE_SENSOR_CAN_STREAM_ID:
        {
            printf("[CANReceiver] received DISTANCE_SENSOR_CAN_STREAM_ID\n");
            can_sender->stream(true);
        }
        break;
    case DISTANCE_SENSOR_CAN_STREAM_OFF_ID:
        {
            printf("[CANReceiver] received DISTANCE_SENSOR_CAN_STREAM_OFF_ID\n");
            can_sender->stream(false);
        }
        break;
    }
}

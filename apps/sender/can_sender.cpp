/*
 * mc_can_sender.cpp
 */


#include "can_sender.h"
#include "can_utils.h"

#include <nuttx/can/can.h>

#include "dds.h"

#include "shared_data.h"

extern "C" void board_userled(int led, bool ledon);

MC_CanSender::MC_CanSender(PeriodicTaskScheduler &sched)
    : PeriodicTask(sched, "can_sender", 1),continuous(false)
{
    fd = CAN::open(O_WRONLY);
    if (fd < 0) {
        perror("Opening CAN");
    }
    //printf("MC_CanSender STARTED\n");
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
    sample_handle = DDS::get_handle("sample");
    alarm_handle = DDS::get_handle("alarm");
    distance_handle = DDS::get_handle("distance");
}


void MC_CanSender::run(float delta_t)
{

    if (continuous) {
        int distance;
        bool d_stale;
        DDS::wait_handle(distance_handle, (void *)&distance, d_stale);
        //printf("[CANSender] %p, DISTANCE %d, stale %d\n", distance_handle, (int)distance, (int)d_stale);
        if (!d_stale) {
            t_can_distance_sensor_data p;
            p.sensor   = __SENSOR_ID__;
            p.distance = distance;
            p.alarm    = 0;
            int id = DISTANCE_SENSOR_DATA_CAN_MASK_ID | __SENSOR_ID__;
            CAN::write(fd, id, (uint8_t *)&p, 8);
            return;
        }
    }

    bool s;
    bool stale;
    DDS::read_handle(sample_handle, (void *)&s, stale);

    if (s & !stale) {

        bool alarm;
        bool stale_alarm;

        DDS::read_handle(alarm_handle, (void *)&alarm, stale_alarm);

        //pirntf("[CANSender] ALARM %d, stale %d\n", (int)alarm, (int)stale_alarm);

        if (alarm) {
            t_can_distance_sensor_data p;

            p.sensor   = __SENSOR_ID__;
            p.distance = 0;
            p.alarm    = alarm;
            int id = DISTANCE_SENSOR_CAN_MASK_ID | __SENSOR_ID__;
            printf("[CANSender] Sending Distance data ID=%x, ALARM=%d\n", id, alarm);
            CAN::write(fd, id, (uint8_t *)&p, 8);
            printf("[CANSender] Sent\n");

            //alarm = false;
            //DDS::publish_handle(alarm_handle, (void *)&alarm, sizeof(bool));

            s = false;
        }
        else
            s = true;
        DDS::publish_handle(sample_handle, (void *)&s, sizeof(bool));

    }

}

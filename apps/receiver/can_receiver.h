/*
 * can_receiver.h
 */

#ifndef __CAN_SENDER_H
#define __CAN_SENDER_H

#include "periodic_task.h"
#include <stdbool.h>
#include "dds.h"
#include "shared_data.h"

class CANReceiver : public PeriodicTask {
public:
    CANReceiver(PeriodicTaskScheduler &sched);
    virtual void run(float delta_t);
protected:
    int fd;
    void * sample_handle;
    void * threshold_handle;
    void * alarm_handle;
};

extern CANReceiver *can_receiver;

#endif

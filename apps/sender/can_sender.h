/*
 * mc_can_sender.h
 */

#ifndef __MC_CAN_SENDER_H
#define __MC_CAN_SENDER_H

#include "periodic_task.h"
#include <stdbool.h>

class MC_CanSender : public PeriodicTask {
public:
    MC_CanSender(PeriodicTaskScheduler &sched);
    virtual void run(float delta_t);
    void stream(bool s) { continuous = s; };
protected:
    int fd;
    void * sample_handle;
    void * alarm_handle;
    void * distance_handle;
    bool continuous;
};

extern MC_CanSender *can_sender;

#endif

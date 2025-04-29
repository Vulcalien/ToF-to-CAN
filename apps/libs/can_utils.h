/*
 * can_utils.h
 */

#ifndef __CAN_UTILS_H
#define __CAN_UTILS_H

#include <fcntl.h>
#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <nuttx/can/can.h>
#include "bus_objects.h"

namespace CAN {
    int open(int mode);
    int write(int fd, int can_id, unsigned char *msg, int len = 8);
};

#endif

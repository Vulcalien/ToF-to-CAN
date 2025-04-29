/*
 * servo.h
 */

#pragma once

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#ifdef CONFIG_RCSERVO_INTERFACE

#include <nuttx/leds/pca9635pw.h>

class RCServo {
 public:
    RCServo(const char * device);
    void set_servo(int servo_id, int microsecs);
 private:
    void config();
    int m_servo_fd;
};

#include "pca9685_driver.h"

extern PCA9685_Dev * servo;

namespace Servo {
    inline void set(int sn, int microsecs) { servo->set_servo(sn, microsecs); };
    inline void all_off() { servo->all_off(); };
};

#endif

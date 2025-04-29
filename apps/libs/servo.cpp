/*
 * servo.cpp
 */

#ifdef CONFIG_RCSERVO_INTERFACE

#include "servo.h"

bool m_initialized = false;

RCServo::RCServo(const char * device)
{
    m_servo_fd = open(device, O_RDONLY);
    if (m_servo_fd < 0) {
        perror("Failed to open servo driver");
        return;
    }
    if (!m_initialized) {
        config();
        m_initialized = true;
    }
}

void RCServo::config()
{
    struct pca9685pw_setled_freq_arg_s f;
    f.freq = 49;

    if (ioctl(m_servo_fd, PWMIOC_SETLED_PWM_CONFIG, (long unsigned int)&f) < 0)
        perror("Failed to set servo frequency");
}


void RCServo::set_servo(int servo_id, int microsecs)
{
    struct pca9685pw_setled_duty_arg_s d;
    d.led = (led_select_e)servo_id;
    d.duty = (microsecs*4096l)/20000l;

    if (ioctl(m_servo_fd, PWMIOC_SETLED_PWM_DUTY, (long unsigned int)&d) < 0)
        perror("Failed to set servo duty");

}

#endif

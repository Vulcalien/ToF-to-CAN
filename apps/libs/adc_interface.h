/*
 * adc_interface.h
 */

#pragma once

#include <stdbool.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>

#define ANALOG_CHANNELS     2
#define LEFT_CHANNEL		10
#define RIGHT_CHANNEL		11	

class ADCInterface {
 public:
    ADCInterface() : m_adc_fd(-1), m_channels(ANALOG_CHANNELS) {};
    bool open();
    bool close();
    bool poll();
    bool read_channel(int channel, int & value);
 private:
    int m_adc_fd;
    int m_channels;
    struct adc_msg_s m_samples[ANALOG_CHANNELS];
};

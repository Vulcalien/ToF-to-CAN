/*
 * adc_interface.cpp
 */

#include <stdio.h>
#include <fcntl.h>
#include "adc_interface.h"

bool ADCInterface::open()
{
    m_adc_fd = ::open("/dev/adc0", O_RDONLY);
    if (m_adc_fd < 0) {
        // perror("ADCInterface: open /dev/adc0 failed");
        return false;
    }
    else
        return true;
}

bool ADCInterface::close()
{
    ::close(m_adc_fd);
    return true;
}


bool ADCInterface::poll()
{
    int readsize = m_channels * sizeof(struct adc_msg_s);
    int nbytes = read(m_adc_fd, m_samples, readsize);
    if (nbytes < 0) {
        perror("ADCInterface: read failed");
        return false;
    }
    if (nbytes == 0)
        return false;
    else
        return true;
}


bool ADCInterface::read_channel(int channel, int & value)
{
    for (int i = 0; i < m_channels; i++) {
        if (m_samples[i].am_channel == channel) {
            value = m_samples[i].am_data;
            return true;
        }
    }
    return false;
}


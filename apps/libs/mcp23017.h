/*
 * mcp23017_driver.h
 */

#pragma once

#include <pthread.h>
#include "i2c_driver.h"

#define A    0
#define B    1

#define IODIR(p)      (0x00 + p)
#define IPOL(p)       (0x02 + p)
#define GPINTEN(p)    (0x04 + p)
#define DEFVAL(p)     (0x06 + p)
#define INTCON(p)     (0x08 + p)
#define IOCON         0x0a
#define GPPU(p)       (0x0c + p)
#define INTF(p)       (0x0e + p)
#define INTCAP(p)     (0x10 + p)
#define GPIO(p)       (0x12 + p)
#define OLAT(p)       (0x14 + p)

typedef enum {
    gpio_OUT = 0,
    gpio_IN
} GPIO_dir;

class MCP23017_Dev : public I2C_Dev {
 public:
    MCP23017_Dev(FAR struct i2c_master_s *i2c_device);
    bool init();
    bool set_dir(int port, int pin, GPIO_dir dir);
    bool poll();
    bool flush();
    bool get_pin(int port, int pin) { return (in_buffer[port] >> pin) & 1; };
    bool event(int port, int pin) { return (evt_change[port] >> pin) & 1; };
    void set_pin(int port, int pin, bool val, bool do_flush = false) {
        pthread_mutex_lock(&m_mutex);
        if (val)
            out_buffer[port+1] |= (1 << pin);
        else
            out_buffer[port+1] &= ~(1 << pin);
        if (do_flush) flush();
        pthread_mutex_unlock(&m_mutex);
    };
 private:
    uint8_t in_buffer[2];
    uint8_t out_buffer[3];
    uint8_t evt_change[2];
    pthread_mutex_t m_mutex;
};

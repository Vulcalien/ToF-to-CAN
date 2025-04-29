/*
 * mcp23017.cpp
 */

#include <stdio.h>
#include "mcp23017.h"

#define MCP23017_ADDR   0x20

MCP23017_Dev::MCP23017_Dev(FAR struct i2c_master_s *i2c_device)
  : I2C_Dev(i2c_device, MCP23017_ADDR)
{
    out_buffer[0] = OLAT(A);
    out_buffer[1] = 0;
    out_buffer[2] = 0;
    pthread_mutex_init(&m_mutex, NULL);
}

bool MCP23017_Dev::init()
{
    if (!write_reg(IOCON, 0x00))
        return false;

    if (!write_reg(INTCON(A), 0x00)) // interrupt on change w.r.t the previous value
        return false;
    if (!write_reg(INTCON(B), 0x00))
        return false;

    if (!write_reg(GPINTEN(A), 0xff)) // enable interrupt on change on all ports
        return false;
    if (!write_reg(GPINTEN(B), 0xff))
        return false;

    usleep(1000);

    uint8_t dummy;
    read_reg(INTCAP(A), dummy); // read INTCAP to clear interrupt conditions
    read_reg(INTCAP(B), dummy);

    return poll();
}

bool MCP23017_Dev::poll()
{
    pthread_mutex_lock(&m_mutex);
    bool r = read_buf(INTF(A), evt_change, 2);
    if (r)
        r = read_buf(GPIO(A), in_buffer, 2);
    //printf("[MCP23017_Dev] evt_change %02x %02x\n", evt_change[0], evt_change[1]);
    pthread_mutex_unlock(&m_mutex);
    return r;
}

bool MCP23017_Dev::flush()
{
    return write_buf(out_buffer, 3);
}

bool MCP23017_Dev::set_dir(int port, int pin, GPIO_dir dir)
{
    uint8_t dir_pattern;

    if (!read_reg(IODIR(port), dir_pattern)) return false;

    switch (dir) {
    case gpio_IN:  dir_pattern |= (1 << pin); break;
    case gpio_OUT: dir_pattern &= ~(1 << pin); break;
    }

    return write_reg(IODIR(port), dir_pattern);
}



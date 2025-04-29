/*
 * i2c_driver.cpp
 */

#include "i2c_driver.h"
#include <stdio.h>

I2C_Dev::I2C_Dev(FAR struct i2c_master_s *i2c_device, int dev_addr)
  : m_i2c_device(i2c_device),
    m_address(dev_addr),
    m_freq(I2C_BUS_FREQ_HZ)
{
}


bool I2C_Dev::read_reg(uint8_t reg, uint8_t & value)
{
    struct i2c_config_s config;
    config.frequency = m_freq;
    config.address = m_address;
    config.addrlen = 7;

    if (i2c_writeread(m_i2c_device, &config, &reg, 1, &value, 1) < 0)
        return false;
    else
        return true;
}


bool I2C_Dev::read_buf(uint8_t reg, uint8_t * value, int len)
{
    struct i2c_config_s config;
    config.frequency = m_freq;
    config.address = m_address;
    config.addrlen = 7;

    if (i2c_writeread(m_i2c_device, &config, &reg, 1, value, len) < 0)
        return false;
    else
        return true;
}


bool I2C_Dev::read_buf_multi(uint8_t * reg, int reg_len, uint8_t * value, int len)
{
    struct i2c_config_s config;
    config.frequency = m_freq;
    config.address = m_address;
    config.addrlen = 7;

    if (i2c_writeread(m_i2c_device, &config, reg, reg_len, value, len) < 0)
        return false;
    else
        return true;
}


bool I2C_Dev::write_reg(uint8_t reg, uint8_t value)
{
    struct i2c_config_s config;
    config.frequency = m_freq;
    config.address = m_address;
    config.addrlen = 7;

    uint8_t buffer[2] = { reg, value };
    if (i2c_write(m_i2c_device, &config, buffer, 2) < 0)
        return false;
    else
        return true;
}


bool I2C_Dev::write_buf(uint8_t * data, int len)
{
    struct i2c_config_s config;
    config.frequency = m_freq;
    config.address = m_address;
    config.addrlen = 7;

    // {
    //     int i;
    //     printf("[%02X] ", m_address);
    //     for (i = 0; i < len;i++) {
    //         printf("%02X ", data[i]);
    //         if ((i % 64) == 63) printf("\n");
    //     }
    //     printf("\n");
    // }

    if (i2c_write(m_i2c_device, &config, data, len) < 0)
        return false;
    else
        return true;
}

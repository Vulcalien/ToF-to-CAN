/*
 * i2c_driver.h
 */

#pragma once

#include <stdint.h>
#include <nuttx/config.h>

#include <errno.h>
#include <debug.h>

#include <nuttx/i2c/i2c_master.h>

/* I2C definitions */

#ifndef I2C_BUS_FREQ_HZ
#define I2C_BUS_FREQ_HZ            (400000)
#endif

class I2C_Dev {
 public:
    I2C_Dev(FAR struct i2c_master_s *i2c_device, int dev_addr);
 //protected:
    bool read_reg(uint8_t reg, uint8_t & value);
    bool write_reg(uint8_t reg, uint8_t value);
    bool read_buf(uint8_t reg, uint8_t * value, int len);
    bool read_buf_multi(uint8_t * reg, int reg_len, uint8_t * value, int len);
    bool write_buf(uint8_t * data, int len);
    int get_address() { return m_address; };
 private:
    FAR struct i2c_master_s *m_i2c_device;
    int m_address;
    int m_freq;
};

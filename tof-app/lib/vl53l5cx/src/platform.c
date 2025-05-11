/**
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "platform.h"

#include <unistd.h>

#include <nuttx/i2c/i2c_master.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define I2C_FREQUENCY 400000 // 400 KHz
#define I2C_MAX_PACKET_SIZE 32

extern struct i2c_master_s *i2cmain;

uint8_t VL53L5CX_RdByte(
        VL53L5CX_Platform *p_platform,
        uint16_t RegisterAdress,
        uint8_t *p_value)
{
    struct i2c_config_s config;
    config.frequency = I2C_FREQUENCY;
    config.address   = (p_platform->address >> 1);
    config.addrlen   = 7;

    uint8_t buf[2];
    buf[0] = (RegisterAdress >> 8) & 0xff;
    buf[1] = (RegisterAdress)      & 0xff;

    return i2c_writeread(i2cmain, &config, buf, 2, p_value, 1);
}

uint8_t VL53L5CX_WrByte(
        VL53L5CX_Platform *p_platform,
        uint16_t RegisterAdress,
        uint8_t value)
{
    struct i2c_config_s config;
    config.frequency = I2C_FREQUENCY;
    config.address   = (p_platform->address >> 1);
    config.addrlen   = 7;

    uint8_t buf[3];
    buf[0] = (RegisterAdress >> 8) & 0xff;
    buf[1] = (RegisterAdress)      & 0xff;
    buf[2] = value;

    return i2c_write(i2cmain, &config, buf, 3);
}

uint8_t VL53L5CX_WrMulti(
        VL53L5CX_Platform *p_platform,
        uint16_t RegisterAdress,
        uint8_t *p_values,
        uint32_t size)
{
    int err = 0;

    struct i2c_config_s config;
    config.frequency = I2C_FREQUENCY;
    config.address   = (p_platform->address >> 1);
    config.addrlen   = 7;

    uint8_t buf[I2C_MAX_PACKET_SIZE];
    buf[0] = (RegisterAdress >> 8) & 0xff;
    buf[1] = (RegisterAdress)      & 0xff;

    // write in blocks of I2C_MAX_PACKET_SIZE bytes or less
    while(size > 0 && !err) {
        const int data_bytes = MIN(size, I2C_MAX_PACKET_SIZE - 2);
        memcpy(buf + 2, p_values, data_bytes);
        err = i2c_write(i2cmain, &config, buf, data_bytes + 2);

        size -= data_bytes;
        p_values += data_bytes;
    }
    return err;
}

uint8_t VL53L5CX_RdMulti(
        VL53L5CX_Platform *p_platform,
        uint16_t RegisterAdress,
        uint8_t *p_values,
        uint32_t size)
{
    int err = 0;

    struct i2c_config_s config;
    config.frequency = I2C_FREQUENCY;
    config.address   = (p_platform->address >> 1);
    config.addrlen   = 7;

    uint8_t buf[2];
    buf[0] = (RegisterAdress >> 8) & 0xff;
    buf[1] = (RegisterAdress)      & 0xff;

    // read in blocks of I2C_MAX_PACKET_SIZE bytes or less
    while(size > 0 && !err) {
        const int data_bytes = MIN(size, I2C_MAX_PACKET_SIZE - 2);
        err = i2c_writeread(i2cmain, &config, buf, 2, p_values, data_bytes);

        size -= data_bytes;
        p_values += data_bytes;
    }
    return err;
}

uint8_t VL53L5CX_Reset_Sensor(
        VL53L5CX_Platform *p_platform)
{
    // TODO
    uint8_t status = 0;

    /* (Optional) Need to be implemented by customer. This function returns 0 if OK */

    /* Set pin LPN to LOW */
    /* Set pin AVDD to LOW */
    /* Set pin VDDIO  to LOW */
    VL53L5CX_WaitMs(p_platform, 100);

    /* Set pin LPN of to HIGH */
    /* Set pin AVDD of to HIGH */
    /* Set pin VDDIO of  to HIGH */
    VL53L5CX_WaitMs(p_platform, 100);

    return status;
}

void VL53L5CX_SwapBuffer(
        uint8_t         *buffer,
        uint16_t         size)
{
    for(uint32_t i = 0; i < size; i += 4) {
        uint32_t tmp = buffer[i]     << 24 | buffer[i + 1] << 16 |
                       buffer[i + 2] << 8  | buffer[i + 3];
        *((uint32_t *) &buffer[i]) = tmp;
    }
}

uint8_t VL53L5CX_WaitMs(
        VL53L5CX_Platform *p_platform,
        uint32_t TimeMs)
{
    return usleep(TimeMs * 1000);
}

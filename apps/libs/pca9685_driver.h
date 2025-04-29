/*
 * pca9685_driver.h
 */

#pragma once

#include "i2c_driver.h"

/* PCA9685PW register addresses */

#define PCA9685PW_MODE_1           (0x00)                     /* Mode register 1 */
#define PCA9685PW_MODE_2           (0x01)                     /* Mode register 2 */
#define PCA9685PW_SUBADR_1         (0x02)
#define PCA9685PW_SUBADR_2         (0x03)
#define PCA9685PW_SUBADR_3         (0x04)
#define PCA9685PW_ALLCALLADR       (0x05)

#define PCA9685PW_LED_0_ON         (0x06)
#define PCA9685PW_LED_0_OFF        (0x08)

#define PCA9685PW_PRESCALER        (0xfe)


/* PCA9685PW_MODE_1 bit definitions */

#define PCA9685PW_MODE_1_RESTART   (1<<7)                     /* restart */
#define PCA9685PW_MODE_1_EXTCLK    (1<<6)                     /* external clock */
#define PCA9685PW_MODE_1_AI        (1<<5)                     /* auto increment bit 0 */
#define PCA9685PW_MODE_1_SLEEP     (1<<4)                     /* low power mode/sleep enable/disable */
#define PCA9685PW_MODE_1_SUB1      (1<<3)                     /* PCA9685PW reponds to I2C subaddress 1 enable/disable */
#define PCA9685PW_MODE_1_SUB2      (1<<2)                     /* PCA9685PW reponds to I2C subaddress 2 enable/disable */
#define PCA9685PW_MODE_1_SUB3      (1<<1)                     /* PCA9685PW reponds to I2C subaddress 3 enable/disable */
#define PCA9685PW_MODE_1_ALLCALL   (1<<0)                     /* PCA9685PW reponds to led all call I2C address enable/disable */


/* PCA9685PW_MODE_2 bit definitions */

#define PCA9685PW_MODE_2_INVRT     (1<<4)                     /* output logic state inverted/not inverted */
#define PCA9685PW_MODE_2_OCH       (1<<3)                     /* output change on stop command/on ACK */
#define PCA9685PW_MODE_2_OUTDRV    (1<<2)                     /* outputs are configured with an open-drain-structure/totem-pole-structure */
#define PCA9685PW_MODE_2_OUTNE1    (1<<1)                     /* handling of outputs in dependency of !OE pin */
#define PCA9685PW_MODE_2_OUTNE0    (1<<0)                     /* handling of outputs in dependency of !OE pin */



class PCA9685_Dev : public I2C_Dev {
 public:
    PCA9685_Dev(FAR struct i2c_master_s *i2c_device);
    bool init(int pwm_freq);
    bool set_servo(int num, int val);
    void all_off();
 private:
    bool set_led(uint8_t const led,
                 uint16_t const on_time,
                 uint16_t const off_time);
};

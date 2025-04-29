/*
 * pca9685_driver.cpp
 */

#ifdef CONFIG_RCSERVO_INTERFACE

#include "pca9685_driver.h"
#include <unistd.h>
#include <stdio.h>
//#include "game_timer.h"

#define PCA9685_I2CADDR 0x40

PCA9685_Dev::PCA9685_Dev(FAR struct i2c_master_s *i2c_device)
    : I2C_Dev(i2c_device, PCA9685_I2CADDR)
{
}


bool PCA9685_Dev::init(int pwm_freq)
{
  uint8_t PCA9685PW_MODE_1_INITIAL_VALUE = PCA9685PW_MODE_1_AI | PCA9685PW_MODE_1_SLEEP | PCA9685PW_MODE_1_ALLCALL;

  if (!write_reg(PCA9685PW_MODE_1, PCA9685PW_MODE_1_INITIAL_VALUE)) {
      printf("ERROR: Could not set initial config for PCA9685PW_MODE_1\n");
      return false;
  }

  usleep(1000);

  short pre_scale = (25000000l)/(4095l*pwm_freq);

  if (!write_reg(PCA9685PW_PRESCALER, pre_scale)) {
      printf("ERROR: Could not set prescaler\n");
      return false;
  }

  usleep(1000);

  PCA9685PW_MODE_1_INITIAL_VALUE = PCA9685PW_MODE_1_AI | PCA9685PW_MODE_1_ALLCALL;

  if (!write_reg(PCA9685PW_MODE_1, PCA9685PW_MODE_1_INITIAL_VALUE)) {
      printf("ERROR: Could not set initial config for PCA9685PW_MODE_1\n");
      return false;
  }

  usleep(1000);


  /* Configure the PCA9685PW output drivers for totem-pole structure since
   * the output of the PCA9685PW are coupled with the Gates of MOSFET's
   * which then drive the LED's.  Since we have this kind of schematic
   * structure we also need to invert the output.
   */

  uint8_t const PCA9685PW_MODE_2_INITIAL_VALUE = PCA9685PW_MODE_2_OUTDRV;

  if (!write_reg(PCA9685PW_MODE_2, PCA9685PW_MODE_2_INITIAL_VALUE)) {
      printf("ERROR: Could not set initial config for PCA9685PW_MODE_2\n");
      return false;
  }

  /* A delay of 500 us is necessary since this is the maximum time which the
   * oscillator of the PCA9685PW needs to be up and running once sleep mode was
   * left.
   */

  usleep(1000);

  /* Turn all led off
   */
  for (int i = 0; i < 16; i++) {
      if (!set_led(i, 0, 0)) {
          printf("ERROR: Could not set led driver outputs to MODE2 (LED's brightness are "
              "controlled by pwm registers)\n");
          return false;
      }
  }

  return true;
}


bool PCA9685_Dev::set_servo(int num, int val)
{
    //if (!game_timer->game_end()) {
        uint16_t duty = (val*4096l)/20000l;
        return set_led(num, 0, duty);
    //}
    //return true;
}


void PCA9685_Dev::all_off()
{
    for (int i = 0; i < 16; i++) {
        set_led(i, 0, 0);
    }
}


bool PCA9685_Dev::set_led(uint8_t const led,
                          uint16_t const on_time,
                          uint16_t const off_time)
{
    uint8_t buf[3];
    uint8_t current_ledout_reg = PCA9685PW_LED_0_ON + 4 * led;

    buf[0] = current_ledout_reg;
    buf[1] = on_time & 0xff;
    buf[2] = on_time >> 8;

    if (!write_buf(buf, 3))
        return false;

    buf[0] = current_ledout_reg + 2;
    buf[1] = off_time & 0xff;
    buf[2] = off_time >> 8;

    if (!write_buf(buf, 3))
        return false;

    return true;
}

#endif


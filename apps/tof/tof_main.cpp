#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <pthread.h>

#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <nuttx/leds/userled.h>
#include <nuttx/timers/timer.h>
#include <sched.h>
#include <math.h>

#include "daemon_utils.h"

#include "tof_processing.h"
#include "params.h"
#include "shared_data.h"
#include "dds.h"

//#define VL53L5CX_DEFAULT_I2C_ADDRESS	        (((uint16_t)0x52) << 1)

extern "C" void set_i2c_rst(bool on);
extern "C" void set_LPn(bool on);
extern "C" void board_userled(int led, bool ledon);

t_daemon_struct       tof_daemon;
extern struct i2c_master_s *i2cmain;

SparkFun_VL53L5CX * myImager = NULL;
TOFProcessor * processor = NULL;

int __SENSOR_ID__ = 255;

void tof_reset(void)
{
    set_LPn(0);
    usleep(2000);
    set_LPn(1);

    set_i2c_rst(0);
    usleep(2000);
    set_i2c_rst(1);
    usleep(2000);
    set_i2c_rst(0);
    usleep(10000);
}

static int tof_start(int argc, FAR char **argv)
{

    board_userled(RED_LED, true);
    printf("[TOF] Starting...\n");

    printf("[TOF] Resetting\n");
    tof_reset();

    I2C_Dev * i2c_drv = new I2C_Dev(i2cmain, VL53L5CX_DEFAULT_I2C_ADDRESS >> 1);
    myImager = new SparkFun_VL53L5CX();

    VL53L5CX_ResultsData * measurementData = new VL53L5CX_ResultsData; // Result data class structure, 1356 byes of RAM

    if (!myImager->begin(0, *i2c_drv)) {
        printf("[TOF] Errors in initialization\n");
        return 1;
    }

    printf("[TOF] Initialization complete\n");

    myImager->setResolution(8*8); //Enable all 64 pads
    myImager->setIntegrationTime(30); // 30ms
    myImager->setRangingFrequency(10); // 5Hz
    myImager->startRanging();

    int imageResolution = myImager->getResolution(); //Query sensor for current resolution - either 4x4 or 8x8
    int imageWidth = sqrt(imageResolution); //Calculate printing width

    __SENSOR_ID__ = param_get_int("TOF_ID", 255);

    printf("[TOF] Start ranging ID=%d\n", __SENSOR_ID__);

    char * s_tof_mode = param_get_string("TOF_MODE", "");
    if (!strcmp(s_tof_mode, "single")) {
        int row = param_get_int("TOF_SLICE_ROW", -1);
        int col = param_get_int("TOF_SLICE_COL", -1);
        processor = new TOFSpotProcessor(row, col);
    }
    else if (!strcmp(s_tof_mode, "matrix"))
        processor = new TOFMatrixProcessor();

    board_userled(GREEN_LED, false);

    daemon_start_notify(&tof_daemon);

    while (true) {

        if (myImager->isDataReady()) {
            board_userled(RED_LED, true);
            if (myImager->getRangingData(measurementData)) { //Read distance data into array
                board_userled(RED_LED, false);
                processor->on_tof_data(measurementData, imageWidth);
            }
            board_userled(RED_LED, false);
        }
        usleep(1000);
    }


    return 0;
}

extern "C" int tof_main(int argc, char *argv[]) {
    if (argc < 2) {
    error:
        printf("Usage: %s [command]\n\n", argv[0]);
        printf("Command can be:\n");
        printf("start             starts the sender\n");
        printf("show              shows distances\n");
        printf("off               show off\n");
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "start")) {
        if (myImager != NULL) {
            printf("Already started\n");
            return EXIT_FAILURE;
        }
        if (!daemon_start("tof", &tof_daemon, tof_start,
                          SCHED_PRIORITY_MAX - 100, 2048)) {
            printf("Error in SENDER start\n");
            return EXIT_FAILURE;
        } else
            return EXIT_SUCCESS;
    }
    else if (!strcmp(argv[1], "show")) {
        if (myImager == NULL) {
            printf("Not started\n");
            return EXIT_FAILURE;
        }
        processor->dump(true);
    }
    else if (!strcmp(argv[1], "off")) {
        if (myImager == NULL) {
            printf("Not started\n");
            return EXIT_FAILURE;
        }
        processor->dump(false);
    }
    else if (!strcmp(argv[1], "a_off")) {
        if (myImager == NULL) {
            printf("Not started\n");
            return EXIT_FAILURE;
        }
        void * alarm_handle = DDS::get_handle("alarm");
        bool alarm = false;
        DDS::publish_handle(alarm_handle, (void *)&alarm, sizeof(bool));
    }
    else
        goto error;

    return EXIT_SUCCESS;
}

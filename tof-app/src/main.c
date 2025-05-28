#include "main.h"

#include <stdio.h>

#include "processing.h"
#include "can-io.h"
#include "tof.h"

int main(int argc, char *argv[]) {
    while(tof_init())
        printf("[Main] ToF initialization failed: retrying\n");
    while(processing_init())
        printf("[Main] Processing initialization failed: retrying\n");

    can_io_start();
    processing_start();
    return 0;
}

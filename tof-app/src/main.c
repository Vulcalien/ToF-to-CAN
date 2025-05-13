#include "main.h"

#include <pthread.h>

#include "processing.h"
#include "can-io.h"
#include "tof.h"

int main(int argc, char *argv[]) {
    while(tof_init())
        printf("[Main] ToF initialization failed: retrying\n");
    processing_init();

    can_io_start();
    processing_run(NULL);
    return 0;
}

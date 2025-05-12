#include "main.h"

#include <pthread.h>

#include "processing.h"
#include "can-io.h"

int main(int argc, char *argv[]) {
    processing_init();

    can_io_start();
    processing_run(NULL);
    return 0;
}

#include "transmit.h"

#include <stdio.h>

int transmit_timing;

int transmit_set_timing(int timing) {
    int err = 0;

    // TODO check if timing is valid
    if(true) transmit_timing = timing;
    else err = 1;

    printf(
        "[Transmit] setting transmit timing to %d (err=%d)\n",
        timing, err
    );
    return err;
}

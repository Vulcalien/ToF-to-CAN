#include "transmission.h"

#include <stdio.h>

int transmission_timing;

int transmission_set_timing(int timing) {
    int err = 0;

    // TODO check if timing is valid
    if(true) transmission_timing = timing;
    else err = 1;

    printf(
        "Transmission: setting transmission timing to %d (err=%d)\n",
        timing, err
    );
    return err;
}

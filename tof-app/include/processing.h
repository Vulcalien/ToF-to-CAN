#pragma once

#include "main.h"

#include <pthread.h>

#include "binarysem.h"

extern pthread_mutex_t processing_data_mutex;
extern binarysem       processing_data_available;
extern int             processing_data_length;

extern short processing_data[64];
extern bool  processing_threshold_status;

extern int processing_init(void);
extern int processing_start(void);

extern int processing_set_mode(int mode);
extern int processing_set_threshold(int threshold);
extern int processing_set_threshold_delay(int delay);

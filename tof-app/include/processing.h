#pragma once

#include "main.h"

#include <pthread.h>

#include "binarysem.h"

#define PROCESSING_DATA_MAX_LENGTH 64

extern pthread_mutex_t processing_data_mutex;
extern binarysem       processing_data_available;
extern int             processing_data_length;

extern short processing_data[PROCESSING_DATA_MAX_LENGTH];
extern bool  processing_below_threshold;
extern bool  processing_threshold_event;

extern int processing_init(void);
extern int processing_start(void);

extern void processing_pause(void);
extern void processing_resume(void);

extern int processing_set_mode(int mode);
extern int processing_set_threshold(int threshold);
extern int processing_set_threshold_delay(int delay);

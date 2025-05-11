#pragma once

#include "main.h"

#include <semaphore.h>

extern int  processing_distance;
extern bool processing_threshold_status;

extern sem_t processing_request_sample;
extern sem_t processing_sample_available;

extern void processing_init(void);

extern void *processing_run(void *arg);

extern int processing_set_mode(int mode);
extern int processing_set_threshold(int threshold);
extern int processing_set_threshold_delay(int delay);

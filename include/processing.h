#pragma once

#include "main.h"

#include <semaphore.h>

extern int  processing_distance;
extern bool processing_threshold_status;

extern sem_t processing_request_sample;
extern sem_t processing_sample_available;

extern void processing_init(void);
extern void processing_config(int mode, int threshold, int threshold_delay);

extern void *processing_run(void *arg);

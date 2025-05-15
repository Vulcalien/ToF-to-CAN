#pragma once

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int val;
} binarysem;

extern int binarysem_init(binarysem *sem, int initial_value);
extern int binarysem_destroy(binarysem *sem);

extern int binarysem_post(binarysem *sem);
extern int binarysem_wait(binarysem *sem);
extern int binarysem_trywait(binarysem *sem);

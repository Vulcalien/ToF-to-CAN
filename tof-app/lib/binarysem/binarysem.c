#include "binarysem.h"

#include <pthread.h>

// Credit: https://stackoverflow.com/a/7478825

int binarysem_init(binarysem *sem, int initial_value) {
    sem->val = (initial_value != 0);

    int err = pthread_mutex_init(&sem->mutex, NULL);
    if(err) return err;

    return pthread_cond_init(&sem->cond, NULL);
}

int binarysem_destroy(binarysem *sem) {
    int err = pthread_mutex_destroy(&sem->mutex);
    if(err) return err;

    return pthread_cond_destroy(&sem->cond);
}

int binarysem_post(binarysem *sem) {
    int err = pthread_mutex_lock(&sem->mutex);
    if(err) return err;

    if(sem->val == 0) {
        sem->val = 1;

        err = pthread_cond_signal(&sem->cond);
        if(err) return err;
    }
    return pthread_mutex_unlock(&sem->mutex);
}

int binarysem_wait(binarysem *sem) {
    int err = pthread_mutex_lock(&sem->mutex);
    if(err) return err;

    while(sem->val == 0) {
        err = pthread_cond_wait(&sem->cond, &sem->mutex);
        if(err) return err;
    }
    sem->val = 0;

    return pthread_mutex_unlock(&sem->mutex);
}

int binarysem_trywait(binarysem *sem) {
    int err = pthread_mutex_lock(&sem->mutex);
    if(err) return err;

    // read semaphore's value, then set it to 0 unconditionally
    const int val = sem->val;
    sem->val = 0;

    err = pthread_mutex_unlock(&sem->mutex);
    if(err) return err;

    // return 0 if waiting successfully, nonzero otherwise
    return (val == 0);
}

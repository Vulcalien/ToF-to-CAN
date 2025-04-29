/*
 * daemon_utils.h
 */

#pragma once

#include <pthread.h>
#include <semaphore.h>

typedef int (*daemon_func)(int argc, FAR char **argv);

typedef struct {
    sem_t startup_sem;
    int pid;
} t_daemon_struct;

int daemon_start(const char * name,  t_daemon_struct * data, daemon_func func, int priority, int stack_size);
void daemon_start_notify(t_daemon_struct * data);

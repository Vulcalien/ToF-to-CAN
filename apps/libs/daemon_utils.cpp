/*
 * daemon_utils.cpp
 */

#include "daemon_utils.h"
#include <sched.h>
#include <stdio.h>

int daemon_start(const char * name,  t_daemon_struct * data, daemon_func func, int priority, int stack_size)
{
    sem_init(&data->startup_sem, 0, 0);
    data->pid = task_create(name, priority, stack_size, func, NULL);
    if (data->pid < 0) {
      fprintf(stderr, "failed to start daemon\n");
      return 0;
    }
    sem_wait(&data->startup_sem);
    return 1;
}

void daemon_start_notify(t_daemon_struct * data)
{
    sem_post(&data->startup_sem);
}


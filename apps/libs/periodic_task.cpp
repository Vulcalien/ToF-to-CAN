/*
 * periodic_task.cpp
 */


#include <stdio.h>
#include <string.h>
#include "periodic_task.h"
#include <nuttx/clock.h>

PeriodicTask::PeriodicTask(PeriodicTaskScheduler & sched, const char * name, int period)
    : m_sched(sched), m_name(name), m_period(period), m_on(false), m_ticks(0), m_last_time(0.0)
{
}

void PeriodicTask::on()
{
    if (!m_on) {
        m_on = true;
        m_ticks = 0;
        m_last_time = TICK2SEC( (float)clock_systime_ticks() );
    }
};

void PeriodicTask::sched_set_first()
{
    m_sched.task_list = this;
    m_sched.task_list_end = this;
    m_task_next = NULL;
}

void PeriodicTask::sched_append()
{
    // insert at the end of the list
    if (m_sched.task_list == NULL) {
        m_sched.task_list = this;
    }
    else {
        m_sched.task_list_end->m_task_next = this;
    }
    m_sched.task_list_end = this;
    m_task_next = NULL;
}

void PeriodicTask::sched_insert()
{
    // insert at the head of the list
    if (m_sched.task_list == NULL) {
        m_sched.task_list = this;
        m_sched.task_list_end = this;
        m_task_next = NULL;
    }
    else {
        m_task_next = m_sched.task_list;
        m_sched.task_list = this;
    }
}

void PeriodicTask::execute(float current_time)
{
    if (!m_on) return;

    if (m_ticks == 0) {
        float delta = current_time - m_last_time;
        m_last_time = current_time;
        if (delta != 0)
            run(delta);
    }
    ++m_ticks;
    if (m_ticks >= m_period) m_ticks = 0;
}

void PeriodicTaskScheduler::run_all_tasks(float current_time)
{
    PeriodicTask * p = task_list;
    while (p != NULL) {
        p->execute(current_time);
        p = p->m_task_next;
    }
}


void PeriodicTaskScheduler::show_all_tasks()
{
    PeriodicTask * p = task_list;
    while (p != NULL) {
        printf("Task %s, Pointer %p\n", p->get_name(), p);
        p = p->m_task_next;
    }
}

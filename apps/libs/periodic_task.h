/*
 * periodic_task.h
 */

#ifndef __PERIODIC_TASK_H
#define __PERIODIC_TASK_H

#include <stdio.h>

class PeriodicTaskScheduler;

class PeriodicTask {

    friend class PeriodicTaskScheduler;

 public:
    PeriodicTask(PeriodicTaskScheduler & sched, const char * name, int period);
    void sched_set_first();
    void sched_append();
    void sched_insert();
    const char * get_name() { return m_name; };
    virtual void run(float delta_t) = 0;
    virtual void on();
    virtual void off() { m_on = false; };
    virtual bool is_on() { return m_on; };
    virtual void set_period(int v) { m_period = v; };

 private:
    void execute(float current_time);
    PeriodicTaskScheduler & m_sched;
    const char * m_name;
    int m_period;
    bool m_on;
    int m_ticks;
    float m_last_time;
    PeriodicTask * m_task_next;
};

class PeriodicTaskScheduler {
    friend class PeriodicTask;
 public:
    PeriodicTaskScheduler() : task_list(NULL), task_list_end(NULL) {};
    void run_all_tasks(float current_time);
    void show_all_tasks();
 private:
    PeriodicTask * task_list;
    PeriodicTask * task_list_end;
};

#endif

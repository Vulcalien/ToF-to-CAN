/*
 * sporadic_task.h
 */

#pragma once

#include <pthread.h>

class SporadicTask {
 public:
    SporadicTask(const char * name);
    virtual void init();
    void go();
    void stop();
    virtual void run() = 0;
    void execute();
    virtual void kill();
    bool is_running() { return m_running; };
 protected:
    const char * m_name;
    sem_t m_sem;
    bool m_running;
};

void * sporadic_thread_function(void *);

class SporadicThread : public SporadicTask {
 public:
    SporadicThread(const char * name) : SporadicTask(name) { };
    void init();
    void kill();
 private:
    pthread_t m_thread;
};

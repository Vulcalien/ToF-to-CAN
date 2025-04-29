/*
 * sporadic_task.cpp
 */

#include <stdio.h>
#include <signal.h>
#include "sporadic_task.h"

SporadicTask::SporadicTask(const char * name) : m_name(name)
{
}

void SporadicTask::init()
{
    m_running = true;
    sem_init(&m_sem, 0, 0);
}


void SporadicTask::execute()
{
    printf("[%s] starting\n", m_name);
    while (true) {
        sem_wait(&m_sem);
        if (!m_running) {
            return;
        }
        run();
    }
}


void SporadicTask::go()
{
    m_running = true;
    sem_post(&m_sem);
}


void SporadicTask::stop()
{
    m_running = false;
    sem_post(&m_sem);
}


void SporadicTask::kill()
{
}

void * sporadic_thread_function(void * x)
{
    SporadicThread * ptr = (SporadicThread *)x;
    ptr->execute();
    return nullptr;
}

void SporadicThread::init()
{
    SporadicTask::init();
    pthread_create(&m_thread, NULL, sporadic_thread_function, (void *)this);
}

void SporadicThread::kill()
{
    pthread_kill(m_thread, 9); // SIGKILL
}



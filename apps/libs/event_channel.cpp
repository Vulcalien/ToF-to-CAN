/*
 * event_channel.cpp
 */

#include <sys/time.h>
#include <errno.h>
#include "event_channel.h"

EventManager event_manager;

EventManager::EventManager()
    : m_head(nullptr)
{
}

void EventManager::add(EventChannel * ch)
{
    ch->m_next_channel = m_head;
    m_head = ch;
}


void EventManager::post_event(t_event_tag tag, void * args, bool enqueue)
{
    EventChannel * p = m_head;
    while (p != nullptr) {
        p->post_event(tag, args, enqueue);
        p = p->m_next_channel;
    }
}


void EventManager::clear_queue()
{
    EventChannel * p = m_head;
    while (p != nullptr) {
        p->clear_queue();
        p = p->m_next_channel;
    }
}


void EventManager::clear_queued_event(t_event_tag tag)
{
    EventChannel * p = m_head;
    while (p != nullptr) {
        p->clear_queued_event(tag);
        p = p->m_next_channel;
    }
}


EventChannel::EventChannel()
    : m_next_channel(nullptr)
{
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
    for (int i = 0; i < evtMAX;i++)
        for (int index = 0; index < evtSameTypeMAX; index++)
            m_waiting_events[i][index] = nullptr;
    event_manager.add(this);
    clear_queue();
}


void EventChannel::clear_queue()
{
    for (int i = 0; i < evtMAX;i++) {
        m_queued_events[i] = false;
    }
}


void EventChannel::clear_queued_event(t_event_tag tag)
{
    m_queued_events[tag] = false;
}


bool EventChannel::post_event(t_event_tag tag, void * arg, bool enqueue)
{
    pthread_mutex_lock(&m_mutex);

    Event ** p = m_waiting_events[(int)tag];

    for (int index = 0; index < evtSameTypeMAX; index++) {
        if (p[index] != nullptr) {
            if (p[index]->test(arg)) {
                m_result = tag;
                pthread_cond_signal(&m_cond);
                pthread_mutex_unlock(&m_mutex);
                return true;
            }
        }
    }

    if (enqueue) {
        m_queued_events[tag] = true;
    }

    pthread_mutex_unlock(&m_mutex);
    return false;
}


t_event_tag EventChannel::waitImpl(int millisecs, const Event *events_[], unsigned int events_count)
{
    pthread_mutex_lock(&m_mutex);

    for (int i = 0; i < (int)events_count;i++) {
        // first check if the event is still in the queue of the event_manager
        int tag = (int)events_[i]->m_tag;
        if (m_queued_events[tag]) {
            m_queued_events[tag] = false;
            m_result = (t_event_tag)tag;
            goto exit_success;
        }

        for (int index = 0; index < evtSameTypeMAX; index++) {
            if (m_waiting_events[tag][index] == nullptr) {
                m_waiting_events[tag][index] = (Event *)events_[i];
                break;
            }
        }
    }

    if (millisecs == 0)
        pthread_cond_wait(&m_cond, &m_mutex);
    else {
        struct timeval tv;
        struct timespec ts;
        gettimeofday(&tv, NULL);

        int32_t usec = tv.tv_usec + millisecs * 1000l;
        int32_t sec = tv.tv_sec + (usec / 1000000l);
        usec = usec % 1000000l;

        ts.tv_sec = sec;
        ts.tv_nsec = usec * 1000l; // here we have nanoseconds
        if (pthread_cond_timedwait(&m_cond, &m_mutex, &ts) == ETIMEDOUT)
            m_result = evtTimeout;
    }

 exit_success:
    t_event_tag ret_value = m_result;

    for (int i = 0; i < evtMAX;i++)
        for (int index = 0; index < evtSameTypeMAX; index++)
            m_waiting_events[i][index] = nullptr;

    pthread_mutex_unlock(&m_mutex);

    return ret_value;
}


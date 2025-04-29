/*
 * event_channel.h
 */

#pragma once

#include <pthread.h>
#include "bus_objects.h"

typedef enum {
    evtNone = -1,
    evtTimeout = 0,
    evtMC_PathDone,
    evtMC_MotorLocked,
    evtMC_PositionTarget,
    evtGP_FallingEdge,
    evtGP_RisingEdge,
    evtAX_TargetGot,
    evtSense_Color,
    evtOb_ObstacleGot,
    evtMAX
} t_event_tag;

class EventChannel;

class Event {
 public:
 Event(t_event_tag tag) : m_tag(tag) {  };
    virtual bool test(void * args) = 0;
    virtual const char * get_name() { return ""; };
    t_event_tag m_tag;
};



class FallingEdgeEvent : public Event {
 public:
    FallingEdgeEvent(int gpio_pin) : Event(evtGP_FallingEdge),m_gpio_pin(gpio_pin) { };
    bool test(void *arg) { return m_gpio_pin == (int)arg; };
 private:
    int m_gpio_pin;
};


class RisingEdgeEvent : public Event {
 public:
    RisingEdgeEvent(int gpio_pin) : Event(evtGP_RisingEdge),m_gpio_pin(gpio_pin) { };
    bool test(void *arg) { return m_gpio_pin == (int)arg; };
 private:
    int m_gpio_pin;
};



class EventManager {
 public:
    EventManager();
    void add(EventChannel * ch);
    void post_event(t_event_tag tag, void * evt, bool enqueue = false);
    void clear_queue();
    void clear_queued_event(t_event_tag tag);
 private:
    EventChannel * m_head;
};

extern EventManager event_manager;

#define evtSameTypeMAX   4

class EventChannel {
 public:
    EventChannel();
    void clear_queue();
    void clear_queued_event(t_event_tag tag);
    bool post_event(t_event_tag tag, void * arg, bool enqueue);
    friend class EventManager;
    t_event_tag waitImpl(int millisecs, const Event *events_[], unsigned int events_count);

 private:
    EventChannel * m_next_channel;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    Event * m_waiting_events[evtMAX][evtSameTypeMAX];
    t_event_tag m_result;
    bool m_queued_events[evtMAX];
};


extern EventChannel common_event_channel;

namespace Events {
    template<typename ...Args>
        inline t_event_tag wait(EventChannel & ch, int millisecs, const Args&... args)
        {
            const Event *list[] = { &args... };
            return ch.waitImpl(millisecs, list, sizeof(list) / sizeof(list[0]));
        };

    template<typename ...Args>
        inline t_event_tag wait(EventChannel & ch, const Args&... args)
        {
            const Event *list[] = { &args... };
            return ch.waitImpl(0, list, sizeof(list) / sizeof(list[0]));
        };

    // inline t_event_tag wait_motion_event() { return wait(common_event_channel,
    //                                                      PathDoneEvent(),
    //                                                      MotorLockedEvent(),
    //                                                      ObstacleGotEvent() ); };
    // inline t_event_tag wait_motion_event(Event & additional_event) { return wait(common_event_channel,
    //                                                                              PathDoneEvent(),
    //                                                                              MotorLockedEvent(),
    //                                                                              ObstacleGotEvent(),
    //                                                                              additional_event); };
    // inline t_event_tag wait_motion_event(int millisecs) { return wait(common_event_channel,
    //                                                                   millisecs,
    //                                                                   PathDoneEvent(),
    //                                                                   MotorLockedEvent(),
    //                                                                   ObstacleGotEvent() ); };

    // inline t_event_tag wait_lift_event(int a) { return wait(common_event_channel, AxisTargetGotEvent(a)); };

};



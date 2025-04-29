/*
 * hashtable.h
 */

#ifndef __HASHTABLE_H
#define __HASHTABLE_H

#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>

#define DATA_SIZE  24

typedef struct t_bucket_data {
    struct t_bucket_data * next;
    char key[24];
    uint32_t last_update;
    uint32_t deadline;
    bool stale;
    bool nullval;
    uint8_t data[DATA_SIZE];
    int len;
    pthread_cond_t wait_condition;
    int waiting_processes;
} t_bucket_data;

#define BUCKETS 24

typedef struct {
    pthread_mutex_t mutex;
    t_bucket_data * buckets[BUCKETS];
} t_hashtable;

namespace DDS {
    void init(void);
    bool insert(const char * key, int deadline);
    bool publish(const char * key, void * buffer, int len);
    bool publish_handle(void * handle, void * buffer, int len);
    bool get(const char * key, void * buffer, bool & stale);
    void * get_handle(const char * key);
    bool read_handle(void * handle, void * buffer, bool & stale);
    bool wait_handle(void * handle, void * buffer, bool & stale);
    void show();
    void display(const char * key);
};

#endif

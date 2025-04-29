/*
 * hashtable.cpp
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dds.h"

#define ENTER_CS()     pthread_mutex_lock(&dds_data.mutex)
#define EXIT_CS()      pthread_mutex_unlock(&dds_data.mutex)

t_hashtable dds_data;

int hash_func(char * key)
{
    int c = 0;
    while (*key != 0) {
        c += *key;
        key++;
    }
    return c % BUCKETS;
}

void DDS::init(void)
{
    pthread_mutex_init(&dds_data.mutex, NULL);
    for (int i = 0; i < BUCKETS;i++) {
        dds_data.buckets[i] = NULL;
    }
}

t_bucket_data * atomic_find(char * key)
{
    int bk = hash_func(key);
    t_bucket_data * d = dds_data.buckets[bk];
    while (d != NULL) {
        if (!strcmp(key, d->key))
            break;
    }
    return d;
}

bool DDS::insert(const char * key, int deadline)
{
    bool ret_val = false;
    ENTER_CS();
    if (atomic_find((char *)key) == NULL) {
        t_bucket_data * data = (t_bucket_data *)malloc(sizeof(t_bucket_data));
        strcpy(data->key, key);
        memset(data->data, 0, DATA_SIZE);
        data->nullval = true;
        data->stale = true;
        data->deadline = deadline;
        data->waiting_processes = 0;
        pthread_cond_init(&data->wait_condition, NULL);
        int bk = hash_func((char *)key);
        data->next = dds_data.buckets[bk];
        dds_data.buckets[bk] = data;
        ret_val = true;
    }
    EXIT_CS();
    return ret_val;
}

static int32_t get_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int32_t msec = tv.tv_usec / 1000l + tv.tv_sec * 1000l;
    return msec;
}

bool DDS::publish(const char * key, void * buffer, int len)
{
    if (len > DATA_SIZE)
        return false;
    bool ret_val = false;
    ENTER_CS();
    t_bucket_data * bk_ptr = atomic_find((char *)key);
    if (bk_ptr != NULL) {
        memcpy(bk_ptr->data, buffer, len);
        bk_ptr->len = len;
        bk_ptr->stale = false;
        bk_ptr->nullval = false;
        bk_ptr->last_update = get_ms();
        ret_val = true;
        if (bk_ptr->waiting_processes > 0) {
            pthread_cond_broadcast(&bk_ptr->wait_condition);
        }
    }
    EXIT_CS();
    return ret_val;
}


bool DDS::publish_handle(void * handle, void * buffer, int len)
{
    if (len > DATA_SIZE || handle == NULL)
        return false;
    bool ret_val = false;
    ENTER_CS();
    t_bucket_data * bk_ptr = (t_bucket_data *)handle;

    memcpy(bk_ptr->data, buffer, len);
    bk_ptr->len = len;
    bk_ptr->stale = false;
    bk_ptr->nullval = false;
    bk_ptr->last_update = get_ms();
    ret_val = true;
    if (bk_ptr->waiting_processes > 0) {
        pthread_cond_broadcast(&bk_ptr->wait_condition);
    }

    EXIT_CS();
    return ret_val;
}


static void aging(t_bucket_data * bk_ptr)
{
    int32_t curr = get_ms();
    if ( (curr - bk_ptr->last_update) > bk_ptr->deadline)
        bk_ptr->stale = true;
    else
        bk_ptr->stale = false;
}

bool DDS::get(const char * key, void * buffer, bool & stale)
{
    bool ret_val = false;
    ENTER_CS();
    t_bucket_data * bk_ptr = atomic_find((char *)key);
    if (bk_ptr != NULL) {
        if (!bk_ptr->nullval) {
            memcpy(buffer, bk_ptr->data, bk_ptr->len);
            aging(bk_ptr);
            stale = bk_ptr->stale;
            ret_val = true;
        }
	else
            stale = true;
    }
    EXIT_CS();
    return ret_val;
}

void * DDS::get_handle(const char * key)
{
    ENTER_CS();
    t_bucket_data * bk_ptr = atomic_find((char *)key);
    EXIT_CS();
    return bk_ptr;
}

bool DDS::read_handle(void * handle, void * buffer, bool & stale)
{
    bool ret_val = false;
    ENTER_CS();
    t_bucket_data * bk_ptr = (t_bucket_data *)handle;
    if (bk_ptr != NULL) {
        if (!bk_ptr->nullval) {
            memcpy(buffer, bk_ptr->data, bk_ptr->len);
            aging(bk_ptr);
            stale = bk_ptr->stale;
            ret_val = true;
        }
	else
            stale = true;
    }
    EXIT_CS();
    return ret_val;
}


bool DDS::wait_handle(void * handle, void * buffer, bool & stale)
{
    bool ret_val = false;
    ENTER_CS();
    t_bucket_data * bk_ptr = (t_bucket_data *)handle;
    if (bk_ptr != NULL) {
        bk_ptr->waiting_processes++;
        pthread_cond_wait(&bk_ptr->wait_condition, &dds_data.mutex);
        bk_ptr->waiting_processes--;
        if (!bk_ptr->nullval) {
            memcpy(buffer, bk_ptr->data, bk_ptr->len);
            aging(bk_ptr);
            stale = bk_ptr->stale;
            ret_val = true;
        }
	else
            stale = true;
    }
    EXIT_CS();
    return ret_val;
}


void DDS::show()
{
    printf("-------------------------------------------------------------\n");
    printf("Key\t\t\t\tNull\tStale\n");
    printf("-------------------------------------------------------------\n");
    ENTER_CS();
    for (int i = 0; i < BUCKETS;i++) {
        t_bucket_data * bk_ptr = dds_data.buckets[i];
        while (bk_ptr != NULL) {
            aging(bk_ptr);
            printf("%-30s\t%s\t%s\n", bk_ptr->key, (bk_ptr->nullval ? "Yes" : "No "), (bk_ptr->stale ? "Yes" : "No "));
            bk_ptr = bk_ptr->next;
        }
    }
    EXIT_CS();
    printf("-------------------------------------------------------------\n");
}

void DDS::display(const char * key)
{
    ENTER_CS();
    t_bucket_data * bk_ptr = atomic_find((char *)key);
    if (bk_ptr != NULL) {
        if (!bk_ptr->nullval) {
            printf("Data : ");
            for (int i = 0; i < bk_ptr->len;i++)
                printf("%02X ", bk_ptr->data[i]);
            printf("\n");
            printf("Stale: %s\n", (bk_ptr->stale ? "Yes" : "No "));
        }
        else
            printf("NULL");
    }
    else
        printf("Key not found\n");
    EXIT_CS();
}

#ifndef _ULTRA_QUEUE_H
#define _ULTRA_QUEUE_H
#include "queue.h"

typedef struct ultra_queue{
    ReqQueue _requests_waiting;
    ReqQueue _requests_working;
    pthread_cond_t _enqueue_allowed;
    pthread_cond_t _dequeue_allowed;
    int _max_capacity;
    pthread_mutex_t _mutex;
    int _size;
}* UltraQueue;

UltraQueue ultraQueueCreate(int max_capacity);
void insertRequest(UltraQueue uq, ReqDetails req_details);
void nonAtomic_insertRequest(UltraQueue uq, ReqDetails req_details);
ReqNode grabRequest(UltraQueue uq);
void nonAtomic_cancelRequest(UltraQueue uq);
void finishRequest(UltraQueue uq, ReqNode reqNode);
int getSizeUltraQueue(UltraQueue uq);


#endif

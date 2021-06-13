#ifndef _QUEUE_H
#define _QUEUE_H
#include <pthread.h>

typedef struct req_details{
    int _connfd;
   // struct timeval *_arrival_time;
   int _arrival_time;
}* ReqDetails;
ReqDetails reqDetailsCreate(int connfd, struct timeval * arrival_time);


typedef struct req_node{
   ReqDetails _req;
   struct req_node * _next;
}* ReqNode;

//ReqNode nodeCreate(int val, struct timeval * arrival);
ReqNode nodeCreate(ReqDetails req);

typedef struct req_queue{
    ReqNode _head;    
    ReqNode _tail;
    pthread_cond_t _enqueue_allowed;
    pthread_cond_t _dequeue_allowed;
    int _max_capacity;
    pthread_mutex_t _mutex;
    int _size;
}* ReqQueue;

ReqQueue queueCreate(int max_capactiy);
void enQueue(ReqQueue q,ReqDetails req);
ReqNode nonAtomic_enQueue(ReqQueue q, ReqDetails req);
int topQueue(ReqQueue q);
int deQueue(ReqQueue q);
//
int nonAtomic_deQueue(ReqQueue q);
// removes specific node from queue, updates the size 
void nonAtomic_removeRequest(ReqQueue q,ReqNode reqNode);
int getSizeQueue(ReqQueue q);

#endif
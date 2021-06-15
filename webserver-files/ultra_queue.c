#include "ultra_queue.h"

UltraQueue ultraQueueCreate(int max_capacity){
    UltraQueue new_uq = (UltraQueue)malloc(sizeof(*new_uq));
    new_uq->_requests_waiting= queueCreate(max_capacity);
    new_uq->_requests_working= queueCreate(max_capacity);
    new_uq->_size=0;
    new_uq->_max_capacity=max_capacity;
    pthread_cond_init(&new_uq->_enqueue_allowed,NULL);
    pthread_cond_init(&new_uq->_dequeue_allowed,NULL);
    pthread_mutex_init(&new_uq->_mutex,NULL);
    return new_uq;
}
void insertRequest(UltraQueue uq, ReqDetails req_details){
    pthread_mutex_lock(&uq->_mutex);
    while(uq->_size==uq->_max_capacity){
        pthread_cond_wait(&uq->_enqueue_allowed,&uq->_mutex);
    }
    nonAtomic_enQueue(uq->_requests_waiting,req_details);
    uq->_size++;
    pthread_cond_signal(&uq->_dequeue_allowed);
    pthread_mutex_unlock(&uq->_mutex);

}
void nonAtomic_insertRequest(UltraQueue uq, ReqDetails req_details){
    nonAtomic_enQueue(uq->_requests_waiting,req_details);
    uq->_size++;
}
ReqNode grabRequest(UltraQueue uq){
    pthread_mutex_lock(&uq->_mutex);
    while(uq->_requests_waiting->_size==0){
        pthread_cond_wait(&uq->_dequeue_allowed,&uq->_mutex);
    }
    ReqDetails req = nonAtomic_deQueue(uq->_requests_waiting);
    ReqNode reqNode = nonAtomic_enQueue(uq->_requests_working,req);

    pthread_mutex_unlock(&uq->_mutex);
    return reqNode;
}
void nonAtomic_cancelRequest(UltraQueue uq){
    nonAtomic_deQueue(uq->_requests_waiting);
    uq->_requests_waiting->_size--;
    uq->_size--;
}
void finishRequest(UltraQueue uq, ReqNode reqNode){
    pthread_mutex_lock(&uq->_mutex);
    nonAtomic_removeRequest(uq->_requests_working,reqNode);
    uq->_size--;
    pthread_cond_signal(&uq->_enqueue_allowed);
    pthread_mutex_unlock(&uq->_mutex);
}

int getSizeUltraQueue(UltraQueue uq){
    if (uq->_requests_waiting->_size + uq->_requests_working->_size != uq->_size){
        return -1; // indicating there is an error
    }
    else{
        return uq->_size;
    }
}
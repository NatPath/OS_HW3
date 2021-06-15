#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include "queue.h"
/*
ReqNode nodeCreate(int value, struct timeval * arrival_time){
    ReqNode node= (ReqNode)malloc(sizeof(*node));
    node->_value=value;
    node->_arrival_time=arrival_time;
    node->_next=NULL;
    //node->_prev=NULL;
    return node;
}
*/
ReqDetails reqDetailsCreate(int connfd, struct timeval * arrival_time){
    ReqDetails new_req = (ReqDetails)malloc(sizeof(*new_req));
    new_req->_connfd=connfd;
    new_req->_arrival_time= arrival_time;
    return new_req;

}
ReqNode nodeCreate(ReqDetails req){
    ReqNode node= (ReqNode)malloc(sizeof(*node));
    node->_req=req;
    node->_next=NULL;
    //node->_prev=NULL;
    return node;
}



ReqQueue queueCreate(int max_capacity){
    ReqQueue q = (ReqQueue)malloc(sizeof(*q));
    q->_head=NULL;
    q->_tail=NULL;
    q->_size=0;
    pthread_cond_init(&q->_enqueue_allowed,NULL);
    pthread_cond_init(&q->_dequeue_allowed,NULL);
    pthread_mutex_init(&q->_mutex,NULL);
    q->_max_capacity=max_capacity;
    return q;
}
ReqQueue queueDestroy(ReqQueue q){
    free(q);
}
 void enQueue(ReqQueue q,ReqDetails req){
    pthread_mutex_lock(&q->_mutex);
    while (q->_size == q->_max_capacity){
        pthread_cond_wait(&q->_enqueue_allowed,&q->_mutex);
    }
    nonAtomic_enQueue(q,req);
    /*
    ReqNode new_node = nodeCreate(req);
    if (q->_size==0){
         q->_head = new_node;
         q->_tail = new_node;
     }
     else{
         q->_tail->_next = new_node;
         q->_tail = q->_tail->_next;
     }
    q->_size++;
    */
    pthread_cond_signal(&q->_dequeue_allowed);
    pthread_mutex_unlock(&q->_mutex);


 }
 ReqNode nonAtomic_enQueue(ReqQueue q, ReqDetails req){
    ReqNode new_node = nodeCreate(req);
    if (q->_size==0){
         q->_head = new_node;
         q->_tail = new_node;
     }
     else{
         q->_tail->_next = new_node;
         q->_tail = q->_tail->_next;
     }
    q->_size++;
    return new_node;
 }
 ReqDetails topQueue(ReqQueue q){
     if (q->_size != 0){
         return q->_head->_req;
     }
     else{
         return NULL; // indicating an error, the queue is empty
     }
 }
 ReqDetails nonAtomic_deQueue(ReqQueue q){
    ReqDetails res;
    res = topQueue(q);
    ReqNode temp = q->_head;
    q->_head = q->_head->_next; 
    free(temp);
    q->_size--;
    return res;
 }
 ReqDetails deQueue(ReqQueue q){

    ReqDetails res;
    pthread_mutex_lock(&q->_mutex);
    while(q->_size==0){
        pthread_cond_wait(&q->_dequeue_allowed,&q->_mutex);
    }
    res = topQueue(q);
    ReqNode temp = q->_head;
    q->_head = q->_head->_next; 
    free(temp);
    q->_size--;

    pthread_cond_signal(&q->_enqueue_allowed);
    pthread_mutex_unlock(&q->_mutex);
    return res;
    
 }
void nonAtomic_removeRequest(ReqQueue q,ReqNode reqNode){
    reqNode = reqNode->_next;
    q->_size--;
}

 int getSizeQueue(ReqQueue q){
     return q->_size;
 }
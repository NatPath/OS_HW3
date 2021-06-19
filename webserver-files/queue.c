#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include "queue.h"
#include <math.h>
#include "segel.h"

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
    node->_prev=NULL;
    return node;
}



ReqQueue queueCreate(){
    ReqQueue q = (ReqQueue)malloc(sizeof(*q));
    q->_head=NULL;
    q->_tail=NULL;
    q->_size=0;
    pthread_cond_init(&q->_enqueue_allowed,NULL);
    pthread_cond_init(&q->_dequeue_allowed,NULL);
    pthread_mutex_init(&q->_mutex,NULL);
    return q;
}
void queueDestroy(ReqQueue q){
    free(q);
}
/*
 void enQueue(ReqQueue q,ReqDetails req){
    pthread_mutex_lock(&q->_mutex);
    while (q->_size == q->_max_capacity){
        pthread_cond_wait(&q->_enqueue_allowed,&q->_mutex);
    }
    nonAtomic_enQueue(q,req);
    pthread_cond_signal(&q->_dequeue_allowed);
    pthread_mutex_unlock(&q->_mutex);
 }
 */

 ReqNode nonAtomic_enQueue(ReqQueue q, ReqDetails req){
    ReqNode new_node = nodeCreate(req);
    if (q->_size==0){
         q->_head = new_node;
         q->_tail = new_node;
     }
     else{
         new_node->_prev = q->_tail;
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
    ReqDetails res = topQueue(q);
    if (res== NULL){
        return NULL;
    }
    ReqNode temp = q->_head;
    q->_head = q->_head->_next; 
    if ( q->_head != NULL){
        q->_head->_prev=NULL;
    }
    free(temp);
    q->_size--;
    return res;
 }
 /*
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
 */
void nonAtomic_removeRequest(ReqQueue q,ReqNode reqNode){
    if ( reqNode->_prev ==NULL){
        nonAtomic_deQueue(q);
    }
    else{
        reqNode->_prev->_next=reqNode->_next;
        if ( reqNode->_next != NULL){
            reqNode->_next->_prev= reqNode->_prev;
        }
        q->_size--;
        free(reqNode);
    }
}

 int getSizeQueue(ReqQueue q){
     return q->_size;
 }

 ReqNode getNthNodeQueue(ReqQueue q,int n){
     ReqNode p=q->_head;
     for (int i=0 ; i<n;i++){
         p=p->_next;
     }
     return p;
 }
void randomDropQueue(ReqQueue q){
    int num_to_drop= (int)ceil(q->_size*0.25);
    int *indexes_taken=(int*)malloc(sizeof(int)*q->_size);
    for (int i=0 ;i<q->_size;i++){
        indexes_taken[i]=0;
    }
    int *indexes_to_drop= (int*)malloc(sizeof(int)*num_to_drop);
    int i=0;
    while (i!=num_to_drop){
        int temp = rand()%q->_size;
        if (indexes_taken[temp]==0){
            indexes_taken[temp]=1;
            indexes_to_drop[i]=temp;
            i++;
        }
    }
    ReqNode *nodes_to_delete= (ReqNode*)malloc(sizeof(*nodes_to_delete)*num_to_drop);
    for (int i=0; i < num_to_drop; i++){
        nodes_to_delete[i]=getNthNodeQueue(q,i);
    }
    for (int i=0; i<num_to_drop;i++){
        Close(nodes_to_delete[i]->_req->_connfd);
        nonAtomic_removeRequest(q,nodes_to_delete[i]);
    }
    q->_size=q->_size-num_to_drop;
}
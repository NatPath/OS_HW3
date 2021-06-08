#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include "queue.h"
Node nodeCreate(int value){
    Node node= (Node)malloc(sizeof(*node));
    node->_value=value;
    node->_next=NULL;
    //node->_prev=NULL;
    return node;
}



Queue queueCreate(int max_capacity){
    Queue q = (Queue)malloc(sizeof(*q));
    q->_head=NULL;
    q->_tail=NULL;
    q->_size=0;
    pthread_cond_init(&q->_enqueue_allowed,NULL);
    pthread_cond_init(&q->_dequeue_allowed,NULL);
    pthread_mutex_init(&q->_mutex,NULL);
    q->_max_capacity=max_capacity;
    return q;
}
Queue queueDestroy(Queue q){
    free(q);
}
 void enQueue(Queue q,int new_val){
    
     
     /*
     
     */
    pthread_mutex_lock(&q->_mutex);
    while (q->_size == q->_max_capacity){
        pthread_cond_wait(&q->_enqueue_allowed,&q->_mutex);
    }
    Node new_node = nodeCreate(new_val);
    if (q->_size==0){
         q->_head = new_node;
         q->_tail = new_node;
     }
     else{
         q->_tail->_next = new_node;
         q->_tail = q->_tail->_next;
     }
    q->_size++;
    pthread_cond_signal(&q->_dequeue_allowed);
    pthread_mutex_unlock(&q->_mutex);


 }
 int topQueue(Queue q){
     if (q->_size != 0){
         return q->_head->_value;
     }
     else{
         return -1; // indicating an error, the queue is empty
     }
 }
 int nonAtomic_deQueue(Queue q){
    int res;
    res = topQueue(q);
    Node temp = q->_head;
    q->_head = q->_head->_next; 
    free(temp);
    q->_size--;
    return res;
    
 }
 int deQueue(Queue q){

    int res;
    pthread_mutex_lock(&q->_mutex);
    while(q->_size==0){
        pthread_cond_wait(&q->_dequeue_allowed,&q->_mutex);
    }
    res = topQueue(q);
    Node temp = q->_head;
    q->_head = q->_head->_next; 
    free(temp);
    q->_size--;

    pthread_cond_signal(&q->_enqueue_allowed);
    pthread_mutex_unlock(&q->_mutex);
    return res;
    
 }

 int getSizeQueue(Queue q){
     return q->_size;
 }
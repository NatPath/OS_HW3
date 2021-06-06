#include <stdio.h>
#include <queue.h>
typedef struct node{
   int _value;
   struct node * _next;
   //struct node * _prev;
}* Node;
Node nodeCreate(int value){
    Node node= (Node)malloc(sizeof(*node));
    node->_value=value;
    node->_next=NULL;
    //node->_prev=NULL;
    return node;
}

typedef struct queue{
    Node _head;    
    Node _tail;
    int _size;
    int _max_capacity;
}* Queue;

Queue queueCreate(int max_capacity){
    Queue q = (Queue)malloc(sizeof(*q));
    q->_head=NULL;
    q->_tail=NULL;
    q->_size=0;
    q->_max_capacity=max_capacity;
    return q;
}
 void enQueue(Queue q,int new_val){
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
 }
 int topQueue(Queue q){
     if (q->_size != 0){
         return q->_head->_value;
     }
     else{
         return -1; // indicating an error, the queue is empty
     }
 }
 int deQueue(Queue q){
     int res;
     if (q->_size != 0){
         res = topQueue(q);
         Node temp = q->_head;
         q->_head = q->_head->_next; 
         free(temp);
         q->_size--;
         return res;
     }
     else{
         return -1; // the queue is empty
     }
 }
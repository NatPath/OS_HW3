typedef struct node{
   int _value;
   struct node * _next;
}* Node;
Node nodeCreate(int val);
typedef struct queue{
    Node _head;    
    Node _tail;
    pthread_cond_t _enqueue_allowed;
    pthread_cond_t _dequeue_allowed;
    int _max_capacity;
    pthread_mutex_t _mutex;
    int _size;
}* Queue;
Queue queueCreate(int max_capactiy);
void enQueue(Queue q,int val);
int topQueue(Queue q);
int deQueue(Queue q);
int getSizeQueue(Queue q);
 int nonAtomic_deQueue(Queue q);
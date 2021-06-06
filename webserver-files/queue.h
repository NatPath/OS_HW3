typedef struct node * Node;
Node nodeCreate(int val);
typedef struct queue * Queue;
Queue queueCreate(int max_capactiy);
void enQueue(Queue q,int val);
int topQueue(Queue q);
int deQueue(Queue q);
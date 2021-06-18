#include <malloc.h>
#include <sys/time.h>
#include "queue.h"

void testRandomDropQueue(){
    ReqQueue q=queueCreate();
    struct timeval *arrival=(struct timeval*)malloc(sizeof(*arrival));
    gettimeofday(arrival,NULL);
    ReqDetails req=reqDetailsCreate(1,arrival);
    for(int i=0; i<100;i++){
        nonAtomic_enQueue(q,req);
    }
    randomDropQueue(q);
    printf("Queue size after randomdrop is : %d\n",q->_size);
    randomDropQueue(q);
    printf("Queue size after second randomdrop is : %d\n",q->_size);
    for(int i=0; i<100;i++){
        nonAtomic_enQueue(q,req);
    }
    randomDropQueue(q);
    printf("Queue size after third randomdrop is : %d\n",q->_size);
}
void testRemoveFromQueue(){
    ReqQueue q=queueCreate();
    struct timeval *arrival=(struct timeval*)malloc(sizeof(*arrival));
    gettimeofday(arrival,NULL);
    ReqDetails req=reqDetailsCreate(1,arrival);
    for(int i=0; i<100;i++){
        nonAtomic_enQueue(q,req);
    }
    ReqNode node = getNthNodeQueue(q,10);

    nonAtomic_removeRequest(q,node);
    printf("Queue size after removal is : %d\n",q->_size);
    int size=q->_size;
    for (int i=0; i<size;i++){
        nonAtomic_deQueue(q);
    }
    printf("Queue size after emptied is : %d\n",q->_size);

}

void testSimpleEnqueueAndDequeue(){
    ReqQueue q=queueCreate();
    struct timeval *arrival=(struct timeval*)malloc(sizeof(*arrival));
    gettimeofday(arrival,NULL);
    ReqDetails req=reqDetailsCreate(1,arrival);
    for(int i=0; i<100;i++){
        nonAtomic_enQueue(q,req);
    }
    printf("%d\n",q->_size);
    for (int i=0;i<100;i++){
        nonAtomic_deQueue(q);
    }
    printf("%d\n",q->_size);
}
int main(){
    testRandomDropQueue();

}
#include "segel.h"
#include "request.h"
#include "queue.h"
#include <sys/time.h>


// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
void getargs(int *port, int *num_of_threads, int *queue_size,  int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *num_of_threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
}


void thread_handles_request(Queue requests){
    // pass a struct as the argument, containing the Queue and the thread id.
    while(1){
        int connfd= deQueue(requests);
        struct timeval *time;
        gettimeofday(time,NULL);
        int dispatch = time->tv_sec*1000+time->tv_usec/1000 -arrival_time;  // ???
        requestHandle(connfd);
        Close(connfd);
    }
}
typedef enum { BLOCK , DROP_TAIL, DROP_HEAD , RANDOM_DROP, DROP_ERROR} OverLoadHandling;
OverLoadHandling choose_OL_Handling_Method(const char * schedalg){
    if (strcmp(schedalg,"block")){
        return BLOCK;
    }
    if (strcmp(schedalg,"dt")){
        return DROP_TAIL;
    }
    if (strcmp(schedalg,"dh")){
        return DROP_HEAD;
    }
    if (strcmp(schedalg,"random")){
        return RANDOM_DROP;
    }
    else{
        return DROP_ERROR;
    }
}
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    int num_of_threads, queue_size;

    getargs(&port,&num_of_threads,&queue_size, argc, argv);
    const char * schedalg = argv[4];
    OverLoadHandling olh = choose_OL_Handling_Method(schedalg);


    // 
    // HW3: Create some threads...
   
    Queue requests= queueCreate(queue_size);
    for (int i=0 ; i< num_of_threads;i++){
        pthread_t t;
        
        pthread_create(&t,NULL,thread_handles_request,(void*)requests);
    }
    //


    listenfd = Open_listenfd(port);
    switch(olh){
        case BLOCK:
        while (1) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
            enQueue(requests,connfd);
        }
        break;
        case DROP_TAIL:
        while (1) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
            pthread_mutex_lock(&requests->_mutex);
            int size = getSizeQueue(requests);
            if (size == requests->_max_capacity){
                close(connfd);
                pthread_mutex_unlock(&requests->_mutex);
            }
            else{
                pthread_mutex_unlock(&requests->_mutex);
                enQueue(requests, connfd);
            }
        }
        break;
        case DROP_HEAD:
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
            pthread_mutex_lock(&requests->_mutex);
            int size = getSizeQueue(requests);
            if (size == requests->_max_capacity){
                nonAtomic_deQueue(requests);
                enQueue(requests,connfd);
                pthread_mutex_unlock(&requests->_mutex);
            }
            else{
                pthread_mutex_unlock(&requests->_mutex);
                enQueue(requests, connfd);
            }
        break;
        case RANDOM_DROP:
        break;
    }
    struct timeval *time;
    int arrival_time;
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        gettimeofday(time,NULL);
        arrival_time = time->tv_sec*1000+time->tv_usec/1000;  // stat-req-arrival
        pthread_mutex_lock(&requests->_mutex);

        pthread_mutex_unlock(&requests->_mutex);

        enQueue(requests,connfd);
	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 
    }

}


    


 

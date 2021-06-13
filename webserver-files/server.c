#include "segel.h"
#include "request.h"
#include "ultra_queue.h"
#include "queue.h"
#include <sys/time.h>
#include "thread_data.h"

// a macro in the spirit of the one showed in the lecture to make syscalls safe.
// first argument is the syscall command.
// second arguments it the variable (which should be intialized beforehand) which will hold the return value
#define DO_SYS_RET( SYSCALL , RET_VALUE ) do { \
  RET_VALUE = SYSCALL ;\
  if ( RET_VALUE ==-1){\
    perror_wrap(#SYSCALL);\
    return;\
  }\
} while (0)\

#define DO_SYS( SYSCALL , RET_VALUE ) do { \
  RET_VALUE = SYSCALL ;\
  if ( RET_VALUE ==-1){\
    perror_wrap(#SYSCALL);\
  }\
} while (0)\

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


void thread_handles_request(UltraQueue requests){
    // pass a struct as the argument, containing the Queue and the thread id.
    /*
    typedef struct thread_data{
        int thread_count;
        int thread_static;
        int thread_dynamic;
    }* ThreadData;
    ThreadData this_thread = ThreadDataCreate(0,0,0);
    */
    int thread_data[3]={0,0,0}; // 1st : thread_count , 2nd : thread _static , 3rd : thread_dynamic
    int thread_count = 0;
    int thread_static = 0;
    int thread_dynamic = 0;
    ReqNode req_node = NULL;
    ReqDetails det = NULL;
    while(1){
        req_node = grabRequest(requests);
        det = req_node->_req;
        //int connfd= deQueue(requests);     
        int connfd = det->_connfd;
        struct timeval *time;
        gettimeofday(time,NULL);
        int dispatch = time->tv_sec*1000+time->tv_usec/1000 - (det->_arrival_time); 
        requestHandle(connfd);
        Close(connfd);
        finishRequest(requests,req_node);
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
   
    UltraQueue requests = ultraQueueCreate(queue_size);
    for (int i=0 ; i< num_of_threads;i++){
        pthread_t t;
        
        pthread_create(&t,NULL,thread_handles_request,(void*)requests);
    }
    //

    struct timeval *time;       
    int arrival_time;
    ReqDetails req; 

    listenfd = Open_listenfd(port);
    switch(olh){
        case BLOCK:
        while (1) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
            gettimeofday(time,NULL);
            arrival_time = time->tv_sec*1000+time->tv_usec/1000;  // stat-req-arrival
            req= reqDetailsCreate(connfd,arrival_time);
            insertRequest(requests, req);
        }
        break;
        case DROP_TAIL:
        while (1) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
            gettimeofday(time,NULL);
            arrival_time = time->tv_sec*1000+time->tv_usec/1000;  // stat-req-arrival
            req= reqDetailsCreate(connfd,arrival_time);
            pthread_mutex_lock(&requests->_mutex);
            int size = getSizeUltraQueue(requests);
            if (size == requests->_max_capacity){
                close(connfd);
                pthread_mutex_unlock(&requests->_mutex);
            }
            else{
                pthread_mutex_unlock(&requests->_mutex);
                insertRequest(requests, req);
            }
        }
        break;
        case DROP_HEAD:
        while(1) {
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
            gettimeofday(time,NULL);
            arrival_time = time->tv_sec*1000+time->tv_usec/1000;  // stat-req-arrival
            req= reqDetailsCreate(connfd,arrival_time);
            pthread_mutex_lock(&requests->_mutex);
            int size = getSizeUltraQueue(requests);
            if (size == requests->_max_capacity){
                nonAtomic_cancelRequest(requests);
                nonAtomic_insertRequest(requests,req);
                pthread_mutex_unlock(&requests->_mutex);
            }
            else{
                pthread_mutex_unlock(&requests->_mutex);
                insertRequest(requests,req);
            }
        }
        break;
        case RANDOM_DROP:
        break;
    }
    
    /*
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        
        enQueue(requests,connfd);
	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 
    }
    */

}


    


 

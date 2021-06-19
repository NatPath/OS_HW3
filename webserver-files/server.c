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

/*
void thread_handles_request(ThreadData statistics){
    UltraQueue requests = statistics->_requests;
    ReqNode req_node = NULL;
    ReqDetails det = NULL;
    struct timeval *arrival=(struct timeval*)malloc(sizeof(*arrival));
    struct timeval *dispatch= (struct timeval*)malloc(sizeof(*dispatch));
    while(1){
        req_node = grabRequest(requests);
        det = req_node->_req;
        int connfd = det->_connfd;
        arrival = det->_arrival_time;
        gettimeofday(dispatch,NULL);
        dispatch->tv_sec-=arrival->tv_sec;
        dispatch->tv_usec-=arrival->tv_usec;
        statistics->_arrival = arrival;
        statistics->_dispatch = dispatch;
        requestHandle(connfd,statistics);
        finishRequest(requests,req_node);
    }
}
*/

void* thread_handles_request(void* stats){
    ThreadData statistics = (ThreadData)stats;
    UltraQueue requests = statistics->_requests;
    ReqNode req_node = NULL;
    ReqDetails det = NULL;
    struct timeval *arrival=(struct timeval*)malloc(sizeof(*arrival));
    struct timeval *dispatch= (struct timeval*)malloc(sizeof(*dispatch));
    while(1){
        req_node = grabRequest(requests);
        det = req_node->_req;
        int connfd = det->_connfd;
        arrival = det->_arrival_time;
        gettimeofday(dispatch,NULL);
        dispatch->tv_sec-=arrival->tv_sec;
        dispatch->tv_usec-=arrival->tv_usec;
        statistics->_arrival = arrival;
        statistics->_dispatch = dispatch;
        requestHandle(connfd,statistics);
        Close(connfd);
        finishRequest(requests,req_node);
    }
    return NULL;
}

typedef enum { BLOCK , DROP_TAIL, DROP_HEAD , RANDOM_DROP, DROP_ERROR} OverLoadHandling;
OverLoadHandling choose_OL_Handling_Method(const char * schedalg){
    if (!strcmp(schedalg,"block")){
        return BLOCK;
    }
    if (!strcmp(schedalg,"dt")){
        return DROP_TAIL;
    }
    if (!strcmp(schedalg,"dh")){
        return DROP_HEAD;
    }
    if (!strcmp(schedalg,"random")){
        return RANDOM_DROP;
    }
    else{
        return DROP_ERROR;
    }
}
int main(int argc, char *argv[])
{
    printf("Server started\n");
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    int num_of_threads, queue_size;

    getargs(&port,&num_of_threads,&queue_size, argc, argv);
    const char * schedalg = argv[4];
    OverLoadHandling olh = choose_OL_Handling_Method(schedalg);


    // 
    // HW3: Create some threads...
   
    UltraQueue requests = ultraQueueCreate(queue_size);
    ThreadData statistics;
    for (int i=0 ; i< num_of_threads;i++){
        pthread_t t;
        statistics = threadDataCreate(i,requests);
        pthread_create(&t,NULL,thread_handles_request,(void*)statistics);
    }
    //

    struct timeval *arrival= (struct timeval*)malloc(sizeof(*arrival));       
    ReqDetails req; 

    listenfd = Open_listenfd(port);
    switch(olh){
        case BLOCK:
            while (1) {
                clientlen = sizeof(clientaddr);
                connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
                gettimeofday(arrival,NULL);
                req= reqDetailsCreate(connfd,arrival);
                insertRequest(requests, req);
            }
            break;
        case DROP_TAIL:
            while (1) {
                clientlen = sizeof(clientaddr);
                connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
                gettimeofday(arrival,NULL);
                req= reqDetailsCreate(connfd,arrival);
                pthread_mutex_lock(&requests->_mutex);
                int size = getSizeUltraQueue(requests);
                if (size == requests->_max_capacity){
                    Close(connfd);
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
                gettimeofday(arrival,NULL);
                req= reqDetailsCreate(connfd,arrival);
                pthread_mutex_lock(&requests->_mutex);
                int size = getSizeUltraQueue(requests);
                if (size == requests->_max_capacity){
                    if (requests->_requests_waiting->_size==0){
                        Close(connfd);
                    }
                    else{
                        nonAtomic_cancelRequest(requests);
                        nonAtomic_insertRequest(requests,req);
                    }
                    pthread_mutex_unlock(&requests->_mutex);
                }
                else{
                    pthread_mutex_unlock(&requests->_mutex);
                    insertRequest(requests,req);
                }
            }
            break;
        case RANDOM_DROP:
            while(1) {
                clientlen = sizeof(clientaddr);
                connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
                gettimeofday(arrival,NULL);
                req= reqDetailsCreate(connfd,arrival);
                pthread_mutex_lock(&requests->_mutex);
                int size = getSizeUltraQueue(requests);
                if (size == requests->_max_capacity){
                    if (requests->_requests_waiting->_size==0){
                        Close(connfd);
                    }
                    else{
                        randomDropQueue(requests->_requests_waiting);
                        nonAtomic_insertRequest(requests,req);
                        pthread_cond_signal(&requests->_dequeue_allowed);
                    }
                    pthread_mutex_unlock(&requests->_mutex);
                }
                else{
                    nonAtomic_insertRequest(requests,req);
                    pthread_cond_signal(&requests->_dequeue_allowed);
                    pthread_mutex_unlock(&requests->_mutex);
                    //insertRequest(requests,req);
                }
            }
        break;
        case DROP_ERROR:
        break;
    }
}


    


 

#include "segel.h"
#include "request.h"
#include "queue.h"

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
    while(1){
        int connfd= deQueue(requests);
        requestHandle(connfd);
        Close(connfd);
    }
}
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    int num_of_threads, queue_size;

    getargs(&port,&num_of_threads,&queue_size, argc, argv);


    // 
    // HW3: Create some threads...
    Queue requests= queueCreate(queue_size);
    for (int i=0 ; i< num_of_threads;i++){
        pthread_t t;
        pthread_create(&t,NULL,thread_handles_request,(void*)requests);

    }
    //


    listenfd = Open_listenfd(port);
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

}


    


 

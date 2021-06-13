#include "ultra_queue.h"
#include <sys/time.h>

typedef struct thread_data{
    int _thread_id;
    int _thread_count;
    int _thread_static;
    int _thread_dynamic;
    struct timeval *_arrival;
    struct timeval *_dispath;
    UltraQueue _requests;

}* ThreadData;

ThreadData threadDataCreate(int id,UltraQueue req);


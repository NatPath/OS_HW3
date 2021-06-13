#include "thread_data.h"

ThreadData threadDataCreate(int id,UltraQueue req){
    ThreadData new_data = (ThreadData)malloc(sizeof(*new_data));
    new_data->_thread_id = id;
    new_data->_thread_count = 0;
    new_data->_thread_dynamic = 0;
    new_data->_thread_static = 0;
    new_data->_arrival = NULL;
    new_data->_dispatch = NULL;
    new_data->_requests = req;

    return new_data;
}

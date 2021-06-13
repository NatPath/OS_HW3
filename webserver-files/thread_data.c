#include "thread_data.h"

ThreadData threadDataCreate(int id){
    ThreadData new_data = (ThreadData)malloc(sizeof(*new_data));
    new_data->_thread_id = id;
    new_data->_thread_count = 0;
    new_data->_thread_dynamic = 0;
    new_data->_thread_static = 0;
    new_data->_current_dispatch = 0;
}

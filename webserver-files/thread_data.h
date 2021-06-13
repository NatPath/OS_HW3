typedef struct thread_data{
    int _thread_id;
    int _thread_count;
    int _thread_static;
    int _thread_dynamic;
    int _current_dispatch;
}* ThreadData;

ThreadData threadDataCreate(int id);


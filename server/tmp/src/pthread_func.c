#include "../include/head.h"
int transFile(int client_fd);
void clean(void * lock)
{
    pthread_mutex_unlock((pthread_mutex_t *)lock);
    // pthread_exit(0); //不可以再执行这个exit(0)，因为可能会触发push-pop
}
void * pthread_func(void * p)
{
    thread_data_t * th_ptr = p;
    long int err;
    while(1)
    {
        pthread_mutex_lock(&lock);
        pthread_cleanup_push(clean, &lock);
        if(head == tail)
            pthread_cond_wait(&cond, &lock);
        th_ptr ->q_head = head;
        head = (head + 1) % queNum;
        pthread_cleanup_pop(0);
        pthread_mutex_unlock(&lock);
        err = transFile(th_ptr ->cli_arr[th_ptr ->q_head]);
    }
    return (void *)err;
}

#include "../include/interfaces.h"
#include "../include/SQL.h"
int servModuleConsole(int client_fd, queryTypePtr querys)
{
    int(*pFunc[servCnt])(int, queryTypePtr);
    pFunc[CD] = chDir;
    pFunc[LS] = listFile;
    pFunc[PUTS] = putFile;
    pFunc[GETS] = getFile;
    pFunc[RM] = rmFile;
    // pFunc[MV] = mvFile;
    // pFunc[6] = chLoginDir;
    // pFunc[CH_PASSWD] = chPasswd;
    /* prtWorkDir模块 */
    prtWorkDir(client_fd, querys);
    int FuncNo = 255, length;
    while(1)
    {
        LoopRecv(client_fd, &length, 4);
        LoopRecv(client_fd, &FuncNo, length);
        if(FuncNo < servCnt)
        {
            pFunc[FuncNo](client_fd, querys);
            FuncNo = 255;
        }
        else
            return -1;
    }
}

int checkUsr(int client_fd, queryTypePtr querys);
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
        queryTypePtr querys = (queryTypePtr)malloc(sizeof(queryType));
        bzero(querys, sizeof(queryType));
        SQL_Con(&querys ->sql);
        err = checkUsr(th_ptr ->cli_arr[th_ptr ->q_head], querys);
        if(err == 0)
            servModuleConsole(th_ptr ->cli_arr[th_ptr ->q_head], querys);
        close(th_ptr ->cli_arr[th_ptr ->q_head]);
        free(querys);
    }
    return (void *)err;
}

#include "../include/interfaces.h"

void sighandler(int p)
{
    write(cancel_fd[1], "cancel", 7);
}
int SOCKSServInit(int *sfd,const char * addr, const char * port);
int pthread_make(pthread_t ** threadArr, int threadNum);
void * pthread_func(void * p);
int EpollInAdd(int epfd, int sfd);
int main(int argc, char * argv[])
{
    /* 准备进程异常终止自启机制 */
    while(fork())
    {
        signal(SIGINT, SIG_IGN);
        int status;
        wait(&status);
        if(WIFEXITED(status))
        {
            printf("Turn off right.\n");
            exit(0);
        }
    }

    /* 初始化线程数、客户端下载等待队列、套接字 */
    ARGS_CHECK(argc, 5);
    queNum = atoi(argv[4]);
    int threadNum = atoi(argv[3]);
    int serv_fd;
    pthread_t * threadArr;
    threadArr = (pthread_t *)calloc(threadNum, sizeof(pthread_t));
    int * clientQue = (int *)calloc(queNum, sizeof(int));
    head = tail = 0;

    /* 创建线程锁、等待条件，创建线程 */
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&lock, NULL);
    for(int i = 0; i < threadNum; ++i)
    {
        thread_data_t thData;
        thData.cli_arr = clientQue;
        thData.q_head = head;
        pthread_create(&threadArr[i], NULL, pthread_func, (void *)&thData);
    }
    SOCKSServInit(&serv_fd, argv[1], argv[2]);

    /* epoll监听客户端发送的tcp报文，发送cond信号；监听服务端退出信号，有序pthread_cancel线程 */
    pipe(cancel_fd);
    int epfd = epoll_create(1);
    EpollInAdd(epfd, cancel_fd[0]);
    EpollInAdd(epfd, serv_fd);
    struct epoll_event event[2];
    signal(SIGINT, sighandler);
    while(1)
    {
        int evsNum = epoll_wait(epfd, event, 2, -1);
        for (int i = 0; i < evsNum; i++)
        {
            if(event[i].data.fd == cancel_fd[0])
            {
                int j;
                for(j = 0; j < threadNum; ++j)
                    pthread_cancel(threadArr[j]);
                for (j = 0; j < threadNum; ++j)
                    pthread_join(threadArr[j], NULL);
                printf("Exit success\n");
                exit(0);
            }
            if(event[i].data.fd == serv_fd)
            {
                int newfd = accept(serv_fd, NULL, NULL);
                while((tail + 1) % queNum == head) sleep(5);//判断队列为满，使用了互斥资源head
                clientQue[tail] = newfd; tail = (tail + 1) % queNum;
                pthread_cond_signal(&cond);
            }
        }
    }
    return 0;
}
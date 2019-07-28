#include "../include/head.h"
/* 发送文件长度，发送文件 */
typedef struct{
    int length;
    char buf[1000];
}little_train;
int LoopRecv(int fd, void * file, int length);

int transFile(int client_fd)
{
    /* 按名字查找文件 */
    char name[1024] = {0};
    int err;
    little_train train;
    bzero(&train, sizeof(train));
    LoopRecv(client_fd, &train.length, 4);
    LoopRecv(client_fd, train.buf, train.length);
    printf("%s\n", train.buf);
    strcpy(name, train.buf);
    __bzero(&train, sizeof(train));

    /* 发送文件名和文件大小 */
    train.length = strlen(name) + 1;
    strcpy(train.buf, name);
    err = send(client_fd, &train, 4 + train.length, 0);
    if(err == 0 || err == -1)
        return -1;
    ERROR_CHECK(err, -1, "send");
    struct stat info;
    int fd = open(name, O_RDWR);
    ERROR_CHECK(fd, -1, "open");
    fstat(fd, &info);
    
    __bzero(&train, sizeof(train));
    train.length = sizeof(info.st_size);
    memcpy(train.buf, &info.st_size, sizeof(info.st_size));
    printf("%ld\n", info.st_size);
    err = send(client_fd, &train, 4 + train.length, 0);
    ERROR_CHECK(err, -1, "send");
    if(err == 0)
        return -1;
    __bzero(&train, sizeof(train));
    long sentbytes; 
    /* 发文件 */
    sentbytes = sendfile(client_fd, fd, NULL, info.st_size);
    ERROR_CHECK(err, -1, "sendfile");
    printf("%ld\n", sentbytes);
    if(sentbytes != info.st_size)
        return -1;
    return 0;
}
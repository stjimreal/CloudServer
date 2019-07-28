#include "../include/head.h"
/* 循环调用 recv() 从 fd 关联的 TCP 流接收总长度 length 大小的 file */
int LoopRecv(int fd, void * file, int length)
{
    int downloaded = 0, bytes = 0;
    char * buf = (char *)file;
    if (length > 1000)
        return -1;
    while(downloaded < length)
    {
        bytes = recv(fd, buf + downloaded, length - downloaded, 0);
        if(bytes == 0)
        {
            close(fd);
            return -1;
        }
            
        ERROR_CHECK(bytes, -1, "recv");
        downloaded += bytes;
    }
    return 0;
}
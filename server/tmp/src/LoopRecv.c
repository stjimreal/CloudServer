#include "../include/head.h"
int LoopRecv(int fd, void * file, int length)
{
    int downloaded = 0, bytes = 0;
    char * buf = (char *)file;
    while(downloaded < length)
    {
        bytes = recv(fd, buf + downloaded, length - downloaded, 0);
        ERROR_CHECK(bytes, -1, "recv");
        downloaded += bytes;
    }
    return 0;
}
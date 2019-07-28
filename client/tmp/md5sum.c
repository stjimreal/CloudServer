/* 用于md5sum的实现 */
/* gcc动态库 -lcrypto */
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <openssl/md5.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#define READ_SIZE 4096
#define MD5_SIZE 128
int md5sum(char * md5, char * file, long size)
{
    char mdStr[MD5_SIZE];
    MD5(file, size, mdStr);
    int i;
    for (i = 0; i < strlen(mdStr); i++)
    {
        sprintf(&md5[i << 1], "%02x", 255 & (int)mdStr[i]);
        // printf("%02x", 255 & (int)mdStr[i]);
    }
    strcat(md5, "\0");
    // printf("%s: %lu", md5, strlen(md5));
    // printf("\n");
    return 0;
}
int main()
{
    int fd = open("hello", O_RDWR, NULL);
    struct stat fileInfo;
    fstat(fd, &fileInfo);
    // 不足 4 KB 的md5sum读数据也不受影响
    char * file = (char *)mmap(NULL, fileInfo.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    char md5[33] = {0};
    md5sum(md5, file, fileInfo.st_size);
    printf("%s\n",md5);
    close(fd);
    return 0;
}
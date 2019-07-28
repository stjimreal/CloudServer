/* 用于md5sum的实现，基于mmap的关联字符串 */
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
/* 用于md5sum的实现，基于mmap的关联字符串 */
/* gcc动态库 -lcrypto */
int md5sum(char * md5, const unsigned char * file, long size)
{
    unsigned char mdStr[MD5_SIZE];
    unsigned char * err =  MD5(file, size, mdStr);
    if(err  == NULL)
    {
        perror("MD5");
        return -1;
    }
    int i;
    for (i = 0; i < strlen(mdStr); i++)
        sprintf(&md5[i << 1], "%02x", 255 & (int)mdStr[i]);
    strcat(md5, "\0");
    return 0;
}
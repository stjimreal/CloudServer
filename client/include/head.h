#ifndef __HEAD_H__
#define __HEAD_H__

#define _GNU_SOURCE 
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <pthread.h>
#include <wait.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/time.h>
#define SALT_LENGTH 16
#define SHA512 6
#define SHA256 5
#define MD5 1

#define ARGS_CHECK(argc,val) {if(argc!=val)  {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retVal,funcName) {if(ret==retVal) {perror(funcName);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName) {if(ret!=0) {printf("%s:%s\n",funcName,strerror(ret));return (void *)-1;}}
#endif


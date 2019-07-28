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
#include <sys/file.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define ATTEMPTION_TIMES 20 
#define PAGE_SIZE 4096
#define PATH_SIZE 256

#define ARGS_CHECK(argc,val) {if(argc!=val)  {printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retVal,funcName) {if(ret==retVal) {perror(funcName);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName) {if(ret!=0) {printf("%s:%s\n",funcName,strerror(ret));return (void *)-1;}}
#define SQL_ERROR_CHECK(ret, retVal, connect) {if(ret == retVal) {printf("%d: %s\n",mysql_errno(connect) , mysql_error(connect));return -1;}}

#endif

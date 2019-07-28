#ifndef __TRANS_THREAD_H__
#define __TRANS_THREAD_H__
#include "head.h"

#define DefaultLocalPath "./"

typedef struct{
    int pthid, q_head;
    int * cli_arr;
}thread_data_t;

pthread_cond_t cond;
pthread_mutex_t lock;
int head, tail;
int queNum;
int cancel_fd[2];

typedef struct {
    int length;
    char buf[1000];
}little_train;


int checkin(int client_fd, char * introduction, const char * salt);
int setPasswd(int client_fd, char * introduction);
int md5sum(char * md5, const char * file, long size);
int SOCKSServInit(int *, const char *, const char *);
int EpollInAdd(int, int);
int LoopRecv(int fd, void * file, int length);
int cdClient(int client_fd, const char *path, char* introduction);
int lsClient(int client_fd, const char *path);
int downloadFileClient(int client_fd, const char *fileName);
int uploadFileClient(int client_fd, const char *fileName);
int rmFileClient(int client_fd, const char *fileName);
int chPasswdClient(int client_fd);
int chLoginDirClient(int client_fd);

#define sendCode(client_fd, NO) \
int code = 4;               \
int rtn = send(client_fd, &code, 4, 0); \
ERROR_CHECK(rtn, -1, "send");\
code = NO;                  \
rtn = send(client_fd, &code, 4, 0);     \
ERROR_CHECK(rtn, -1, "send")\

#endif
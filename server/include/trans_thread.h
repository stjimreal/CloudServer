#ifndef __TRANS_THREAD_H__
#define __TRANS_THREAD_H__
#include "head.h"

/* 默认文件路径 */
#define DefaultPath "/home/jimlau/arsonal"
/* cmdFunc的序号 */
#define CD 0
#define LS 1
#define PUTS 2
#define GETS 3
#define RM 4
#define MV 5
/* chLoginDir */
#define CH_LOG_DIR 6
/* chPasswd */
#define CH_PASSWD 7

/* ls缓冲的大小 */
#define LS_SIZE 65536

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



#endif
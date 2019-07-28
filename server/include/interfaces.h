#include "trans_thread.h"

int LoopRecv(int fd, void * file, int length);
int SOCKSServInit(int *sfd,const char * addr, const char * port);
void * pthread_func(void * p);
int EpollInAdd(int epfd, int sfd);
int downloadFile(int client_fd);
int md5sum(char * md5, const unsigned char * file, long size);

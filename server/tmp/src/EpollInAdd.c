#include <sys/epoll.h>
#include <stdio.h>
int EpollInAdd(int epfd, int sfd)
{
    int err;
    struct epoll_event evs;
    evs.events = EPOLLIN;
    evs.data.fd = sfd;
    err = epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &evs);
    return 0;
}
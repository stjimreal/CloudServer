#include "../include/head.h"
int SOCKSServInit(int *sfd,const char * addr, const char * port)
{
    int err = *sfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(err, -1, "socket");
    struct sockaddr_in socks;
    socks.sin_addr.s_addr = inet_addr(addr);
    socks.sin_family = AF_INET;
    socks.sin_port = htons(atoi(port));
    int val = 1;
    setsockopt(*sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
    bind(*sfd, (struct sockaddr *)&socks, sizeof(struct sockaddr));
    err = listen(*sfd, 5);
    ERROR_CHECK(err, -1, "listen");
    return 0;
}
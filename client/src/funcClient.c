#include "../include/bashUI.h"
#include "../include/trans_thread.h"

int cdClient(int client_fd, const char* path, char* introduction)
{
    sendCode(client_fd, CD);
    little_train buf;
    buf.length = strlen(path);
    strcpy(buf.buf, path);
    send(client_fd, &buf, buf.length + 4, 0);
    bzero(&buf, sizeof(buf));
    buf.length = strlen(introduction);
    strcpy(buf.buf, introduction);
    send(client_fd, &buf, buf.length + 4, 0);

    bzero(introduction, strlen(introduction));
    bzero(&buf, sizeof(buf));
    LoopRecv(client_fd, &buf.length, sizeof(int));
    LoopRecv(client_fd, introduction, buf.length);
    return 0;
}

int lsClient(int client_fd, const char* path)
{
    sendCode(client_fd, LS);
    ERROR_CHECK(rtn, -1, "sendCode");
    char str[LS_SIZE] = ".";
    int length = 1;
    if(path)
    {
        length = strlen(path);
        strcpy(str, path);
    }
    rtn = send(client_fd, &length, 4, 0);
    ERROR_CHECK(rtn, -1, "send");
    rtn = send(client_fd, str, length, 0);
    ERROR_CHECK(rtn, -1, "send");

    bzero(str, sizeof(str));
    LoopRecv(client_fd, &length, sizeof(int));
    LoopRecv(client_fd, str, length);
    printf("%s", str);
    return 0;
}

int uploadFileClient(int client_fd, const char *path)
{
    sendCode(client_fd, PUTS);
    little_train train;
    int nameLen = 0;
    int pathLen = strlen(path);
    while(pathLen >= 0 && path[pathLen] != '/') 
    {
        ++nameLen;  --pathLen;
    }
    strncpy(train.buf, &path[pathLen + 1], nameLen);
    train.length = nameLen;
    rtn = send(client_fd, &train, train.length + 4, 0);
    ERROR_CHECK(rtn, -1, "send");

    int fd = open(path, O_RDWR, 0666);
    struct stat info;
    fstat(fd, &info);
    char * str = mmap(NULL, info.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);

    /* 传md5sum */
    bzero(&train, sizeof(train));
    md5sum(train.buf, str, info.st_size);
    munmap(str, info.st_size);
    train.length = 32;
    rtn = send(client_fd, &train, 4 + train.length, 0);
    ERROR_CHECK(rtn, -1, "send");

    /* 接收回执，如果有则秒传，如果没有则开始上传 */
    bzero(&train, sizeof(train));
    LoopRecv(client_fd, &train.length, sizeof(int));
    LoopRecv(client_fd, &train.buf, train.length);
    if(train.buf[0] == 'y')
    {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_YELLOW);
        printf("上传成功\n");
        RESETFONT();
        return 0;
    } else if(train.buf[0] == -1)
    {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_WHITE);
        printf("File Exists.\n");
        RESETFONT();
        return -1;
    }
    /* 传文件大小 */
    bzero(&train, sizeof(train));
    train.length = 4;
    memcpy(train.buf, &info.st_size, sizeof(int));
    send(client_fd, &train, 4 + train.length, 0);

    /* 传文件 */
    sendfile(client_fd, fd, NULL, info.st_size);

    /* 接收回执，确定文件上传成功 */
    bzero(&train, sizeof(train));
    LoopRecv(client_fd, &train.length, sizeof(int));
    LoopRecv(client_fd, &train.buf, train.length);
    if(train.buf[0] == 0)
    {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_YELLOW);
        printf("上传成功\n");
        RESETFONT();
        return 0;
    } else {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_WHITE);
        printf("上传失败!\n");
        RESETFONT();
        return -1;
    }
    return 0;
}

int downloadFileClient(int client_fd, const char *fileName)
{
    sendCode(client_fd, GETS);
    little_train train;
    bzero(&train, sizeof(train));

    train.length = strlen(fileName);
    strcpy(train.buf, fileName);
    rtn = send(client_fd, &train, train.length + 4, 0);

    /* 创建文件 */
    char truePath[PATH_SIZE] = DefaultLocalPath;
    LoopRecv(client_fd, &train.length, 4);
    LoopRecv(client_fd, &train.buf, train.length);
    strcat(truePath, train.buf);
    int fd = open(truePath, O_CREAT|O_EXCL|O_RDWR, 0666); //创建并打开文件
    ERROR_CHECK(fd, -1, "open");

    /* 接收文件md5sum */
    little_train buf;
    bzero(&buf, sizeof(buf));
    LoopRecv(client_fd, &buf.length, 4);
    LoopRecv(client_fd, buf.buf, buf.length);
    char md5Before[33];
    strcpy(md5Before, buf.buf);

    /* 接收文件大小 */
    bzero(&buf, sizeof(buf));
    LoopRecv(client_fd, &buf.length, 4);
    LoopRecv(client_fd, buf.buf, buf.length);
    long int size, Downloaded = 0, records = 0,bytes;
    char * str;
    memcpy(&size, &buf.buf, buf.length);
    bzero(&buf, sizeof(buf));
    printf("The file size is %.2lf MB\n", (float)size / 1024 / 1024);

    long int bite = size * 0.02;
    struct timeval begin, end;
    ftruncate(fd, size);
    str = (char *)mmap(NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    ERROR_CHECK(str, (char *)-1, "mmap");

    /* DMA处理大文件 */
    gettimeofday(&begin, NULL);
    // LoopRecv(client_fd, str, size);
    while(Downloaded < size)
    {
        bytes = recv(client_fd, str + Downloaded, size - Downloaded, 0);
        ERROR_CHECK(bytes, -1, "recv");
        if(bytes == 0)
            break;
        Downloaded += bytes;
        if(Downloaded - records > bite)
        {
            printf("%.2f%%\r", (float)Downloaded / size * 100);
            records = Downloaded;
        }else if(Downloaded >= size)
            printf("100.00%%\n");
        fflush(stdout);
        MOVEDOWN(1);
    }   
    gettimeofday(&end, NULL);
    printf("time is %ld us\n", (-begin.tv_sec + end.tv_sec) * 1000000 - begin.tv_usec + end.tv_usec);

    char md5[33] = {0};
    rtn = md5sum(md5, str, size);
    ERROR_CHECK(rtn, -1, "md5sum");
    munmap(str, size);
    if(strcmp(md5Before, md5) == 0)
    {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_YELLOW);
        printf("下载成功\n");
        RESETFONT();
    }
        
    else {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_WHITE);
        printf("下载失败!\n");
        RESETFONT();
        remove(truePath);
    }
    return 0;
}

int rmFileClient(int client_fd, const char* path)
{
    sendCode(client_fd, RM);
    
    /* 发送路径 */
    little_train buf;
    bzero(&buf, sizeof(buf));
    strcpy(buf.buf, path);
    buf.length = strlen(path);
    send(client_fd, &buf, 4 + buf.length, 0);

    /* 接受回执 */
    bzero(&buf, sizeof(buf));
    LoopRecv(client_fd, &buf.length, sizeof(int));
    LoopRecv(client_fd, buf.buf, buf.length);
    if(buf.buf[0] == 0)
    {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_YELLOW);
        printf("删除成功！\n");
        RESETFONT();
    } else if(buf.buf[0] == 'n') {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_WHITE);
        printf("文件不存在！\n");
        RESETFONT();
    } else {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_RED);
        printf("删除失败！\n");
        RESETFONT();
    }
    return 0;
}

int chLoginDirClient(int client_fd)
{
    sendCode(client_fd, CH_LOG_DIR);
    little_train buf;
    printf("确定修改登录目录为当前目录吗？\n");
    *(int *)buf.buf = CH_LOG_DIR;
    buf.length = 4;
    rtn = send(client_fd, &buf, buf.length + 4, 0);
    ERROR_CHECK(rtn, -1, "send");
    SET_FRONT_COLOR(FRONT_YELLOW);
    BLACKEN();
    printf("修改成功！\n");
    RESETFONT();
    return 0;
}

int chPasswdClient(int client_fd)
{
    sendCode(client_fd, CH_PASSWD);
    little_train buf;
    *(int *)buf.buf = CH_PASSWD;
    buf.length = 4;
    rtn = send(client_fd, &buf, buf.length + 4, 0);
    ERROR_CHECK(rtn, -1, "send");
    setPasswd(client_fd, buf.buf);
    return 0;
}

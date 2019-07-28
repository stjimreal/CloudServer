#include "../include/SQL.h"
#include "../include/interfaces.h"

/* 检查用户名和密码 */
int checkUsr(int client_fd, queryTypePtr querys)
{
    /* 接受用户名 */
    little_train buf;
    int rtn;
    bzero(&buf, sizeof(buf));
    LoopRecv(client_fd, &buf.length, 4);
    rtn = LoopRecv(client_fd, buf.buf, buf.length);
    ERROR_CHECK(rtn, -1, "LoopRecv");
    strcpy(querys->usrname, buf.buf);

    /* 判断是否为新用户 */
    char * passwd = NULL;
    LoginCheck(querys, &passwd);
    bzero(&buf, sizeof(buf));
    if(passwd)
    {
        int cnt = 0, j = 0;
        while(passwd[j] != '\0' && cnt < 3)
        {
            if(passwd[j] == '$')
                ++cnt;
            buf.buf[j] = passwd[j];
            ++j;
        }
        buf.length = strlen(buf.buf);
        send(client_fd, &buf, 4 + buf.length, 0);

        int i = 0;
        while(i++ < ATTEMPTION_TIMES)
        {
            bzero(&buf, sizeof(buf));
            LoopRecv(client_fd, &buf.length, 4);
            LoopRecv(client_fd, buf.buf, buf.length);
            rtn = strcmp(passwd, buf.buf);
            bzero(&buf, sizeof(buf));
            buf.length = 4;
            if(rtn == 0)
            {
                *(int *)buf.buf = 0;
                send(client_fd, &buf, buf.length + 4, 0);
                break;
            }
            *(int *)buf.buf = -1;
            send(client_fd, &buf, buf.length + 4, 0);
        }
    } else {
        buf.length = -1;
        send(client_fd, &buf.length, 4, 0);
        // 成功返回 0，失败返回 -1。
        bzero(&buf, sizeof(buf));
        rtn =  LoopRecv(client_fd, &buf.length, 4);
        rtn |= LoopRecv(client_fd, buf.buf, buf.length);
        ERROR_CHECK(rtn, -1, "LoopRecv");
        fileInfo info;
        strcpy(info.fileName, "~");
        info.fileType = 'd';
        bzero(info.md5sum, sizeof(info.md5sum));
        rtn = createFile(querys, &info);
        rtn = rtn | changePasswd(querys, buf.buf);
        ERROR_CHECK(rtn, -1, "LoopRecv");
    }
    if(passwd)
        free(passwd);
    return rtn;
}

/* pwd查询当前路径✔️ */
int prtWorkDir(int client_fd, queryTypePtr querys)
{
    little_train normpathBuf;
    bzero(&normpathBuf, sizeof(little_train));
    long node = querys->cwdNode;
    char * stack[25];
    int top = 0;
    while(1)
    {
        PWD(querys->query, node);
        ReadyforQuery(querys, STDIN_FILENO);
        if(querys->queryRNum != 1)
        {
            break;
        }
        querys->queryRow = mysql_fetch_row(querys->queryRes);
        stack[top++] = (char *)calloc(64, sizeof(char));
        strcpy(stack[top - 1], querys->queryRow[0]);
        node = atol(querys->queryRow[1]);
        mysql_free_result(querys->queryRes);
    }
    normpathBuf.length = 0;
    while(top > 0)
    {
        int j = 0;
        while(stack[top - 1][j]) normpathBuf.buf[normpathBuf.length++] = stack[top - 1][j++];
        normpathBuf.buf[normpathBuf.length++] = '/';
        free(stack[--top]);
    }
    bzero(querys->query, sizeof(querys->query));
    sprintf(querys->query, "select fileName from fileList where node = %lu;", querys ->cwdNode);
    ReadyforQuery(querys, STDIN_FILENO);
    querys->queryRow = mysql_fetch_row(querys->queryRes);
    strcpy(&normpathBuf.buf[normpathBuf.length], querys->queryRow[0]);
    mysql_free_result(querys->queryRes);
    ++normpathBuf.length;
    node = send(client_fd, &normpathBuf, 4 + normpathBuf.length, 0);

    ERROR_CHECK(node, -1, "send");
    return 0;
}

/* cd，编号0，根据相对路径进入目录 */
/* 注意：cd现在还不会处理错误路径！ */
int chDir(int client_fd, queryTypePtr querys)
{
    int err;
    little_train buf, normpathBuf;
    bzero(&buf, sizeof(buf));
    bzero(&normpathBuf, sizeof(normpathBuf));
    LoopRecv(client_fd, &buf.length, sizeof(int));
    LoopRecv(client_fd, buf.buf, buf.length);
    LoopRecv(client_fd, &normpathBuf.length, sizeof(int));
    LoopRecv(client_fd, normpathBuf.buf, normpathBuf.length);
    long int node = forwardSRCH(querys, buf.buf, buf.length, &normpathBuf.buf[normpathBuf.length]);
    ERROR_CHECK(node, -1, "forwordSRCH");
    normpathBuf.length = strlen(normpathBuf.buf);
    err = send(client_fd, &normpathBuf, normpathBuf.length + 4, 0);
    ERROR_CHECK(err, -1, "send");
    querys->cwdNode = node;
    return 0;
}

/* ls ，编号1，返回目录中的子成员 */
/* fileType owner size LastChanged fileName */
int listFile(int client_fd, queryTypePtr querys)
{
    little_train buf;
    bzero(&buf, sizeof(buf));
    long node;
    int rtn;
    LoopRecv(client_fd, &buf.length, sizeof(int));
    LoopRecv(client_fd, buf.buf, buf.length);
    node = forwardSRCH(querys, buf.buf, buf.length, NULL);
    bzero(&buf, sizeof(buf));

    /* 开始查找成员 */
    if(node <= 0)
    {
        strcpy(buf.buf, "No such file or directory.\n");
        buf.length = 27;
        rtn = send(client_fd, &buf, buf.length + 4, 0);
        ERROR_CHECK(rtn, -1, "send");
        return -1;
    }
    char str[LS_SIZE] = {0};
    fileInfo info;
    char truePath[PATH_SIZE] = {0};
    long int fileSize = 0;
    int length = 0;
    char LastChanged[64] = {0};
    
    sprintf(querys->query, "select LastChanged from fileList where node = %ld;", node);
    ReadyforQuery(querys, STDIN_FILENO);
    if (querys->queryRNum > 0)
    {
        /* fileType size LastChanged fileName */
        querys->queryRow = mysql_fetch_row(querys->queryRes);
        sprintf(LastChanged, "%s", querys->queryRow[0]);
        sprintf(str, "%s%c %15ld %s %s\n", str, 'd', (long)0, LastChanged, ".");
    }
    mysql_free_result(querys->queryRes);

    sprintf(querys->query, "select LastChanged from fileList where node=(select prenode from fileList where node = %lu);", node);
    ReadyforQuery(querys, STDIN_FILENO);
    if (querys->queryRNum > 0)
    {
        /* fileType size LastChanged fileName */
        querys->queryRow = mysql_fetch_row(querys->queryRes);
        sprintf(LastChanged, "%s", querys->queryRow[0]);
        sprintf(str, "%s%c %15ld %s %s\n", str, 'd', (long)0, LastChanged, "..");
    }
    mysql_free_result(querys->queryRes);

    CWDSRCH(querys->query, node, buf.buf);
    ReadyforQuery(querys, STDIN_FILENO);
    if(querys->queryRNum <= 0)
    {
        length = strlen(str);
        rtn = send(client_fd, &length, 4, 0);
        ERROR_CHECK(rtn, -1, "send");
        rtn = send(client_fd, str, length, 0);
        ERROR_CHECK(rtn, -1, "send");
        return 0;
    }
    while((querys->queryRow = mysql_fetch_row(querys->queryRes)) != NULL)
    {
        strcpy(info.fileName, querys->queryRow[0]);
        info.fileType = querys->queryRow[3][0];
        strcpy(LastChanged, querys->queryRow[4]);
        if(info.fileType != 'd' && querys->queryRow[2] && strlen(querys->queryRow[2]))
        {
            strcpy(info.md5sum, querys->queryRow[2]);
            sprintf(truePath, "%s/%s", DefaultPath, info.md5sum);
            struct stat fileSIZE;
            stat(truePath, &fileSIZE);
            fileSize = fileSIZE.st_size;
        }
        sprintf(str, "%s%c %15ld %s %s\n", str, info.fileType, fileSize, LastChanged, info.fileName);
    }
    mysql_free_result(querys->queryRes);

    length = strlen(str);
    rtn = send(client_fd, &length, 4, 0);
    ERROR_CHECK(rtn, -1, "send");
    rtn = send(client_fd, str, length, 0);
    ERROR_CHECK(rtn, -1, "send");

    return 0;
}

/* 创建、接收文件 */
int putFile(int client_fd, queryTypePtr querys)
{
    /* 收到目标文件名、md5sum */
    char truePath[PATH_SIZE] = DefaultPath "/";
    char md5[33] = {0};
    fileInfo info;
    bzero(&info, sizeof(info));
    int err;
    little_train train;
    bzero(&train, sizeof(train));
    LoopRecv(client_fd, &train.length, 4);
    LoopRecv(client_fd, train.buf, train.length);
    strcpy(info.fileName, train.buf);
    info.fileType = 'f'; //可以根据后缀分析文件类型

    bzero(&train, sizeof(train));
    LoopRecv(client_fd, &train.length, 4);
    LoopRecv(client_fd, train.buf, train.length);
    strcpy(md5, train.buf);

    /* 重名检查、md5sum检查 */
    bzero(&train, sizeof(train));
    train.length = 1;
    CWDSRCH(querys->query, querys->cwdNode, info.fileName);
    ReadyforQuery(querys, STDIN_FILENO);
    mysql_free_result(querys->queryRes);
    if(querys->queryRNum != 0)
    {
        train.buf[0] = -1;
        err = send(client_fd, &train, 4 + train.length, 0);
        return -1;
    }
    MD5FILT(querys->query, md5);
    ReadyforQuery(querys, STDIN_FILENO);
    mysql_free_result(querys->queryRes);
    if(querys->queryRNum != 0)
    {
        strcpy(info.md5sum, md5);
        createFile(querys, &info);
        train.buf[0] = 'y';
        err = send(client_fd, &train, 4 + train.length, 0);
        ERROR_CHECK(err, -1, "send");
        return 0;
    }
    train.buf[0] = 'n';
    err = send(client_fd, &train, 4 + train.length, 0);
    ERROR_CHECK(err, -1, "send");


    strcat(truePath, md5);
    int fd = open(truePath, O_CREAT| O_EXCL | O_RDWR, 0666);
    long size, Downloaded, bytes;
    LoopRecv(client_fd, &train.length, sizeof(int));
    LoopRecv(client_fd, &size, train.length);

    int fds[2];
    pipe(fds);
    while(Downloaded < size)
    {
        bytes = splice(client_fd, NULL, fds[1], NULL, 4096, SPLICE_F_MOVE | SPLICE_F_MORE);
        if(bytes == 0)
            break;
        ERROR_CHECK(bytes, -1, "splice");
        err = splice(fds[0], NULL, fd, NULL, bytes, SPLICE_F_MOVE| SPLICE_F_MORE);
        ERROR_CHECK(bytes, -1, "splice");
        Downloaded += bytes;
    }

    /* 进行md5sum检查 */
    bzero(&train, sizeof(train));
    char * str = (char *)mmap(NULL, size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    md5sum(info.md5sum, str, size);
    munmap(str, size);
    train.length = 1;
    if(strcmp(md5, info.md5sum) == 0)
    {
        train.buf[0] = 0;
        createFile(querys, &info);
    } else {
        train.buf[0] = -1;
        remove(truePath);
    }
    send(client_fd, &train, train.length + 4, 0);
    return 0;
}

/* 发送文件长度，发送文件 */
int getFile(int client_fd, queryTypePtr querys)
{
    /* 收到目标文件路径 */
    char truePath[PATH_SIZE] = {0};
    char relevantPath[PATH_SIZE] = {0};
    char fileName[64] = {0};
    int err, pathLen;
    little_train train;
    bzero(&train, sizeof(train));
    LoopRecv(client_fd, &train.length, 4);
    LoopRecv(client_fd, train.buf, train.length);

    strcpy(relevantPath, train.buf);
    pathLen = train.length;

    int nameLen = 0;
    while(pathLen >= 0 && relevantPath[pathLen] != '/') 
    {
        ++nameLen;  --pathLen;
    }
    strncpy(fileName, &relevantPath[pathLen + 1], nameLen);
    if(relevantPath[pathLen] == '/')
        relevantPath[pathLen] = '\0';
    long node = forwardSRCH(querys, relevantPath, pathLen, NULL);

    /* 发送文件名称 */
    err = send(client_fd, &nameLen, 4, 0);
    ERROR_CHECK(err, -1, "send");
    err = send(client_fd, fileName, nameLen, 0);
    ERROR_CHECK(err, -1, "send");
    fileInfo queryInfo;

    /* 按结点文件夹查找目录下匹配姓名的文件md5sum，文件类型，文件结点 */
    CWDSRCH(querys->query, node, fileName);
    ReadyforQuery(querys, STDIN_FILENO);
    if(querys->queryRNum != 1)
    {
        return -1;
    }
    querys->queryRow = mysql_fetch_row(querys->queryRes);
    queryInfo.node = atol(querys->queryRow[1]);
    strcpy(queryInfo.md5sum, querys->queryRow[2]);
    queryInfo.fileType = querys->queryRow[3][0];
    mysql_free_result(querys->queryRes);

    if(queryInfo.fileType == 'd')
        return -1;

    sprintf(truePath, "%s/%s", DefaultPath, queryInfo.md5sum); 

    /* 发送md5sum和文件大小 */
    bzero(&train, sizeof(train));
    train.length = strlen(queryInfo.md5sum);
    err = send(client_fd, &train.length, 4, 0);
    ERROR_CHECK(err, -1, "send");
    err = send(client_fd, queryInfo.md5sum, train.length, 0);
    ERROR_CHECK(err, -1, "send");
    
    struct stat info;
    int fd = open(truePath, O_RDWR, NULL);
    ERROR_CHECK(fd, -1, "open");
    fstat(fd, &info);
    __bzero(&train, sizeof(train));
    train.length = sizeof(info.st_size);
    memcpy(train.buf, &info.st_size, sizeof(info.st_size));
    err = send(client_fd, &train, 4 + train.length, 0);
    ERROR_CHECK(err, -1, "send");
    if(err == 0)
        return -1;
    __bzero(&train, sizeof(train));
    long sentbytes; 
    /* 发文件 */
    sentbytes = sendfile(client_fd, fd, NULL, info.st_size);
    ERROR_CHECK(err, -1, "sendfile");
    if(sentbytes != info.st_size)
        return -1;
    return 0;
}

/* 删除文件，目前仅限于文件删除 */
int rmFile(int client_fd, queryTypePtr querys)
{
    /* 获取地址 */
    little_train train;
    bzero(&train, sizeof(train));
    LoopRecv(client_fd, &train.length, sizeof(int));
    LoopRecv(client_fd, train.buf, train.length);

    char fileName[64] = {0};
    int pathLen = train.length;
    int nameLen = 0;
    while(pathLen >= 0 && train.buf[pathLen] != '/') 
    {
        ++nameLen;  --pathLen;
    }
    strncpy(fileName, &train.buf[pathLen + 1], nameLen);
    if(train.buf[pathLen] == '/')
        train.buf[pathLen] = '\0';
    long node = forwardSRCH(querys, train.buf, pathLen, NULL);

    /* 文件检查 */
    char md5[33] = {0};
    bzero(&train, sizeof(train));
    train.length = 1;
    CWDSRCH(querys->query, node, fileName);
    ReadyforQuery(querys, STDIN_FILENO);
    querys->queryRow = mysql_fetch_row(querys->queryRes);
    if(querys->queryRow[2])
        strcpy(md5, querys->queryRow[2]);
    mysql_free_result(querys->queryRes);
    if(querys->queryRNum == 0)
    {
        train.buf[0] = 'n';
        send(client_fd, &train, train.length + 4, 0);
        return 0;
    } else if(querys->queryRNum != 1)
    {
        train.buf[0] = -1;
        send(client_fd, &train, train.length + 4, 0);
        return -1;
    }

    RMFILE(querys->query, node, fileName);
    ReadyforQuery(querys, STDOUT_FILENO);

    train.buf[0] = 0;
    send(client_fd, &train, train.length + 4, 0);

    /* md5过滤 */
    MD5FILT(querys->query, md5);
    ReadyforQuery(querys, STDIN_FILENO);
    mysql_free_result(querys->queryRes);
    if(querys->queryRNum == 0)
    {
        char truePath[PATH_SIZE] = DefaultPath "/";
        strcat(truePath, md5);
        remove(truePath);
    }

    return 0;
}
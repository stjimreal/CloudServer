#define _GNU_SOURCE
#include "../include/bashUI.h"
#include "../include/trans_thread.h"

int main(int argc, char * argv[])
{
    /* 客户端连接 */
    if(argc < 3)
    {
        printf("./client [ip] [port]\n");
        return -1;
    }
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servSock;
    bzero(&servSock, sizeof(struct sockaddr));
    servSock.sin_family = AF_INET;
    servSock.sin_port = htons(atoi(argv[2]));
    servSock.sin_addr.s_addr = inet_addr(argv[1]);
    int rtn;
    rtn = connect(client_fd, (struct sockaddr *)&servSock, sizeof(servSock));
    ERROR_CHECK(rtn, -1, "connect");
    little_train buf;
    char usrName[64] = {0};
    char introduction[256];

    /* 网盘登录，用户名、密码检查 */
    strcpy(introduction, "请输入用户名");
    beautifyGets(NULL, introduction, FRONT_PURPLE);
    fgets(usrName, LINE_SIZE, stdin);
    if(strlen(usrName) > 0)
        buf.length = strlen(usrName) - 1;
    usrName[buf.length] = '\0';
    
    rtn = send(client_fd, &buf.length, 4, 0);
    ERROR_CHECK(rtn, -1, "send");
    rtn = send(client_fd, usrName, buf.length, 0);
    ERROR_CHECK(rtn, -1, "send");
    bzero(&buf, sizeof(buf));
    LoopRecv(client_fd, &buf.length, 4);
    if(buf.length > 0)
    {
        LoopRecv(client_fd, &buf.buf, buf.length);
        rtn = checkin(client_fd, introduction, buf.buf);
    }else{
        beautifyGets(NULL, "确定注册该用户名吗？(y/n)", FRONT_PURPLE);
        fgets(buf.buf, LINE_SIZE, stdin);
        buf.buf[strlen(buf.buf) - 1] = '\0';
        if(buf.buf[0] == 'y'|| buf.buf[0] == 'Y')
        {
            bzero(&buf, sizeof(buf));
            buf.length = strlen(buf.buf);
            setPasswd(client_fd, introduction);
            SET_FRONT_COLOR(FRONT_YELLOW);
            BLACKEN();
            printf("Success!\n");
            goto end;
        }else
            goto end;
    }
    
    /* 获取当前目录✔️ */
    bzero(introduction, sizeof(introduction));
    LoopRecv(client_fd, &buf.length, 4);
    LoopRecv(client_fd, introduction, buf.length);
    /* 发送指令 */
    bzero(&buf, sizeof(buf));
    char NickName[64] = NICKNAME;
    if(strcmp(NICKNAME, "") == 0)
        strcpy(NickName, usrName);
    char line[256] = {0};
    char * wordArr[ARGS_NUM];
    while(1)
    {
        int i = 0, wordCnt = 0, incentive = 1;
        bzero(wordArr, sizeof(char *) * ARGS_NUM);
        while (wordCnt < ARGS_NUM)
        {
            bzero(line, sizeof(line));
            if(incentive)
            {
                beautifyGets(NickName, introduction, FRONT_BLUE);
                incentive = 0;
            }
            fgets(line, 256, stdin);
            i = 0;
            while(line[i] != '\n'&& wordCnt < ARGS_NUM)
            {
                int j = 0;
                while(line[i] == ' ') ++i; 
                while(line[i] != '\n' && line[i] != ' ') {++j;++i;}
                if(j == 0)
                    continue;
                wordArr[wordCnt] = (char *)malloc(j + 1);
                bzero(wordArr[wordCnt], j + 1);
                strncpy(wordArr[wordCnt], &line[i - j], j);
                /* 可以发包了 */
                ++wordCnt;
            }
            if(wordCnt == 0 || strcmp(wordArr[0], "help") == 0)
            {
                printf("[command](ls/gets/puts/rm/cd/mv) [path_to_file]\n");
                incentive = 1;
            }
            else if(strcmp(wordArr[0], "ls") == 0)
                wordCnt = ARGS_NUM;
            else if(wordCnt < ARGS_NUM)
                printf("请输入路径：");
        }
        if(wordCnt > 0) switch (wordArr[0][0])
        {
        case 'c':
            switch(wordArr[0][1])
            {
                case 'd': cdClient(client_fd, wordArr[1], introduction);break;
                case 'h':
                if(strcmp(wordArr[0], "chPasswd"))
                    chPasswdClient(client_fd);
                if(strcmp(wordArr[0], "chLoginDir"))
                    chLoginDirClient(client_fd);
                break;
                default:
                    break;
            }
            break;
        case 'l':
            lsClient(client_fd, wordArr[1]);break;
        case 'p':
            uploadFileClient(client_fd, wordArr[1]);break;
        case 'g':
            downloadFileClient(client_fd, wordArr[1]);break;
        case 'r':
            rmFileClient(client_fd, wordArr[1]);break;
        default:
            break;
        }

        while(wordCnt > 0)
            free(wordArr[--wordCnt]);
    }

end:
    close(client_fd);
    printf("再见！\n");
    return 0;
}
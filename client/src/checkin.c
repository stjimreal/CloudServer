#include "../include/bashUI.h"
#include "../include/trans_thread.h"
int genSalt(char * salt, int length, int type);
/* 内部接口，密码设置 */
int setPasswd(int client_fd, char * introduction)
{
    char passwd[128] = {0}, check[128] = {0};
    int rtn, length;
    while(1)
    {
        /* 输入密码： */
        static int cnt = 0;
        strcpy(introduction, "请输入新密码");
        beautifyGets(NULL, introduction, FRONT_PURPLE);
        fgets(passwd, LINE_SIZE, stdin);
        passwd[strlen(passwd) - 1] = '\0';
        printf("\033[1A\r\033[14C********************************\n");
        strcpy(introduction, "请再次确认");
        beautifyGets(NULL, introduction, FRONT_PURPLE);
        fgets(check, LINE_SIZE, stdin);
        check[strlen(check) - 1] = '\0';
        printf("\033[1A\r\033[12C**********************************\n");
        if(strcmp(passwd, check) == 0)
            break;
        if(cnt ++ > 5)
        {
            SET_FRONT_COLOR(FRONT_RED);
            BLACKEN();
            printf("Too many attemptions!\n");
            close(client_fd);
            exit(-1);
        }
    }
    char salt[24] = {0};

    genSalt(salt, SALT_LENGTH, SHA512);
    length = strlen(crypt(passwd, salt));
    const char * cryptedCode = crypt(passwd, salt);
    rtn = send(client_fd, &length, 4, 0);
    ERROR_CHECK(rtn, -1, "send");
    rtn = send(client_fd, cryptedCode, length, 0);
    ERROR_CHECK(rtn, -1, "send");
    return 0;
}

/* 内部接口，密码检查 */
int checkin(int client_fd, char * introduction, const char * salt)
{
    char passwd[128] = {0};
    little_train buf;
    int rtn;
    while(1)
    {
        /* 输入密码： */
        strcpy(introduction, "请输入密码");
        beautifyGets(NULL, introduction, FRONT_PURPLE);
        fgets(passwd, LINE_SIZE, stdin);
        passwd[strlen(passwd) - 1] = '\0';
        printf("\033[1A\r\033[12C********************************\n");
        bzero(&buf, sizeof(buf));
        strcpy(buf.buf ,crypt(passwd, salt));
        buf.length = strlen(buf.buf);
        rtn = send(client_fd, &buf, 4 + buf.length, 0);
        ERROR_CHECK(rtn, -1, "send");
        bzero(&buf, sizeof(little_train));
        LoopRecv(client_fd, &buf.length, 4);
        LoopRecv(client_fd, buf.buf, buf.length);
        if(*(int *)buf.buf == 0)
        {
            SET_FRONT_COLOR(FRONT_YELLOW);
            BLACKEN();
            printf("Success!\n");
            RESETFONT();
            break;
        }
        printf("%d\n", *(int *)buf.buf);
        SET_FRONT_COLOR(FRONT_RED);
        BLACKEN();
        printf("Wrong Password!\n");
    }
    return 0;
}
#include "../include/SQL.h"

/* 登录检查，如果有密码则passwd赋值，反之，则passwd赋值NULL */
int LoginCheck(queryTypePtr querys, char ** passwdPtr)
{
    CHECKUSR(querys->query, querys->usrname);
    ReadyforQuery(querys, 0);
    if(querys -> queryRNum != 1)
    {
        *passwdPtr = NULL;
        return 0;
    }
    *passwdPtr = (char *)malloc(sizeof(char) * 128);
    while ((querys -> queryRow = mysql_fetch_row(querys -> queryRes)) != NULL)
    {
        for (int i = 0; i < querys->queryCNum; ++i)
        {
            if(i == 0)
                strcpy(*passwdPtr, querys -> queryRow[i]);
            if(i == 1)
                querys -> cwdNode = atol(querys -> queryRow[i]);
        }
    }
    mysql_free_result(querys->queryRes);
    querys->queryRes = NULL;
    return 0;
}

int changePasswd(queryTypePtr querys, const char * passwd)
{
    CHANGEPASSWD(querys->query, passwd, querys ->usrname);
    ReadyforQuery(querys, 1);
    return 0;
}

int createFile(queryTypePtr querys, fileInfoPtr info)
{
    /* 注意prenode = cwdNode */
    ADDFILE(querys->query, querys->usrname, info ->fileName, info ->fileType, querys ->cwdNode, info ->md5sum);
    ReadyforQuery(querys, 1);
    return 0;
}

/* ls, cd, download, upload的第2个参数处理，保证最后一位是目录，normaPath可以为NULL，返回目录Node */
long forwardSRCH(queryTypePtr querys, const char * buf, int length, char * normPath)
{
    char str[128] = {0};
    long node = querys ->cwdNode;
    for (int i = 0, j = 0, k = 0; i < length;)
    {
        str[j++] = buf[i++];
        if(buf[i] == '/' || buf[i] == '\0')
        {
            ++i;
            j = 0;
            if(strcmp(str, ".") == 0)
            {
                bzero(str, 1);
                continue;
            }

            CWDSRCH(querys->query, node, str);
            if(strcmp(str, "..") == 0)
            {
                PWD(querys->query, node);
                if(normPath)
                {
                    while(normPath[k] != '/') normPath[k--] = '\0';
                    normPath[k] = '\0';
                }
            }
            ReadyforQuery(querys, STDIN_FILENO);
            if(querys->queryRNum != 1)
            {
                printf("No such file or Directory.\n");
                return -1;
            }

            while((querys->queryRow = mysql_fetch_row(querys->queryRes)) != NULL)
            {
                if(querys->queryCNum > 2 && querys->queryRow[3][0] != 'd')
                {
                    return -1;
                    printf("No such file or Directory.\n");
                }  
                node = atol(querys->queryRow[1]);
                if(normPath && querys->queryCNum > 2)
                {
                    normPath[k++] = '/';
                    strcat(&normPath[k], querys->queryRow[0]);
                    k += strlen(querys->queryRow[0]);
                }
            }
            mysql_free_result(querys->queryRes);
            while(buf[i] == '/') ++i;
            bzero(str, sizeof(str));
        }
    }
    return node;
}
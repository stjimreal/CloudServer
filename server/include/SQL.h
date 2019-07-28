#ifndef __SQL_H
#define __SQL_H
#define _GNU_SOURCE
#include "head.h"
#include <mysql/mysql.h>

#define QUERY_SIZE 4096
#define FILE_TABLE "fileList "

#define PRENODE "prenode "
#define NODE "node "
#define FILE_NAME "fileName "
#define MD5SUM "md5sum "
#define OWNER "owner "
#define FILE_TYPE "fileType "
#define PASSWD "passwd "
#define LAST_CHANGED "LastChanged "
#define CREATE_TIME "createTime "

#define FROM "FROM "
#define WHERE "WHERE "
#define INSERT_INTO "INSERT INTO "
#define VALUES "VALUES ("
#define SELECT "SELECT "
#define DELETE "DELETE "
#define UPDATE "UPDATE "
#define SET "SET "
/* 查找用户✔️ */
#define CHECKUSR(queryStr, usrname)  {sprintf(queryStr, "select passwd, node from fileList where passwd is not NULL and owner = '%s';", usrname);}
/* 修改密码✔️ */
#define CHANGEPASSWD(queryStr, passwd, usrname)  {sprintf(queryStr, "update fileList set passwd = '%s' where (prenode = 0 OR passwd is not NULL) AND owner = '%s';", passwd, usrname);}
/* 增加文件条目✔️ */
#define ADDFILE(queryStr, usrname, fileName, type, prenode, md5sum) {sprintf(queryStr, "insert into fileList(owner, fileName, fileType, prenode, md5sum) VALUES('%s', '%s', '%c', '%lu', '%s');", usrname, fileName, type, prenode, md5sum);}
#define RMFILE(queryStr, prenode, fileName) {sprintf(queryStr, "delete from fileList where prenode = %ld and fileName = '%s';", prenode, fileName);}

/* 查找文件名或文件按 md5sum 按用户，按当前目录 */
/* 下载文件，按当前目录和文件名查找文件md5sum；上传文件，按当前目录查找文件名，按md5sum查找md5sum；ls，按当前目录查找所有文件名 */

/* 全局MD5码检查，用于上传文件✔️ */
#define MD5FILT(queryStr, md5sum) {sprintf(queryStr, "select md5sum from fileList where md5sum = '%s';", md5sum);}

/* ls、重名检查、MD5SUM✔️ */
#define CWDSRCH(queryStr, node, fileName){if(strlen(fileName) == 0) sprintf(queryStr, "SELECT fileName, node, md5sum, fileType, LastChanged from fileList where prenode = %lu;", node); else sprintf(queryStr, "SELECT fileName, node, md5sum, fileType from fileList where prenode = %lu AND fileName = '%s';", node, fileName);}

/* 向前检索当前路径✔️ */
#define PWD(queryStr, node) {sprintf(queryStr, "SELECT fileName, node from fileList where node = (select prenode from fileList where node = %lu);",  node);}

/* 用于查询的结构体，包括姓名usrname，工作目录结点cwdNode，salt值，查询字符串query，查询传出数据结构 */
typedef struct 
{
    char query[512];
    char usrname[64];
    MYSQL_ROW queryRow;
    MYSQL *sql;
    MYSQL_RES * queryRes;
    long int cwdNode;
    int queryRNum;
    int queryCNum;
} queryType, *queryTypePtr;

typedef struct 
{
    char fileName[256];
    char md5sum[33];
    long node;
    char fileType;
}fileInfo, *fileInfoPtr;

#define ReadyforQuery(querys, chType)   \
{                                       \
printf("%s\n", querys -> query);        \
int err = mysql_query(querys -> sql, querys -> query);  \
SQL_ERROR_CHECK(err, -1, querys -> sql);      \
if(chType == 0) {                             \
    querys -> queryRes = mysql_store_result(querys -> sql); \
    querys -> queryRNum = mysql_num_rows(querys -> queryRes); \
    printf("%d rows in set.\n", querys -> queryRNum); \
    querys -> queryCNum = mysql_num_fields(querys -> queryRes);}}

/* 内建账号、密码、IP地址、数据库名等信息建立连接到数据库 */
int SQL_Con(MYSQL **sql_ptr);
/* 通过查询usrname进行登录检查，如有则传出有内容的passwd，无则passwd为NULL */
int LoginCheck(queryTypePtr querys, char ** passwd);
/* 传入passwd，按cwdNode修改passwd */
int changePasswd(queryTypePtr querys, const char * passwd);
/* 创建文件或文件夹 */
int createFile(queryTypePtr querys, fileInfoPtr info);
/* 根据querys -> cwdNode、buf（必须是目录）向前搜索路径，返回标准化的路径 */
long forwardSRCH(queryTypePtr querys, const char * buf, int length, char * normPath);
/* 向回搜索父结点路径 */
int prtWorkDir(int client_fd, queryTypePtr querys);
/* 传出创建的当前目录下的{fileName, node, md5sum, fileType}数组 */
int srchFile(long int node, const char* fileName, fileInfoPtr* fileInfoArrPointer);

#define servCnt 5
int getFile(int client_fd, queryTypePtr querys);
int listFile(int client_fd, queryTypePtr querys);
int putFile(int client_fd, queryTypePtr querys);
int chDir(int client_fd, queryTypePtr querys);
int rmFile(int client_fd, queryTypePtr querys);
int mvFile(int client_fd, queryTypePtr querys);
int chLoginDir(int client_fd, queryTypePtr querys);
int chPasswd(int client_fd, queryTypePtr querys);
#endif
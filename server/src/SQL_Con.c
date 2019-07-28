#include "../include/SQL.h"
/* 内建账号、密码、IP地址、数据库名等信息建立连接到数据库 */
int SQL_Con(struct MYSQL **conn)
{
	char* server="localhost";
	char* user=NULL;
	char* password="?.,/.,";
	char* database="JimCloudDisk";
    *conn=mysql_init(NULL);
	void * err = mysql_real_connect(*conn,server,user,password,database,0,NULL,0);
	SQL_ERROR_CHECK(err, NULL, *conn);
	printf("Connected..\n");
	return 0;
}
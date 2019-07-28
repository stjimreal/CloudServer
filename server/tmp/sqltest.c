/* 数据库实现一期功能：cd、ls、ll、mv、gets、puts */
/* 登录数据库的特性C语言实现编译后缀为-lmysqlclient */
#include "../include/SQL.h"
int sql_insert(struct SQL *sql, const char *name, const char *data)
{
    char buf[QUERY_SIZE];
    sprintf(buf,"%s%s%s%s%s", "INSERT INTO ", "fileList " , "(", name, ")");
    sprintf(buf, "%s%s%s%s%s", buf, "VALUES", "(",data, ")");
    int ret = mysql_query(sql, buf);
    SQL_ERROR_CHECK(ret, -1, sql);
    printf("Inserted %lu Rows\n", (unsigned long)mysql_affected_rows(sql));
    return 0;
}

int LoginCheck(MYSQL* sql, const char* name, char **salt)
{
    /* 检索查看是否重名 */
    char buf[QUERY_SIZE] = CHECKUSR(name);

    return 0;
}

int main(int argc, char * argv[])
{
    MYSQL_RES *res;
    MYSQL_ROW row;
    int err = SQL_Con(&sql);
    ERROR_CHECK(err, -1, "SQL_Con");
    ARGS_CHECK(argc, 3);
    char loginName[64] = argv[1];
    char loginPasswd[128] = argv[2];
    char *salt;
    err = LoginCheck(sql, loginName, &salt);
    ERROR_CHECK(err, -1, "createCounter");

}

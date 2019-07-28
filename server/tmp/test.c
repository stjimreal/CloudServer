#include "../include/SQL.h"




/* 属于子线程轮询服务工作区，包括的数据结构有socket描述符，sql，usrname，cwdNode（不包含密码） */
int main()
{
    queryTypePtr querys = (queryTypePtr)malloc(sizeof(queryType));
    strcpy(querys->usrname, "jimlau");//接收
    char salt[] = "$6$ve3df3aDS8$";
    char key[] = "?.,/.,";
    // char passwd[128] = "$6$df4trKDF32V$lvj34oi23nr09vV93df4K4V9G0CV";
    char * passwd;
    /* 用户登录一次，connect一次 */
    SQL_Con(&querys->sql);
    LoginCheck(querys, &passwd);
    if(passwd)
    {
        printf("passwd = %s\n", passwd);
    }
    // changePasswd(querys, crypt(key, salt));
    
    
    return 0;
}
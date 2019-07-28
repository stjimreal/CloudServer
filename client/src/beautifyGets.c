#include "../include/bashUI.h"
/* 客户端引导标准输入指令，输出加粗绿色昵称 name，加粗 color 色引导 dir */
int beautifyGets(const char * name, const char * dir, int color)
{
    if(name != NULL)
    {
        BLACKEN();
        SET_FRONT_COLOR(FRONT_GREEN);
        printf("%s", name);
        RESETFONT();
        printf(":");
    }
    BLACKEN();
    SET_FRONT_COLOR(color);
    printf("%s", dir);
    RESETFONT();
    printf("$ ");
    
    return 0;
}
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
char x[11];
int main()
{
    bzero(x, 11);
    printf("'%s';\n", x);
    return 0;
}
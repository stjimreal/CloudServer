#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define ARGS_NUM 2
int main()
{
    char line[256];
    char * wordArr[ARGS_NUM];
    int wordCnt = 0, i = 0;
    bzero(line, sizeof(line));
    bzero(wordArr, sizeof(char *) * ARGS_NUM);
    fgets(line, sizeof(line), stdin);
    while(line[i] != '\n'&& wordCnt < ARGS_NUM)
    {
        int j = 0;
        while(line[i] == ' ') ++i; 
        while(line[i] != '\n' && line[i] != ' ') {++j;++i;}
        if(j == 0)
            continue;
        wordArr[wordCnt] = (char *)malloc(j);
        strncpy(wordArr[wordCnt], &line[i - j], j);

        /* 可以发包了 */
        printf("%s ", wordArr[wordCnt]);
        ++wordCnt;
    }

    return 0;
}
    
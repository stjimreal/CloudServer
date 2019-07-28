#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <stdio.h>
#include <crypt.h>
/* 生成长度为 length 的 salt 值，以 type 为加密方式 */
int genSalt(char * salt, int length, int type)
{
    salt[0] = '$';
    // if(length > 16 || length < 2)
    //     length = 16;
    if(type != 5 && type != 6 && type != 1)
        type = 6;
    salt[1] = type + '0';
    salt[2] = '$';
    int i, flag;
    srand(time(NULL));
    for(i = 3; i < length + 3; ++i)
    {
        flag = rand()%3;
        switch (flag)
        {
        case 0:
            salt[i] = 'a'+rand()%26;
            break;
        case 1:
            salt[i] = 'A'+rand()%26;
        case 2:
            salt[i] = '0'+rand()%10;
        default:
            break;
        }
    }
    salt[i] = '$';
    salt[i + 1] = '\0';
    return 0;
}
// int encodePasswd(const char * passwd, int size, int)
// {
//     if(argc < 3)
//         return -1;
//     char salt[24];
//     genSalt(salt, 16, atoi(argv[2]));
//     printf("%s\n", salt);
//     long length = strlen(crypt(argv[1], salt));
//     char * sptr = crypt(argv[1], salt);
//     printf("%lu: %s\n", length, sptr);
//     return 0;
// }
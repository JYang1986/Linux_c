/*
*UNIX高级环境编程习题3.2
*Author: Yang
*Date: 2017.03.05
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define ERROR_SUCCESS 0
#define ERROR_FAILED -1
#define INVALID_FD -1
#define INVALID_VALUE -1

#define PRINTF(format, ...) \
    do {\
        printf("[<%s>@%s-%d]: " format,\
            __func__, __FILE__, __LINE__, ##__VA_ARGS__);\
    }while(0)

int Dup2Check(int iOldFd, int iNewFd)
{
    //Fd的值由调用者保证
    assert(iOldFd > 0);
    assert(iNewFd > 0);

    int iTempFd = INVALID_FD;
    //系统中进程可以打开的最大文件数
    int iTableSize = getdtablesize();
    int iFlag = INVALID_VALUE;

    if (iTableSize < iNewFd)
    {
        PRINTF("new fd(%d) out of system limit.\n", iNewFd);
        return ERROR_FAILED;
    }

    //不能用fcntl，就用dup检查iOldFd的有效性
    if ((iTempFd = dup(iOldFd)) == INVALID_FD)
    {
        PRINTF("dup old fd(%d) failed, (errno:%d) %s.\n", iOldFd, errno, strerror(errno));
        return ERROR_FAILED;
    }
    else
    {
        if (ERROR_FAILED == close(iTempFd))
        {
            PRINTF("close temp fd(%d) failed, (errno:%d) %s.\n", iTempFd, errno, strerror(errno));
            return ERROR_FAILED;
        }
    }

    //检查iNewfd的有效性，如果打则关闭
    if ((iTempFd = dup(iNewFd)) == INVALID_FD)
    {
        //iNewFd is not openned
        if (EBADF == errno)
        {
            return ERROR_SUCCESS;
        }
        else
        {
            PRINTF("dup new fd(%d) failed, (errno:%d) %s.\n", iNewFd, errno, strerror(errno));
            return ERROR_FAILED;
        }
    }
    else
    {
        if (ERROR_FAILED == close(iNewFd))
        {
            PRINTF("close new fd(%d) failed, (errno:%d) %s.\n", iNewFd, errno, strerror(errno));
            return ERROR_FAILED;
        }

        if (ERROR_FAILED == close(iTempFd))
        {
            PRINTF("close temp fd(%d) failed, (errno:%d) %s.\n", iTempFd, errno, strerror(errno));
            return ERROR_FAILED;
        }
    }

    return ERROR_SUCCESS;
}

int MyDup2(int iOldFd, int iNewFd)
{
    assert(iNewFd > 0);

    int i;
    int j;
    int iDupFd = INVALID_FD;
    int iIndexHit = INVALID_VALUE;
    int iFdArray[iNewFd];

    if (ERROR_SUCCESS != Dup2Check(iOldFd, iNewFd))
    {
        return INVALID_FD;
    }

    if (iOldFd == iNewFd)
    {
        PRINTF("MyDup2 success. old fd(%d), new fd(%d)\n", iOldFd, iNewFd);
        return iNewFd;
    }

    for (i = 0; i < iNewFd; i++)
    {
        iFdArray[i] = INVALID_FD;
    }

    for (i = 0; i < iNewFd; i++)
    {
        iDupFd = dup(iOldFd);
        if (iDupFd == INVALID_FD)
        {
            break;
        }
        iFdArray[i] = iDupFd;

        if (iDupFd == iNewFd)
        {
            iIndexHit = i;
            break;
        }
     }

    for(j = 0; j < i; j++)
    {
        (void)close(iFdArray[j]);
    }

    if (INVALID_VALUE != iIndexHit)
    {
         PRINTF("MyDup2 success. old fd(%d), new fd(%d)\n", iOldFd, iNewFd);
         return iNewFd;
    }
    else
    {
        PRINTF("MyDup2 failed. errno=%d\n", errno);
        return INVALID_FD;
    }
}

int main(int argc, char *argv[])
{
    int iOldFd = INVALID_FD;
    int iNewFd = 10;
    char *sFileName = "dup2test.txt";

    if ((iOldFd = open(sFileName, O_RDWR)) >0 )
    {
        MyDup2(iOldFd, iNewFd);
    }

    MyDup2(1, iNewFd);
    MyDup2(100, iNewFd);

    (void)close(iOldFd);
    (void)close(iNewFd);

    return ERROR_SUCCESS;
}

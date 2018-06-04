/**
 * @chinese
 * @file   usr_sem.c
 * @author 刘训
 * @date   Wed Jul 13 10:01:47 2011
 *
 * @brief 信号量模块接口
 * @endchinese
 *
 * @english
 * @file   usr_sem.c
 * @author Liu Xun
 * @date   Wed Jul 13 10:01:47 2011
 *
 * @brief semaphore module
 * @endenglish
 *
 *
 *
 */
#include <sys/sem.h>
#include <sys/ipc.h>
#include "usr_sem.h"
#include "config.h"
#include "debug.h"

#if 0
#define SEGSIZE 1024
#define READTIME 1
#endif


/**利用ftok函数实现 关键key*/
#define SEM_KEY ftok("/",12)
int finger_enroll = FREE ,uart3_transport = FREE; /**控制指纹/UART之间切换*/

 union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int uart3_semid = -1;

/**
 * @chinese
 * 生成信号量
 *
 * @return 成功-0,失败-(-1)
 * @endchinese
 *
 * @english
 * generate semaphore.
 *
 * @return Success-0,fail-(-1)

 * @endenglish
 *
 */
int usr_sem_creat()
{
    union semun sem;

    sem.val = 2;
    uart3_semid = semget(SEM_KEY,1,IPC_CREAT|0666);
    if (-1 == uart3_semid){
        plog("create semaphore error\n");
        return (-1);
    }
    semctl(uart3_semid,0,SETVAL,sem);
    return 0;
}

/**
 * @chinese
 * 删除信号量
 *
 * @return
 * @endchinese
 *
 * @english
 * delete semaphore.
 *
 * @return

 * @endenglish
 *
 */
void usr_sem_del()
{
    union semun arg;

    arg.val = 0;
    semctl(uart3_semid,0,IPC_RMID,arg);
}

/**
 * @chinese
 * 抢占
 *
 * @return 成功-0,失败-(-1)
 * @endchinese
 *
 * @english
 * seize
 *
 * @return Success-0,fail-(-1)

 * @endenglish
 *
 */
int usr_sem_wait()
{
    if(uart3_semid < 0 )
        return 0;
// struct sembuf sops={0,-1,IPC_NOWAIT};
    struct sembuf sops={0,-1,0};
    return (semop(uart3_semid,&sops,1));
}

/**
 * @chinese
 * 释放
 *
 * @return 成功-0,失败-(-1)
 * @endchinese
 *
 * @english
 * release
 *
 * @return Success-0,fail-(-1)

 * @endenglish
 *
 */
int usr_sem_post()
{
    if(uart3_semid < 0 )
        return 0;
//struct sembuf sops={0,+1,IPC_NOWAIT};
    struct sembuf sops={0,1,0};
    return (semop(uart3_semid,&sops,1));
}

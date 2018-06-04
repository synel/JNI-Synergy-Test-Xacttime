/**
 * @file   usr_sem.h
 * @author 刘训
 * @date   Wed Jul 13 10:02:07 2011
 *
 * @brief
 *
 *
 */
#ifndef USR_SEM_H
#define USR_SEM_H
#include<stdio.h>

typedef enum _MUTEX_T{
    FREE = 0,
    UART3,ENROLL
    }mutex_t;
extern int finger_enroll ,uart3_transport;

int usr_sem_creat();
void usr_sem_del();
int usr_sem_wait();
int usr_sem_post();
#endif

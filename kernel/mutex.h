//


#ifndef XV6_RISCV_SAMPLES_MUTEX_H
#define XV6_RISCV_SAMPLES_MUTEX_H

#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"

#define NMUTEX 256

struct mutex
{
    struct spinlock safety_lock;   // спинлок для защиты доступа к мьютексу
    struct sleeplock lock;         // слиплок для реализации блокировок с приостановкой
    uint access_number;            // счетчик доступа к мьютексу
    int holder_pid;                // идентификатор процесса, который захватил мьютекс
};


void mutinit(void);
void acquire_mutex(struct mutex *mutex);
void release_mutex(struct mutex *mutex);
int  holding_mutex(struct mutex *mutex);
void destroy_mutex(struct mutex *mutex);

struct mutex *mutex_dup(struct mutex *mutex);
struct mutex *mutex_alloc(void);




#endif //XV6_RISCV_SAMPLES_MUTEX_H
// Created by George Tsagol on 14.07.2024.
//
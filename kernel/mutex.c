//
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "proc.h"
#include "mutex.h"
#include "defs.h"


struct {
    struct spinlock lock;
    struct mutex mutex[NMUTEX];
} mutex_table;


void
mutinit(void)
{
    initlock(&mutex_table.lock, "mutex_table");
    for (int i = 0; i < NMUTEX; i++)
        initsleeplock(&mutex_table.mutex[i].lock, "mutex_i");
}

int
create_mutex(void)
{
    int i = 0,flag = 0;
    struct proc* p = myproc();
    acquire(&p->lock);
    acquire(&mutex_table.lock);
    for (i = 0; i < NMUTEX; i++)
    {
        if (mutex_table.mutex[i].count < 1)
        {
            mutex_table.mutex[i].count++;
            p->mutex_table[i] = mutex_table.mutex + i;
            flag = 1;
            break;
        }
    }
    if (!flag)
    {
        release(&mutex_table.lock);
        release(&p->lock);
        return -1;
    }
    release(&mutex_table.lock);
    release(&p->lock);
    return i;
}

int
lock_mutex(int i)
{
    if (i < 0 || i >= NMUTEX || mutex_table.mutex[i].count < 1) return -1;
    struct mutex* m = mutex_table.mutex + i;
    acquiresleep(&m->lock);
    return 0;
}

int
unlock_mutex(int i)
{
    if (i < 0 || i >= NMUTEX || mutex_table.mutex[i].count < 1) return -1;
    struct mutex* m = mutex_table.mutex + i;
    releasesleep(&m->lock);
    return 0;
}

int
destroy_mutex(int i)
{
    acquire(&mutex_table.lock);
    if (i < 0 || i >= NMUTEX || mutex_table.mutex[i].count < 1)
    {
        release(&mutex_table.lock);
        return -1;
    }
    struct mutex* m = mutex_table.mutex + i;
    if (m->lock.locked)
        releasesleep(&m->lock);
    m->count--;
    myproc()->mutex_table[i] = 0;
    release(&mutex_table.lock);
    return 0;
}

uint64
sys_create_mutex(void)
{
    return create_mutex();
}

uint64
sys_lock_mutex(void)
{
    int n;
    argint(0, &n);
    return lock_mutex(n);
}

uint64
sys_unlock_mutex(void)
{
    int n;
    argint(0, &n);
    return unlock_mutex(n);
}

uint64
sys_destroy_mutex(void)
{
    int n;
    argint(0, &n);
    return destroy_mutex(n);
}
// Created by George Tsagol on 06.05.2024.
//

//
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "stat.h"
#include "proc.h"

struct mutex mutex_table[NMUTEX];

void mutinit(void)
{
    struct mutex *temp;
    for (temp = mutex_table; temp < &mutex_table[NMUTEX]; temp++)
    {
        initlock(&temp->sp_lock, "spinlock");
        initsleeplock(&temp->sl_lock, "sleeplock");
        temp->lock=0;
        temp->pid=-1;
    }
}

int validate_mutex(int mut_desc)
{
    if (mut_desc < 0 || mut_desc >= NOMUTEX) return -1;
    return 0;
}

int lock_mutex(int mut_desc)
{
    if (validate_mutex(mut_desc) == -1) return -1;
    struct proc *process = myproc();
    acquire(&process->lock);
    struct mutex *mutex = process->table_mutex[mut_desc];
    if (!mutex || holdingsleep(&mutex->sl_lock)) return -1;
    uint process_id = process->pid;
    release(&process->lock);
    acquiresleep(&mutex->sl_lock);
    acquire(&mutex->sp_lock);
    mutex->pid = process_id;
    release(&mutex->sp_lock);
    return 0;
}

uint64 sys_acquire_mutex()
{
    int mut_desc; argint(0, &mut_desc);
    return lock_mutex(mut_desc);
}

int unlock_mutex(int mut_desc)
{
    if (validate_mutex(mut_desc) == -1) return -1;
    struct proc *process = myproc();
    acquire(&process->lock);
    struct mutex *mutex = process->table_mutex[mut_desc];
    if (!mutex || !holdingsleep(&mutex->sl_lock)) return -1;
    uint process_id = process->pid;
    release(&process->lock);
    acquire(&mutex->sp_lock);
    if (process_id != mutex->pid)
    {
        release(&mutex->sp_lock);
        return -1;
    }
    mutex->pid = -1;
    release(&mutex->sp_lock);
    releasesleep(&mutex->sl_lock);
    return 0;
}

uint64 sys_release_mutex()
{
    int mut_desc; argint(0, &mut_desc);
    return unlock_mutex(mut_desc);
}

int generate_mutex(void)
{
    struct proc* process = myproc();
    int index = -1;
    for (int i = 0; i < NMUTEX; ++i)
    {
        acquire(&mutex_table[i].sp_lock);
        if (mutex_table[i].lock)
        {
            release(&mutex_table[i].sp_lock);
            continue;
        }
        mutex_table[i].lock = 1;
        acquire(&process->lock);
        for (int j = 0; j < NOMUTEX; ++j)
        {
            if (!(process->table_mutex[j]))
            {
                process->table_mutex[j] = &mutex_table[i];
                index = j;
                break;
            }
        }
        release(&process->lock);
        release(&mutex_table[i].sp_lock);
        if (index != -1) return index;
    }
    return -1;
}

uint64 sys_create_mutex()
{
    int descriptor = generate_mutex();
    return descriptor;
}

// oсвобождение ресурсов мьютекса
int destroy_mutex(int mut_desc)
{
    if (validate_mutex(mut_desc) == -1) return -1;
    struct proc *process = myproc();
    acquire(&process->lock);
    struct mutex *mutex = process->table_mutex[mut_desc];
    release(&process->lock);
    if (!mutex) return -1;
    process->table_mutex[mut_desc] = 0;
    acquire(&mutex->sp_lock);
    if (!(mutex->lock))
    {
        release(&mutex->sp_lock);
        return -1;
    }
    mutex->lock--;
    release(&mutex->sp_lock);
    if (holdingsleep(&mutex->sl_lock)) releasesleep(&mutex->sl_lock);
    return 0;
}

uint64 sys_free_mutex()
{
    int mut_desc; argint(0, &mut_desc);
    return destroy_mutex(mut_desc);
}
// Created by George Tsagol on 06.05.2024.
//

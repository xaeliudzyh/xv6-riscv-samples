//
#include "mutex.h"
#include "riscv.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "proc.h"
struct mutex mutex_table[NMUTEX];

void mutinit()
{
    for (struct mutex *current_mutex = mutex_table; current_mutex < mutex_table + NMUTEX; ++current_mutex)
    {
        initlock(&current_mutex->safety_lock, "mutex safety lock"); // инициализируем спинлок для защиты мьютекса
        initsleeplock(&current_mutex->lock, "mutex sleep lock");     // инициализируем слиплок для функционала мьютекса
    }
}

struct mutex *mutex_alloc()
{
    struct mutex *mutex;
    for (mutex = mutex_table; mutex < mutex_table + NOMUTEX;++mutex)
    {
        acquire(&mutex->safety_lock);  // захватываем спинлок для проверки доступности мьютекса
        if (!mutex->access_number)  // если мьютекс не используется
        {
            ++mutex->access_number;  // увеличиваем счетчик доступа
            release(&mutex->safety_lock);  // освобождаем спинлок
            return mutex;  // возвращаем указатель на выделенный мьютекс
        }
        release(&mutex->safety_lock);  // освобождаем спинлок, если мьютекс занят
    }
    return 0;  // если все мьютексы заняты, возвращаем 0
}

struct mutex *mutex_dup(struct mutex *mutex)
{
    acquire(&mutex->safety_lock);  // захватываем спинлок для проверки состояния мьютекса
    if (mutex->access_number < 1)  // если счетчик доступа меньше 1, вызываем панику
        panic("mutex_dup");
    mutex->access_number++;  // увеличиваем счетчик доступа
    release(&mutex->safety_lock);  // освобождаем спинлок
    return mutex;  // возвращаем указатель на мьютекс
}

void acquire_mutex(struct mutex *mutex)
{
    acquiresleep(&mutex->lock);  // захватываем слиплок для мьютекса
    mutex->holder_pid = myproc()->pid;  // сохраняем PID процесса, захватившего мьютекс
}

void release_mutex(struct mutex *mutex)
{
    mutex->holder_pid = 0;  // обнуляем PID процесса, владевшего мьютексом
    releasesleep(&mutex->lock);  // освобождаем слиплок
}

int holding_mutex(struct mutex *mutex)
{
    return holdingsleep(&mutex->lock);  // проверяем, удерживает ли текущий процесс мьютекс
}

void destroy_mutex(struct mutex *mutex)
{
    acquire(&mutex->safety_lock);  // захватываем спинлок для изменения состояния мьютекса
    if (mutex->access_number-- < 1)  // уменьшаем счетчик доступа и проверяем его значение
        panic("destroy mutex");  // вызываем панику, если счетчик доступа < 1
    if (holding_mutex(mutex))  // если мьютекс удерживается текущим процессом то освобождаем мьютекс
        release_mutex(mutex);
    release(&mutex->safety_lock);  // освобождаем спинлок
}
// Created by George Tsagol on 14.07.2024.
//

//
#include "types.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "proc.h"

uint64 sys_create_mutex(void)
{
    struct proc *p = myproc();
    for (int md = 0; md < NOMUTEX; md++)
    {
        if (p->open_mutex[md] == 0)  // проверяем, есть ли свободное место для мьютекса в тек-м процессе
        {
            struct mutex *new_mutex = mutex_alloc();  // выделяем новый мьютекс из таблицы
            if (!new_mutex) return -1;  // если выделение не удалось - ошибка
            p->open_mutex[md] = new_mutex;  // сохраняем указатель на новый мьютекс в таблице процесса
            return md;  // возвращаем индекс мьютекса
        }
    }
    return -2;  // если нет свободного места - ошибка
}

uint64 sys_acquire_mutex(void)
{
    int md;
    argint(0, &md);  // получаем индекс мьютекса
    if (md >= NOMUTEX || md < 0) return -1;  // проверяем корректность
    struct mutex *mutex = myproc()->open_mutex[md];  // получаем указатель на мьютекс по индексу
    if (!mutex) return -2;  // если мьютекс не существует - ошибка
    if (holding_mutex(mutex)) return -3;  // если мьютекс уже захвачен - ошибка
    acquire_mutex(mutex);  // захватываем мьютекс
    return 0;
}

uint64 sys_release_mutex(void)
{
    int md;
    argint(0, &md);
    if (md >= NOMUTEX || md < 0) return -1;
    struct mutex *mutex = myproc()->open_mutex[md];
    if (!mutex) return -2;
    if (!holding_mutex(mutex)) return -3;
    release_mutex(mutex);  // освобождаем мьютекс
    return 0;
}

uint64 sys_destroy_mutex(void)
{
    int md;
    argint(0, &md);
    if (md >= NOMUTEX || md < 0) return -1;
    struct proc *p = myproc();  // получаем указатель на текущий процесс
    struct mutex *mutex = p->open_mutex[md];  // получаем указатель на мьютекс по индексу
    if (!mutex) return -2;
    destroy_mutex(mutex);  // уничтожаем мьютекс, уменьшая счетчик доступа
    p->open_mutex[md] = 0;  // очищаем запись о мьютексе в таблице процесса
    return 0;
}
// Created by George Tsagol on 15.07.2024.
//

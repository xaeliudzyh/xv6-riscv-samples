//
#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"
#include "fs.h"

// хранение состояния протокола логирования
struct
{
    struct spinlock lock; // блокировка для синхронизации доступа к полям
    int syscall_ticks; // кол-во тиков для логирования системных вызовов
    int interrupt_ticks; // кол-во тиков для логирования прерываний
    int switch_ticks; // кол-во тиков для логирования переключений контекста
    int exec_ticks; // кол-во тиков для логирования запуска приложений
} protocol;

// инициализация структуры протокола логирования
void protocol_init(void)
{
    initlock(&protocol.lock, "system protocol"); // инициализация блокировки
    protocol.syscall_ticks = 0;
    protocol.interrupt_ticks = 0;
    protocol.switch_ticks = 0;
    protocol.exec_ticks = 0;
}

int enable_protocol(int num, int ticks_)
{
    acquire(&protocol.lock);
    if (!num)
    {
        protocol.syscall_ticks = ticks + ticks_; // настройка логирования системных вызовов
    } else if (num == 1)
    {
        protocol.interrupt_ticks = ticks + ticks_; // настройка логирования прерываний
    } else if (num == 2)
    {
        protocol.switch_ticks = ticks + ticks_; // настройка логирования переключений контекста
    } else if (num == 3)
    {
        protocol.exec_ticks = ticks + ticks_; // настройка логирования запуска прилож
    } else
    {
        release(&protocol.lock);
        return -1;
    }
    release(&protocol.lock);
    return 0;
}

// отключение логирования определенного типа событий
int disable_protocol(int num) {
    acquire(&protocol.lock);
    if (num == 0)
    {
        protocol.syscall_ticks = 0; // отключение логирования системных вызовов
    } else if (num == 1)
    {
        protocol.interrupt_ticks = 0; // отключение логирования прерываний
    } else if (num == 2)
    {
        protocol.switch_ticks = 0; // отключение логирования переключний контекста
    } else if (num == 3)
    {
        protocol.exec_ticks = 0; // отключение логирования запуска приложений
    } else
    {
        release(&protocol.lock); // освобождение блокировки в случае ошибки
        return -1;
    }
    release(&protocol.lock);
    return 0;
}

// системный вызов для включения логирования
uint64 sys_enable_protocol(void)
{
    int num = 0;
    int ticks_ = 0;
    argint(0, &num); // получение номера типа событий
    argint(1, &ticks_); // получение количества тиков
    return enable_protocol(num, ticks_);
}

// системный вызов для отключения логирования
uint64 sys_disable_protocol(void)
{
    int num = 0;
    argint(0, &num); // получение номера типа событий
    return disable_protocol(num);
}

// функция для проверки, истекли ли тики логирования системных вызовов
int syscall_ticks(void)
{
    int status = protocol.syscall_ticks >= ticks;
    return status;
}

// функция для проверки, истекли ли тики логирования прерываний
int interrupt_ticks(void)
{
    int status = protocol.interrupt_ticks >= ticks;
    return status;
}

// функция для проверки, истекли ли тики логирования переключений контекста
int switch_ticks(void)
{
    int status = protocol.switch_ticks >= ticks;
    return status;
}

// истекли ли тики логирования запуска приложений
int exec_ticks(void)
{
    int status = protocol.exec_ticks >= ticks;
    return status;
}

// Created by George Tsagol on 22.06.2024.
//

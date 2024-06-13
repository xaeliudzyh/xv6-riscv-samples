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

// структура для хранения диагностического буфера
struct
{
    struct spinlock lock; // спин-замок для синхронизации доступа к буферу
    char* begin; // указатель на начало данных в буфере
    char* end; // указатель на конец данных в буфере
    char buf[MAXDBUF]; // сам буфер для хранения диагностических сообщений
} diagnostical_buf;

// массив символов для преобразования чисел в строки
static char digits[] = "0123456789abcdef";

void diagnostical_buf_init(void)
{
    initlock(&diagnostical_buf.lock, "diagnostical buf");
    diagnostical_buf.begin = diagnostical_buf.buf + MAXDBUF; // установка начального указателя на конец буфера
    diagnostical_buf.end = diagnostical_buf.buf; // установка конечного указателя на начало буфера
}

// функция записи символа в буфер
void write_in_buf(char c)
{
    // проверка, что буфер не полон
    if (diagnostical_buf.end != diagnostical_buf.begin)
    {
        *(diagnostical_buf.end) = c; // запись символа в конец буфера
        diagnostical_buf.end++; // сдвиг указателя на конец на байт
    }
    else // если буфер полон
    {
        *(diagnostical_buf.end) = c; // запись символа в конец буф
        diagnostical_buf.end++, diagnostical_buf.begin++; // сдвиг обоих указателей на байт вперед
    }

    // если указатель на начало достиг конца, то оборачиваем его в начало
    if (diagnostical_buf.begin == diagnostical_buf.buf + MAXDBUF) diagnostical_buf.begin = diagnostical_buf.buf;

    // наобороьт
    if (diagnostical_buf.end == diagnostical_buf.buf + MAXDBUF) diagnostical_buf.end = diagnostical_buf.buf;
}
void write_int(int a, int base, int sign)
{
    char buf[16]; // буфер для хранения строкового представления числа
    int i;
    uint x;

    // проверка знака и преобразование отриц числа в полож
    if(sign && (sign = a < 0)) x = -a;
    else x = a;

    i = 0;

    // преобразование числа в строку в нужнофй системе счисления
    do
    {
        buf[i++] = digits[x % base];
    } while((x /= base) != 0);

    // если число отриц, добавляем знак минуса
    if(sign != 0) buf[i++] = '-';

    // запись числа в буфер в обратном порядке
    while(--i >= 0) write_in_buf(buf[i]);
}

void write_ptr(uint64 a)
{
    int i;
    write_in_buf('0'); // добавляем '0' в начало для указания на 16чное число
    write_in_buf('x'); // добавляем 'x' для указания на 16чное число

    // преобразование указателя в строку
    for (i = 0; i < (sizeof(uint64) * 2); i++, a <<= 4)
        write_in_buf(digits[a >> (sizeof(uint64) * 8 - 4)]); // запись каждого символа в буфер
}

void pr_msg(char* fmt, ...)
{
    va_list ap; // для хранения списка аргументов
    int i, c;
    char *s;
    // проверяем, чтобы форматная строка не была нулевой
    if (!fmt) panic("null fmt");
    // захватываем спин-лок для безопасного доступа к буферу
    acquire(&diagnostical_buf.lock);

    // захватываем лок для доступа к переменной ticks
    acquire(&tickslock);

    // записываем открывающую квадратную скобку
    write_in_buf('[');

    // записываем текущее значение ticks в десятичном формате
    write_int(ticks, 10, 1);
    write_in_buf(']');
    write_in_buf(' ');
    // освобождаем ticks
    release(&tickslock);
    // инициализация списка аргументов
    va_start(ap, fmt);
    // проходим по форматной строке
    for(i = 0; (c = fmt[i] & 0xff) != 0; i++)
    {
        // если символ не '%', записываем его в буфер
        if(c != '%')
        {
            write_in_buf(c);
            continue;
        }

        // получаем следующий символ после '%'
        c = fmt[++i] & 0xff;
        if(!c) break; // конец строки -- выходим из цикла

        // обрабатываем спецификатор формата
        switch(c)
        {
            case 'd':
                // целое число в десятичном формате
                write_int(va_arg(ap, int), 10, 1);
                break;
            case 'x':
                // целое число в 16чном формате
                write_int(va_arg(ap, int), 16, 1);
                break;
            case 'p':
                // указатель
                write_ptr(va_arg(ap, uint64));
                break;
            case 's':
                // строка
                if(!(s = va_arg(ap, char*)))
                s = "(null)"; // если строка нулевая
                for(; *s; s++)
                    write_in_buf(*s); // записываем каждый символ строки
                break;
            case '%':
                // символ '%'
                write_in_buf('%');
                break;
            default:
                write_in_buf('%');
                write_in_buf(c);
                break;
        }
    }
    // завершаем работу со списком аргументов
    va_end(ap);
    // добавляем символ новой строки в конец
    write_in_buf('\n');
    release(&diagnostical_buf.lock);
}


int dmesg(char* buff)
{
    // чекаем что указатель на буфер не нулевой
    if (!buff) return -1;
    acquire(&diagnostical_buf.lock);
    // начинаем с символа, на который указывает begin
    char* sym = diagnostical_buf.begin;
    int i = 0;
    // копирование данных из диаг буфера в юзер буфер
    do
    {
        // если указатель дошел до конца буфера - брейкаем цикл
        if (sym == diagnostical_buf.buf + MAXDBUF) break;
        // копируем байт из диагностического буфера в пользовательский буфер
        if (copyout(myproc()->pagetable, (uint64)(buff + i), sym, 1) < 0)
        {
            // если копирование не удалось, освобождаем лок и возвращаем ошибку
            release(&diagnostical_buf.lock);
            return -1;
        }
        i++;
        sym++;
        // если указатель дошел до конца буфера, оборачиваем его в начало
        if (sym == diagnostical_buf.buf + MAXDBUF) sym = diagnostical_buf.buf;

    } while (sym != diagnostical_buf.end); // продолжаем до тех пор, пока не дойдем до конца данных
    // добавляем нуль-терминатор в конец строки
    char* end = "\0";
    if (copyout(myproc()->pagetable, (uint64)(buff + i), end, 1) < 0)
    {
        // если копирование не удалось, освобождаем лок и возвращаем ошибку
        release(&diagnostical_buf.lock);
        return -1;
    }
    release(&diagnostical_buf.lock);

    return 0;
}

uint64 sys_dmesg(void)
{
    uint64 user_buf;

    // получаем аргумент системного вызова - указатель на буфер в пространстве юзера
    argaddr(0, &user_buf);

    // вызываем dmesg, передавая ей юзер буфер
    return dmesg((char*)user_buf);
}

// Created by George Tsagol on 13.06.2024.
//

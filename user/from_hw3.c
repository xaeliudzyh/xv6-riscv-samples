//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int n = 0;
    // проверяем, что передан хотя бы один параметр командной строки
    if (argc < 2)
    {
        printf("not enough params...\n");
        exit(1);
    }
    int p1[2], p2[2]; // массивы для 2 каналов
    // создаем первый канал
    n = pipe(p1);
    if (n < 0)
    {
        printf("failed to create a pipe...\n");
        exit(2);
    }
    // создаем второй канал
    n = pipe(p2);
    if (n < 0)
    {
        printf("failed to create a pipe...\n");
        exit(2);
    }
    int pid = 0;
    // создаем дочерний процесс
    pid = fork();
    if (pid < 0)
    {
        printf("failed to create a child process...\n");
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);
        exit(2);
    }
    if (!pid)
    {
        // для дочернего процесса
        close(p1[1]); // закрываем неиспользуемую половину канала
        close(p2[0]); // закрываем неиспользуемую половину канала
        char buf[1]; // буфер для чтения

        while(1) {
            n = read(p1[0], buf, 1); // читаем из первого канала
            if (n < 0)
            {
                printf("can't read from the pipe...\n");
                close(p1[0]);
                close(p2[1]);
                exit(2);
            }
            if (!n) break; // выход из цикла, если нет данных для чтения
            // выводим полученный символ
            printf("%d: received %c\n", getpid(), buf[0]);
            // отправляем символ обратно через второй канал
            n = write(p2[1], buf, 1);
            if (n < 0)
            {
                printf("can't write to the pipe...\n");
                close(p1[0]);
                close(p2[1]);
                exit(2);
            }
        }
        close(p1[0]); close(p2[1]);
        exit(0);
    }
    if (pid > 0)
    {
        // для родительского процесса
        close(p1[0]); close(p2[1]); // закрываем неиспользуемую половину канала
        n = write(p1[1], argv[1], strlen(argv[1])); // отправляем строку в первый канал
        if (n < 0)
        {
            printf("can't write to the pipe...\n");
            close(p2[0]);
            close(p1[1]);
            exit(2);
        }
        close(p1[1]); // закрываем первый канал после записи
        char buf[1]; // буфер для чтения
        while(1)
        {
            n = read(p2[0], buf, 1); // читаем из второго канала
            if (n < 0)
            {
                printf("can't read from the pipe...\n");
                close(p2[0]);
                exit(2);
            }
            if (!n) break; // выход из цикла, если нет данных для чтения
            // выводим полученный символ
            printf("%d: received %c\n", getpid(), buf[0]);
        }
        close(p2[0]);
        exit(0);
    }
    exit(0);
}
// Created by George Tsagol on 23.06.2024.
//

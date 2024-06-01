//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // проверка наличия аргументов
    if (argc < 2)
    {
        printf("Недостаточно параметров...\n");
        exit(1);
    }
    int p1[2], p2[2], n = 0;
    n = pipe(p1);
    // проверка на успешное создание первого pipe
    if (n < 0)
    {
        printf("Не удалось создать pipe...\n");
        exit(2);
    }
    n = pipe(p2);
    // проверка на успешное создание второго pipe
    if (n < 0)
    {
        printf("Не удалось создать pipe...\n");
        exit(2);
    }
    int mid = 0;
    mid = create_mutex();
    // проверка на успешное создание мьютекса
    if (mid < 0)
    {
        printf("Не удалось создать мьютекс...\n");
        exit(2);
    }
    int pid = 0;
    pid = fork();
    // проверка на успешное создание дочернего процесса
    if (pid < 0)
    {
        printf("Не удалось создать дочерний процесс...\n");
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);
        exit(2);
    }
    // код дочернего процесса
    if (!pid)
    {
        close(p1[1]);
        close(p2[0]);
        char buf[1];
        // бесконечный цикл чтения и записи данных
        while(1)
        {
            n = read(p1[0], buf, 1);
            // проверка на успешное чтение из pipe
            if (n < 0)
            {
                printf("Не удалось прочитать из pipe...\n");
                close(p1[0]);
                close(p2[1]);
                exit(2);
            }
            if (!n) break;
            n = lock_mutex(mid);
            // проверка на успешную блокировку мьютекса
            if (n < 0)
            {
                printf("Не удалось заблокировать мьютекс...\n");
                close(p1[0]);
                close(p2[1]);
                exit(2);
            }
            printf("%d: received %s\n", getpid(), buf); // слово "получено" почему-то плохо отображается
            n = unlock_mutex(mid);
            // проверка на успешное разблокирование мьютекса
            if (n < 0)
            {
                printf("Не удалось разблокировать мьютекс...\n");
                close(p1[0]);
                close(p2[1]);
                exit(2);
            }
            n = write(p2[1], buf, 1);
            // проверка на успешную запись в pipe
            if (n < 0)
            {
                printf("Не удалось записать в pipe...\n");
                close(p1[0]);
                close(p2[1]);
                exit(2);
            }
        }
        close(p1[0]);
        close(p2[1]);
        exit(0);
    }
    // код родительского процесса
    if (pid > 0)
    {
        close(p1[0]);
        close(p2[1]);
        n = write(p1[1], argv[1], strlen(argv[1]));
        // проверка на успешную запись в pipe
        if (n < 0)
        {
            printf("Не удалось записать в pipe...\n");
            close(p2[0]);
            close(p1[1]);
            exit(2);
        }
        close(p1[1]);
        char buf[1];
        // бесконечный цикл чтения данных
        while(1)
        {
            n = read(p2[0], buf, 1);
            // проверка на успешное чтение из pipe
            if (n < 0)
            {
                printf("Не удалось прочитать из pipe...\n");
                close(p2[0]);
                exit(2);
            }
            if (!n) break;
            n = lock_mutex(mid);
            // проверка на успешную блокировку мьютекса
            if (n < 0)
            {
                printf("Не удалось заблокировать мьютекс...\n");
                close(p2[0]);
                exit(2);
            }
            printf("%d: получено %s\n", getpid(), buf);
            n = unlock_mutex(mid);
            // проверка на успешное разблокирование мьютекса
            if (n < 0)
            {
                printf("Не удалось разблокировать мьютекс...\n");
                close(p2[0]);
                exit(2);
            }
        }
        close(p2[0]);
        wait(&n);
        n = destroy_mutex(mid);
        // проверка на успешное уничтожение мьютекса
        if (n < 0)
        {
            printf("Не удалось уничтожить мьютекс...\n");
            exit(2);
        }
        exit(0);
    }
    exit(0);
}

// Created by George Tsagol on 01.06.2024.
//

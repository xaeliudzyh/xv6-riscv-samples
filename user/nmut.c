// Created by George Tsagol on 01.06.2024.

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
            printf("%d: получено %s\n", getpid(), buf);
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
            printf("%d: получено %s\n", getpid(), buf);
        }

        close(p2[0]);
        exit(0);
    }

    exit(0);
}
//

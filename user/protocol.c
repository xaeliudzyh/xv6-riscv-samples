//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        int num = -1, ticks_num = -1;
        // первый аргумент командной строки конвертируем в число
        num = atoi(argv[1]);
        // второй аргумент командной строки конвертируем в число
        ticks_num = atoi(argv[2]);
        if (ticks_num <= 0) // значит нужно отключить протокол
        {
            int status = disable_protocol(num);
            if (status < 0)
            {
                // если ошибка, выводим  ошибочку
                printf("wrong arguments...\n");
                exit(1);
            }
        }
        else
        {
            // если кол-во тиков больше нуля, включаем протокол на заданное колво тиков
            int status = enable_protocol(num, ticks_num);
            if (status < 0)
            {
                // если возникла ошибка, выводим сообщение и завершаем программу с кодом ошибки
                printf("wrong arguments...\n");
                exit(1);
            }
        }
    }
    else
    {
        // если количество аргументов неправильное, выводим ошибкку
        printf("wrong number of arguments...\n");
        exit(1);
    }
    exit(0);
}
// Created by George Tsagol on 23.06.2024.
//

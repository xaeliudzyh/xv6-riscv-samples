//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int n_buf = 1, n = 0;
    char *c = 0;
    do {
        if (c) free(c); //  если буфер уже был выделен - освобождаем его
        c = (char*) malloc(n_buf * sizeof(char)); // выделяем новый буфер увеличенного размера
        n = dmesg(c); // вызываем dmesg для копирования данных в буфер
        n_buf *= 2; // удваиваем размер буфера для следующей итерации
    } while (n == -1); // повторяем, пока dmesg не вернет ненулевое значение

    printf("%s\n", c); // выводим с
    free(c);
    exit(0);
}

// Created by George Tsagol on 13.06.2024.
//

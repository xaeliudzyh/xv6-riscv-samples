//
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];
    int length = 0;
    int read_result;

    while (length < BUFFER_SIZE - 1) {
        read_result = read(0, buffer + length, 1);

        if (read_result < 0) {
            fprintf(2, "Ошибка чтения\n");
            return 1;
        }

        if (read_result == 0) {
            if (length == 0) {
                fprintf(2, "Пустая строка\n");
                return 1;
            }
            break;
        }

        if (buffer[length] == '\n') {
            break;
        }

        length++;
    }

    if (length >= BUFFER_SIZE - 1) {
        fprintf(2, "Превышен размер буфера\n");
        return 1;
    }

    buffer[length] = '\0';

    if (strlen(buffer) == 0) {
        fprintf(2, "Пустая строка\n");
        return 1;
    }
    char* space_pos = strchr(buffer, ' ');
    if (!space_pos) {
        fprintf(2, "Некорректный формат данных: отсутствует пробел\n");
        return 1;
    }

    int num1 = atoi(buffer);
    int num2 = atoi(space_pos + 1);

    printf("%d\n", num1 + num2);

    return 0;
}

// Created by George Tsagol on 06.03.2024.
//

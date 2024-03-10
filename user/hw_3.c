//
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int fd[2];
    if (pipe(fd) < 0) {
        fprintf(2, "Pipe creation failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        // Ошибка создания процесса
        fprintf(2, "Fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // Дочерний процесс
        close(0);       // Закрываем стандартный ввод
        if (dup(fd[0]) < 0) {
            fprintf(2, "Dup failed\n");
            exit(1);
        }
        close(fd[0]);   // Закрываем дублированный конец канала
        close(fd[1]);   // Закрываем конец канала для записи

        char *wc_args[] = {"wc", 0}; // Аргументы для вызова wc
        if (exec("wc", wc_args) < 0) {
            fprintf(2, "Exec failed\n");
            exit(1);
        }
    } else {
        // Родительский процесс
        close(fd[0]);   // Закрываем конец канала для чтения
        for (int i = 1; i < argc; i++) {
            write(fd[1], argv[i], strlen(argv[i]));
            write(fd[1], "\n", 1);  // Добавляем символ перевода строки после каждого аргумента
        }
        close(fd[1]);   // Закрываем конец канала для записи после передачи данных

        if (wait(0) < 0) {
            fprintf(2, "Wait failed\n");
            exit(1);
        }
        exit(0);
    }
    return 0;
}

// Created by George Tsagol on 11.03.2024.
//

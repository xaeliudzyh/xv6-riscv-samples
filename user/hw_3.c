//
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    if (argc < 2){
        fprintf(2, "Error: incorrect count of argument\n");
        exit(-1);
    }
    int fd[2];
    if (pipe(fd) < 0) {
        fprintf(2, "Pipe creation failed\n");
        exit(1);
    }

    int pid = fork(),st;
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
        close(fd[0]);
        int size_;
        for (int i = 1; i < argc; ++i){
            size_ = strlen(argv[i]);
            if (write(fd[1], argv[i], size_) == -1){
                fprintf(2, "ERROR: could not write to pipe\n");
                exit(-1);
            }
            if (write(fd[1], "\n", 1) == -1){
                fprintf(2, "ERROR: could not write to pipe\n");
                exit(-1);
            }
        }
        close(fd[1]);
        wait(&st);
    }
    return 0;
}

// Created by George Tsagol on 11.03.2024.
//

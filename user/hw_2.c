//
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"


int main(int argc, char *argv[]) {
    if (argc < 2){
        fprintf(2, "Error: incorrect count of argument\n");
        exit(-1);
    }
    int fd[2],st;
    // Создание канала
    if (pipe(fd) == -1) {
        fprintf(2, "Error: pipe process failed\n");
        return 1;
    }

    int pid = fork(); // Создание дочернего процесса
    if (pid < 0) {
        fprintf(2, "Error: child process failed\n");
        return 1;
    }

    if (pid > 0) {  // Родительский процесс
        close(fd[0]); // Закрытие конца канала для чтения
        // Запись аргументов командной строки в канал
        int size_;
        for (int i = 1; i < argc; i++) {
            size_ = strlen(argv[i]);
            if (write(fd[1], argv[i], size_) == -1){
                fprintf(2, "Error: cannot write to pipe\n");
                exit(-1);
            }
            if (write(fd[1], "\n", 1) == -1){
                fprintf(2, "Error: cannot write to pipe\n");
                exit(-1);
            }
        }
        close(fd[1]); // Закрытие конца канала для записи после отправки всех данных
        wait(&st);      // Ожидание завершения дочернего процесса
    } else {  // Дочерний процесс
        close(fd[1]); // Закрытие конца канала для записи
        char buffer[1024];
        int count;
        // Чтение данных из канала и их вывод
        while ((count = read(fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            printf("%s", buffer);
        }
        close(fd[0]); // Закрытие конца канала для чтения после чтения всех данных
        exit(0); // Завершение дочернего процесса
    }

    return 0;
}
// Created by George Tsagol on 10.03.2024.
//

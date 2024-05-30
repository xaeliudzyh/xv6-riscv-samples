//
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

int main(int argc, char** argv) {
    int child = fork();
    if(child<0)
    {
        fprintf(2,"Error: child process failed");
        return 1;
    }
    else if(child==0)
    {
        sleep(15);
        exit(1);
    }
    else {
        // Родительский процесс
        printf("Parent PID: %d, Child PID: %d\n", getpid(), child); kill(child);
        int status;  // Ожидание дочернего процесса
        wait(&status);
        printf("Child %d terminated with return status %d\n", child, status);
        return 0;
    }
}
// Created by George Tsagol on 10.03.2024.
//

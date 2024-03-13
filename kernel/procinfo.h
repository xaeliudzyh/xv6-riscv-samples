//
// Created by George Tsagol on 12.03.2024.
#ifndef XV6_RISCV_SAMPLES_PROCINFO_H
#define XV6_RISCV_SAMPLES_PROCINFO_H
#define PROC_NAME_MAX 16
struct procinfo {
    char name[PROC_NAME_MAX];  // Имя процесса
    int state;                 // Состояние процесса
    int parent_pid;            // PID родительского процесса
};

#endif //XV6_RISCV_SAMPLES_PROCINFO_H
//
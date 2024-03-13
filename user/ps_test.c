//
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/procinfo.h"
#define NPROC 64
void testTotalProcesses() {
    // Получаем общее количество процессов, передавая NULL в качестве plist
    int total_procs = ps_listinfo((void*)0, 0);
    printf("Total processes: %d\n", total_procs);
}

void testInsufficientBufferSize() {
    // Пытаемся получить информацию о процессах в слишком маленький буфер
    struct procinfo plist[2]; // Предполагая, что в системе больше двух процессов
    int res = ps_listinfo(plist, 2);
    if (res > 2 || res < 0) {
        printf("Error: Buffer too small, need at least %d slots\n", res);
    } else {
        printf("Insufficient buffer size test passed\n");
    }
}

void testInvalidAddress() {
    // Пытаемся использовать невалидный адрес
    int res = ps_listinfo((void*)-1, 10);
    if (res < 0) {
        printf("Invalid address test passed\n");
    } else {
        printf("Error: Test with invalid address should have failed\n");
    }
}

void testSuccess() {
    // Пытаемся получить информацию о всех процессах, увеличивая размер буфера при необходимости
    struct procinfo *plist = malloc(sizeof(struct procinfo) * NPROC);
    if (!plist) {
        printf("Memory allocation failed\n");
        return;
    }

    int res = ps_listinfo(plist, NPROC);
    if (res >= 0) {
        printf("Success test passed: Fetched info for %d processes\n", res);
    } else {
        printf("Error: Test failed with code %d\n", res);
    }

    free(plist);
}

int main(void) {
    testTotalProcesses();
    testInsufficientBufferSize();
    testInvalidAddress();
    testSuccess();
    exit(0);
}

// Created by George Tsagol on 13.03.2024.
//

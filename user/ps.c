//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/procinfo.h"
#define NPROC 64
const char* state_to_string(int state) {
    static const char* states[] = { "UNUSED", "USED", "SLEEPING", "RUNNABLE", "RUNNING ", "ZOMBIE" };
    if (state < 0 || state >= sizeof(states) / sizeof(const char*)) {
        return "UNKNOWN";
    }
    return states[state];
}

int main(void) {
    struct procinfo procinfos[NPROC];
    int count = ps_listinfo(procinfos, NPROC);

    if (count < 0) {
        fprintf(2, "ps: error retrieving process information\n");
        exit(1);
    }

    printf("STATE\t\tNAME\t\tPARENT_PID\n");

    for (int i = 0; i < count; i++) {
        printf("%s\t%s\t\t%d\n",
               state_to_string(procinfos[i].state),
               procinfos[i].name,
               procinfos[i].parent_pid);
    }

    exit(0);
}

// Created by George Tsagol on 13.03.2024.
//

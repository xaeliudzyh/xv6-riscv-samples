//
#ifndef MUTEX_H
#define MUTEX_H

#include "spinlock.h"
#include "sleeplock.h"

struct mutex {
    int count;
    struct sleeplock lock;
};

struct {
    struct spinlock lock;
    struct mutex mutex[NMUTEX];
} mutex_table;

#endif
// Created by George Tsagol on 06.05.2024.
//

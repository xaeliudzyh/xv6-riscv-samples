//
#ifndef MUTEX_H
#define MUTEX_H

#include "spinlock.h"
#include "sleeplock.h"

struct mutex {
    int count;
    struct sleeplock lock;
};

#endif
// Created by George Tsagol on 06.05.2024.
//

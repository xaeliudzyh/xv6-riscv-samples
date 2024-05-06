//
#include "spinlock.h"
#include "sleeplock.h"

struct mutex {
    uint64 lock;             // is the lock held?
    struct sleeplock sl_lock;
    struct spinlock sp_lock;    // spinlock protecting this sleep lock
    uint64 pid;            // num of descriptors that refer to this mutex
};
// Created by George Tsagol on 06.05.2024.
//

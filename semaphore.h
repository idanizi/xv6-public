// changed #task2.2
// Created by idan on 9/20/16.

#ifndef XV6_PUBLIC_SEMAPHORE_H
#define XV6_PUBLIC_SEMAPHORE_H

#include "user.h"

struct semaphore {
    int s1;
    int s2;
    int value;
    int wake;
};

void semaphore_init(struct semaphore *sm, int value) {
    sm->s1 = kthread_mutex_alloc();
    sm->s2 = kthread_mutex_alloc();
    sm->value = value;
    sm->wake = 0;
}

int semaphore_delete(struct semaphore *sm) {
    kthread_mutex_unlock(sm->s1);
    kthread_mutex_unlock(sm->s2);
    if (kthread_mutex_dealloc(sm->s1) < 0) {
        printf(1,"semaphore: semaphore_delete sm->s1\n"); // todo del
        return -1;
    }
    if (kthread_mutex_dealloc(sm->s2) < 0) {
        printf(1,"semaphore: semaphore_delete sm->s2\n"); // todo del
        return -1;
    }
    return 0;
}

void semaphore_down(struct semaphore *sm) {
    kthread_mutex_lock(sm->s1);
//    printf(1, "tid=%d: semaphore_down\n", kthread_id()); // todo del
    sm->value--;
    if (sm->value < 0) {
        kthread_mutex_unlock(sm->s1);
        kthread_mutex_lock(sm->s2);
        kthread_mutex_lock(sm->s1);
        sm->wake--;
        if (sm->wake > 0) {
            kthread_mutex_unlock(sm->s2);
        }
    }
    kthread_mutex_unlock(sm->s1);
}

void semaphore_up(struct semaphore *sm) {
    kthread_mutex_lock(sm->s1);
//    printf(1, "tid=%d: semaphore_up\n", kthread_id()); // todo del
    sm->value++;
    if (sm->value <= 0) {
        sm->wake++;
        if (sm->wake == 1) {
            kthread_mutex_unlock(sm->s2);
        }
    }
    kthread_mutex_unlock(sm->s1);
}

#endif //XV6_PUBLIC_SEMAPHORE_H

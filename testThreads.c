// changed: user space program created for testing the kernel level threads #task1.2
// Created by idan on 9/14/16.
#include "types.h"
#include "stat.h"
#include "user.h"
#include "syscall.h"
#include "param.h"

// CONSTANTS
#define NTHREADS 20
#define USTACK 4000

void run() {
    printf(1, "in user space run\n");
    int id = kthread_id();
    int pid = getpid();
    printf(1, "tid=%d going to sleep 500\n", id);
    sleep(500);
    printf(1, "thread id: %d\n", id);
    printf(1, "process pid: %d\n", pid);
    printf(1, "tid=%d going to sleep 500 (#2)\n", id);
    sleep(500);
    printf(1, "tid=%d: hey! I'm out.\n", id);
    kthread_exit();
}

int main(int argc, char *argv[]) {
    int tidArray[NTHREADS];
    void *stack[NTHREADS];
    int pid = getpid();
    int i;

    // init arrays
    for (i = 0; i < NTHREADS; i++) {
        tidArray[i] = 0;
        stack[i] = 0;
    }

    for (i = 0; i < NTHREADS; i++) {
        printf(1, "testThreads: main: pid=%d: creating thread, start_func: %d\n", pid, (void *) &run);
        stack[i] = (void *) malloc(USTACK);
        tidArray[i] = kthread_create((void *) &run, stack[i], USTACK);
    }
    int rest;
    for (i = 0; i < NTHREADS; i++) {
        printf(1, "testThreads: main: pid=%d: join thread\n", pid);
        rest = kthread_join(tidArray[i]);
        printf(1, "testThreads: main: pid=%d: after join\n");
        printf(1, "testThreads: main: pid=%d: result: %d, tid: %d\n", pid, rest, tidArray[i]);
        // free memory
        if (stack[i]) {
            free(stack[i]);
        }
    }


    exit();
}

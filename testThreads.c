// changed: user space program created for testing the kernel level threads #task1.2
// Created by idan on 9/14/16.
#include "types.h"
#include "stat.h"
#include "user.h"

void run() {
    int id = kthread_id();
    int pid = getpid();
    int time = uptime();
    printf(1, "thread id: %d\n", id);
    printf(1, "process pid: %d\n", pid);
    while(uptime() - time < 1000);
    printf(1, "hey\n");
    kthread_exit();
}

int main(int argc, char *argv[]){
    void *stack = (void *) malloc(4000);
    void *(*start_func)();
    start_func = (void *) &run;
    int pid = getpid();
    printf(1, "pid of current process: %d\n", pid);
    printf(1, "creating thread\n");
    int tid = kthread_create(start_func, stack, 4000);
    printf(1, "join thread\n");
    int rest = kthread_join(tid);
    printf(1, "after join");
    printf(1, "result: %d, tid: %d\n", rest, tid);
    exit();
}

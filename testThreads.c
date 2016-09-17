// changed: user space program created for testing the kernel level threads #task1.2
// Created by idan on 9/14/16.
#include "types.h"
#include "stat.h"
#include "user.h"
#include "syscall.h"
#include "param.h"

//void run2(){
//
//}

void run() {
    printf(1, "in user space run\n");
    int id = kthread_id();
    int pid = getpid();
    int time = uptime();
    printf(1, "tid=%d going to busy wait1\n");
    while(uptime() - time < 500);
    printf(1, "thread id: %d\n", id);
    printf(1, "process pid: %d\n", pid);
    printf(1, "tid=%d going to busy wait2\n");
    while(uptime() - time < 500);
    printf(1, "hey\n");
    kthread_exit();
//    return (void *) 1;
}

int main(int argc, char *argv[]) {
    int stack_size = 4000;
    void *stack = (void *) malloc(stack_size);
    void *(*start_func)();
    start_func = (void *) &run;
    int pid = getpid();
    printf(1, "testTreads: main: pid=%d: creating thread, start_func: %d\n", pid, start_func);
    int tid = kthread_create(start_func, stack, stack_size);
    printf(1, "testTreads: main: pid=%d: join thread\n", pid);
//    printf(1, "testTreads: main: pid=%d: sleep 1000 ticks\n", pid);
//    int time = uptime();
//    int count = 0;
//    while (uptime() - time < 1000){
//        count++;
//    }
//    printf(1, "testTreads: main: pid=%d: finish sleep %d", pid, count);
    int rest = kthread_join(tid);
    printf(1, "testTreads: main: pid=%d: after join\n");
    printf(1, "testTreads: main: pid=%d: result: %d, tid: %d\n", pid, rest, tid);
    exit();
}

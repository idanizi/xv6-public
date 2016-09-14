// changed: user space program created for testing the kernel level threads #task1.2
// Created by idan on 9/14/16.
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]){
    // TODO: write a user program to test all the system calls stated above
    /*
     * int kthread_create( void*(*start_func)(), void* stack, int stack_size);
     * int kthread_id();
     * void kthread_exit();
     * int kthread_join(int thread_id);
     */
    // todo: malloc for *stack at the user space program, pass the pointer by sysprpc.c to the system call in the kernel
    exit();
}

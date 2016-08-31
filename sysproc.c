#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
        return fork();
}

int
sys_exit(void)
{
        // changed: calling exit function with status parameter #task1
        int status;
        if(argint(0, &status) < 0)
                return -1;
        exit(status);
        // changed #end
        return 0; // not reached
}

int
sys_wait(void)
{
        // changed #task1
        int* status;
        if(argptr(0, (char**) &status, sizeof(int*)) < 0)
                return -1;
        // changed #end
        return wait(status);
}

int
sys_kill(void)
{
        int pid;

        if(argint(0, &pid) < 0)
                return -1;
        return kill(pid);
}

int
sys_getpid(void)
{
        return proc->pid;
}

int
sys_sbrk(void)
{
        int addr;
        int n;

        if(argint(0, &n) < 0)
                return -1;
        addr = proc->sz;
        if(growproc(n) < 0)
                return -1;
        return addr;
}

int
sys_sleep(void)
{
        int n;
        uint ticks0;

        if(argint(0, &n) < 0)
                return -1;
        acquire(&tickslock);
        ticks0 = ticks;
        while(ticks - ticks0 < n) {
                if(proc->killed) {
                        release(&tickslock);
                        return -1;
                }
                sleep(&ticks, &tickslock);
        }
        release(&tickslock);
        return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
        uint xticks;

        acquire(&tickslock);
        xticks = ticks;
        release(&tickslock);
        return xticks;
}

// changed: adding system calls that need to be handled by the kernel #task2.1

// system call to change process priority
int sys_priority(void){

    // collect arguments
    int priorityNumber;
    if(argint(0, &priorityNumber) < 0)
        return -1;

    // call kernel space priority function, and return its value to the caller
    return priority(priorityNumber);
}

// changed #end

#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
// changed #task2.2
#include "perf.h"
// changed #end

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

// changed: adding system calls that need to be handled by the kernel #task2.1 #task2.2

// system call to change process priority
int sys_priority(void){

    // collect arguments
    int priorityNumber;
    if(argint(0, &priorityNumber) < 0)
        return -1;

    // call kernel space priority function, and return its value to the caller
    priority(priorityNumber);
    return 0;
}

// system call to extract information and present to the user - process times
int sys_wait_stat(void) {
    int *status;
    struct perf *performance;

    if (argptr(0, (char **) &status, sizeof(int *)) < 0) {
        return -1;
    }

    if (argptr(1, (char **) &performance, sizeof(struct perf *)) < 0) {
        return -1;
    }

    cprintf("kernel: performance->tTime = %d\n", performance->tTime);
    cprintf("kernel: performance->cTime = %d\n", performance->cTime);
    cprintf("kernel: performance->reTime = %d\n", performance->reTime);
    cprintf("kernel: performance->ruTime = %d\n", performance->ruTime);
    cprintf("kernel: performance->sTime = %d\n", performance->sTime);

    return wait_stat(status, performance);
}

// system call to support user space program 'policy' to change scheduler policy.
int sys_policy(void) {
    int sched_policy_id;
    if (argint(0, &sched_policy_id) < 0)
        return -1; // fail

    policy(sched_policy_id);

    return 0; // success
}
// changed #end

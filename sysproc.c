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
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
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
  while(ticks - ticks0 < n){
    if(proc->killed){
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

// changed: support threads system calls #task2.1
int sys_kthread_create(void) {
    void *(*start_func)() = 0;
    void *stack = 0;
    int stack_size = 0;

    if (// load parameters from user space to kernel space
            argptr(0, (char **) start_func, sizeof(void **)) < 0 ||
            argptr(1, (char **) &stack, sizeof(void *)) < 0 ||
            argint(2, &stack_size) < 0
            ) {
        return -1;
    }
  return kthread_create(start_func, stack, stack_size);
}

int sys_kthread_id(void) {
    // no parameters for this function
    return kthread_id();
}

int sys_kthread_exit(void) {
    // no parameters for this function
    kthread_exit();
    return 0;
}

int sys_kthread_join(void) {
    int thread_id = 0;

    if (// load parameters from user space to kernel space
            argint(0, &thread_id) < 0
            ) {
        return -1;
    }
    return kthread_join(thread_id);
}
//changed #end
// Changed: Created by idan on 9/6/16. define the kernel thread structure
#ifndef XV6_PUBLIC_KTHREAD_H
#define XV6_PUBLIC_KTHREAD_H

#include "spinlock.h"

// CONSTANTS
#define NTHREAD 16
#define MAX_MUTEXES 64

enum threadstate {
    T_UNUSED, T_EMBRYO, T_SLEEPING, T_RUNNABLE, T_RUNNING, T_ZOMBIE
};

struct thread {
//    uint sz;                     // Size of thread memory (bytes) // note: stayed in process
//    pde_t* pgdir;                // Page table // note: stayed in process
    char *kstack;                // Bottom of kernel stack for this thread
    enum threadstate state;      // Thread state
    int tid;                     // Thread ID
    struct proc *parent;       // Parent thread // DONE: is needed for threads
    struct trapframe *tf;        // Trap frame for current syscall
    struct context *context;     // swtch() here to run thread
    void *chan;                  // If non-zero, sleeping on chan
    int killed;                  // If non-zero, have been killed
//    struct file *ofile[NOFILE];  // Open files note: not used for thread -> they have shared resources
//    struct inode *cwd;           // Current directory note: not used for thread -> they have shared resources
    char name[16];               // Thread name (debugging)
};

// DONE: 1. contains some constants, as well as the prototypes of the KLT package API functions.
// DONE: 2. Understand what each of the fields in proc struct means to the process.
// DONE: 3. transfer the entire system to work with the new thread structure.
// DONE: 4. every process will have 1 initial thread running in it.
// DONE: 5. create the system calls that will allow the user to create and manage new threads.

/*
 * DONE: moving to threads #task1.1
 * DONE: 1. Add a pointer to current thread (of type struct thread) right after the pointer to proc struct
 * (see struct cpu at proc.h).
 * DONE: 2. Update function seginit (see vm.c) so that the line 31:
 * c->gdt[SEG_KCPU] = SEG(STA_W, &c->cpu, 8, 0);
 * will be replaced with following line:
 * c->gdt[SEG_KCPU] = SEG(STA_W, &c->cpu, 12, 0);
 * DONE: 3. Add a global pointer to current thread by inserting following line to proc.h:
 * extern struct thread *thread asm("%gs:8");
 */
// DONE: additional changes may be required (e.g., the scheduler, for example, must update thread in addition to proc).
// DONE: Add a constant to kthread.h called NTHREAD = 16.
// DONE: each process should contain its own static array of threads.
/*
 * todo: synchronizing all shared fields of the thread owner when they are accessed
 * DONE: add lock to proc struct,
 */

// DONE: thread system calls #task1.2

enum mutexstate{
    M_UNUSED, M_UNLOCKED, M_LOCKED, M_EMBRYO
};

struct kthread_mutex_t{
    int mid; // mutex id
    enum mutexstate state;
    struct thread * owner;
    char name[16];
};





#endif //XV6_PUBLIC_KTHREAD_H
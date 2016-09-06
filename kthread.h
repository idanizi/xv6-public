// Changed: Created by idan on 9/6/16. define the kernel thread structure
#ifndef XV6_PUBLIC_KTHREAD_H
#define XV6_PUBLIC_KTHREAD_H

// CONSTANTS
#define NTHREAD 16

enum threadstate {
    T_UNUSED, T_EMBRYO, T_SLEEPING, T_RUNNABLE, T_RUNNING, T_ZOMBIE
};

struct thread {
    uint sz;                     // Size of thread memory (bytes)
//    pde_t* pgdir;                // Page table - note: not used for thread -> they have shared resources
    char *kstack;                // Bottom of kernel stack for this thread
    enum threadstate state;      // Thread state
    int tid;                     // Thread ID
    struct thread *parent;       // Parent thread // TODO: is needed?
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
// todo: 3. transfer the entire system to work with the new thread structure.
// todo: 4. every process will have 1 initial thread running in it.
// todo: 5. create the system calls that will allow the user to create and manage new threads.



/*
 * TODO: moving to threads #task1.1
 * todo: 1. Add a pointer to current thread (of type struct thread) right after the pointer to proc struct
 * (see struct cpu at proc.h).
 * todo: 2. Update function seginit (see vm.c) so that the line 31:
 * c->gdt[SEG_KCPU] = SEG(STA_W, &c->cpu, 8, 0);
 * will be replaced with following line:
 * c->gdt[SEG_KCPU] = SEG(STA_W, &c->cpu, 12, 0);
 * todo: 3. Add a global pointer to current thread by inserting following line to proc.h:
 * extern struct thread *thread asm("%gs:8");
 */
// todo: additional changes may be required (e.g., the scheduler, for example, must update thread in addition to proc).
// todo: Add a constant to kthread.h called NTHREAD = 16.
// todo: each process should contain its own static array of threads.
/*
 * todo: change scheduler in proc.c to support threads:
 * As you have seen in assignment 1, XV6 implements scheduling policy that goes over the list of processes
 * and chooses the next RUNNABLE process in the array. You are expected to change the scheduling so it
 * will run over all the available threads of a specific process before proceeding to the next process. This is
 * similar to the original scheduling policy, just over threads.
 */

/*
 * todo: synchronizing all shared fields of the thread owner when they are accessed
 * add lock to proc struct, initlock "proc" at allocproc.
 */

/*
 * todo: EXPECTED BEHAVIOR:
 * todo: 1. Fork – should duplicate only the calling thread,
 * if other threads exist in the process they will not
 * exist in the new process
 * todo: 2. Exec – should start running on a single thread of the new process.
 * Note that in a multi-threaded environment a thread might be running while another thread of the same process is attempting to perform exec.
 * The thread performing exec should “tell” other threads of the same process to
 * destroy themselves and only then complete the exec task.
 * todo: Hint: You can review the code that is performed when the proc->killed field is set and write your implementation similarly.
 * todo: 3. Exit – should kill the process and all of its threads,
 * remember while a single threads executing exit, others threads of the same process might still be running.
 */

// todo: thread system calls #task1.2
// todo: implement int kthread_create( void*(*start_func)(), void* stack, int stack_size);
/*
 * look in allocproc and copy the code & idea
 * todo: malloc for *stack ant the user space program, pass the pointer by sysprpc.c to the system call in the kernel
 * You will need to create the stack in user mode and send its pointer to the system call in order to be
 * consistent with current memory allocator of xv6.
 *
 * Calling kthread_create will create a new thread within the context of the calling process.
 * The newly created thread state will be RUNNABLE.
 * The caller of kthread_create must allocate a user stack for the new thread to use
 * (it should be enough to allocate a single page i.e., 4K for the thread stack).
 * This does not replace the kernel stack for the thread.
 * start_func is a pointer to the entry function, which the thread will start executing.
 * Upon success, the identifier of the newly created thread is returned.
 * In case of an error, a non-positive value is returned.
 */
// todo: implement int kthread_id();
/*
 * Upon success, this function returns the caller thread's id. In case of error,
 * a non-positive error identifier is returned. Remember, thread id and process id are not identical.
 */
// todo: implement void kthread_exit();
/*
 * This function terminates the execution of the calling thread. If called by a thread (even the main thread)
 * while other threads exist within the same process, it shouldn’t terminate the whole process. If it is the
 * last running thread, process should terminate. Each thread must explicitly call kthread_exit() in order to
 * terminate normally.
 */
// todo: implement int kthread_join(int thread_id);
/*
 * This function suspends the execution of the calling thread until the target thread (of the same process),
 * indicated by the argument thread_id, terminates. If the thread has already exited (or not exists),
 * execution should not be suspended. If successful, the function returns zero. Otherwise, -1 should be
 * returned to indicate an error.
 */
// TODO: write a user program to test all the system calls stated above

#endif //XV6_PUBLIC_KTHREAD_H
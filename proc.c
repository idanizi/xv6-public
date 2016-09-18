#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;

struct spinlock threadLocks[NPROC]; // changed #task1.1

static struct proc *initproc;

int nextpid = 1;
int nexttid = 1; // changed #task1.1
extern void forkret(void);

extern void trapret(void);

static void wakeup1(void *chan);

int debug_mode = 0;

void debug(int mode){
    debug_mode = mode;
}

void
pinit(void) {
    initlock(&ptable.lock, "ptable");
    // changed: initlock for threads locks done here #task1.1
    int i = 0, j = 0;
    for (i = 0; i < NPROC; i++) {
        itoa(i, ptable.proc[i].name);
        strcat(ptable.proc[i].name, "_tl"); // name for thread lock.
        initlock(&threadLocks[i], ptable.proc[i].name);
        for(j = 0; j < NTHREAD; j++){
            ptable.proc[i].threadTable.threads[j].state = T_UNUSED;
            ptable.proc[i].threadTable.threads[j].parent = 0;
            ptable.proc[i].threadTable.threads[j].chan = 0;
            ptable.proc[i].threadTable.threads[j].context = 0;
            ptable.proc[i].threadTable.threads[j].killed = 0;
            ptable.proc[i].threadTable.threads[j].kstack = 0;
            ptable.proc[i].threadTable.threads[j].tf = 0;
            ptable.proc[i].threadTable.threads[j].tid = 0;
        }
    }
    // changed #end
}

// DONE: loop for init all 16 threads in the process:
/*
 * 1. first thread EMBRYO / runnable
 * DONE: 2. all the rest are unused
 * 3. tid for each thread
 * 4. go for each field and initiate it:
 * TODO: t->name;
 * DONE: allocproc: t->state = EMBRYO;
 * DONE: allocproc: t->tid = nexttid++;
 * DONE: pinit: t->chan = 0;
 * DONE: userinit/fork: t->context;
 * DONE: pinit: t->killed = 0;
 * DONE: allocproc: t->parent = p;
 * DONE: userinit/fork: t->tf;
 */
//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(void) {
    struct proc *p;
    char *sp;
    int pindex = 0; // changed #task1.1

    acquire(&ptable.lock); // fixme panic acquire
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        pindex++; // changed #task1.1
        if (p->state == UNUSED) {
            goto found;
        }
    }
    release(&ptable.lock); // fixme panic acquire
    return 0;

    found:
    p->state = EMBRYO;
    p->pid = nextpid++;

    // changed: threads table lock association (pointer) -> instead of initlock here. (moved to pinit()) #task1.1
    // changed: give process a name. #task1.1
    itoa(p->pid, p->name);
    strcat(p->name, "_p");

    // assign thread table lock pointer to its lock in the threadLocks array.
    p->threadTable.lock = &threadLocks[pindex];
    struct thread *t = p->threadTable.threads; // first thread;
    t->parent = p;// NOTE: parent of thread its his process
    t->tid = nexttid++;

    // end of critical section process
    release(&ptable.lock); // changed location #task1.1 // fixme panic acquire

    // Allocate kernel stack.
    if ((t->kstack = kalloc()) == 0) {
        t->state = T_UNUSED;
        return 0;
    }
    sp = t->kstack + KSTACKSIZE;

    // Leave room for trap frame.
    sp -= sizeof *t->tf;
    t->tf = (struct trapframe *) sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= 4;
    *(uint *) sp = (uint) trapret;

    sp -= sizeof *t->context;
    t->context = (struct context *) sp;
    memset(t->context, 0, sizeof *t->context);
    t->context->eip = (uint) forkret;

    // changed: this code has been canceled to be moved to the loop of initiating threads in the new process #task1.1
//  // Allocate kernel stack.
//  if((p->kstack = kalloc()) == 0){
//    p->state = UNUSED;
//    return 0;
//  }
//  sp = p->kstack + KSTACKSIZE;
//
//  // Leave room for trap frame.
//  sp -= sizeof *p->tf;
//  p->tf = (struct trapframe*)sp;
//
//  // Set up new context to start executing at forkret,
//  // which returns to trapret.
//  sp -= 4;
//  *(uint*)sp = (uint)trapret;
//
//  sp -= sizeof *p->context;
//  p->context = (struct context*)sp;
//  memset(p->context, 0, sizeof *p->context);
//  p->context->eip = (uint)forkret;

    // changed #end

    return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void) {
    struct proc *p;
    extern char _binary_initcode_start[], _binary_initcode_size[];

    p = allocproc();
    initproc = p;
    // changed #task1.1
    struct thread *t = p->threadTable.threads; // first thread
    // changed #end
    if ((p->pgdir = setupkvm()) == 0)
        panic("userinit: out of memory?");
    inituvm(p->pgdir, _binary_initcode_start, (int) _binary_initcode_size);
    p->sz = PGSIZE;
    memset(t->tf, 0, sizeof(*t->tf));
    t->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    t->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    t->tf->es = t->tf->ds;
    t->tf->ss = t->tf->ds;
    t->tf->eflags = FL_IF;
    t->tf->esp = PGSIZE;
    t->tf->eip = 0;  // beginning of initcode.S

    // changed: removed to support threads #task1.1
//  if((p->pgdir = setupkvm()) == 0)
//    panic("userinit: out of memory?");
//  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
//  p->sz = PGSIZE;
//  memset(p->tf, 0, sizeof(*p->tf));
//  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
//  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
//  p->tf->es = p->tf->ds;
//  p->tf->ss = p->tf->ds;
//  p->tf->eflags = FL_IF;
//  p->tf->esp = PGSIZE;
//  p->tf->eip = 0;  // beginning of initcode.S

    // changed #end

    safestrcpy(p->name, "initcode", sizeof(p->name)); // changed #task1.1 // todo is needed?
    p->cwd = namei("/");

    acquire(&ptable.lock); // fixme &ptable.lock deadlock?
    t->state = T_RUNNABLE; // changed: first thread runnable #task1.1
    p->state = RUNNABLE;
    release(&ptable.lock); // fixme &ptable.lock deadlock?
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n) {
    uint sz;

    sz = proc->sz;
    if (n > 0) {
        if ((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
            return -1;
    } else if (n < 0) {
        if ((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
            return -1;
    }
    proc->sz = sz;

    // changed #task1.1
    switchuvm(thread);
    // changed #end
    return 0;
}

/*
 * DONE: 1. Fork – should duplicate only the calling thread,
 * if other threads exist in the process they will not
 * exist in the new process
 */
// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void) {
    int i, pid;
//  int tid; // changed #tsak1.1
    struct proc *np;
    struct thread *nt; // changed #task1.1

    // Allocate process.
    if ((np = allocproc()) == 0) {
        return -1;
    }

    // changed: supporting threads #task1.1
    // first thread of the new process is the new embryo thread
    nt = np->threadTable.threads;
    // Copy process state from p.
    if ((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0) {
        // case of failure
        if (nt->kstack)
            kfree(nt->kstack);
        nt->kstack = 0;
        nt->state = T_UNUSED;
        np->state = UNUSED;
        return -1;
    }
    // changed #end
    np->sz = proc->sz;
    np->parent = proc;

    // changed #task1.1
    *nt->tf = *thread->tf;

    // Clear %eax so that fork returns 0 in the child.
    nt->tf->eax = 0;
    // changed #end

    // file system usage for process
    for (i = 0; i < NOFILE; i++)
        if (proc->ofile[i])
            np->ofile[i] = filedup(proc->ofile[i]);
    np->cwd = idup(proc->cwd);

    safestrcpy(np->name, proc->name, sizeof(proc->name)); // todo why?

    pid = np->pid;
//  tid = nt->tid; // changed #task1.1

    // lock to force the compiler to emit the np->state write last.
    acquire(&ptable.lock);  // fixme panic acquire
    np->state = RUNNABLE;
    nt->state = T_RUNNABLE; // changed #task1.1
    if (debug_mode) cprintf("fork: nt->tid: %d created\n", nt->tid); // todo del
    release(&ptable.lock); // fixme panic acquire

    return pid;
}

/*
 * DONE: 3. Exit – should kill the process and all of its threads,
 * remember while a single threads executing exit, others threads of the same process might still be running.
 */
// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void) {
    if (debug_mode && (proc || thread)) { // todo del
        cprintf("exiting ");
        if (proc && proc->state == RUNNING)
            cprintf("proc->pid: %d ", proc->pid);
        if (thread && thread->state == T_RUNNING) {
            cprintf("thread->tid: %d ", thread->tid);
            if (thread->parent)
                cprintf("thread->parent->pid: %d ", thread->parent->pid);
        }
        cprintf("\n");
    }

    struct proc *p;
    struct thread *t; // changed #task1.1
    int fd;

    // todo: do the same for threads - initthread?
    if (proc == initproc)
        panic("init exiting");

    // kthread_join to all other threads
    acquire(thread->parent->threadTable.lock); // fixme deadlocks
    for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
        if (t->tid != thread->tid && t->state != T_UNUSED) {
            t->killed = 1;
            // wake threads from sleep to finish their run
            if (t->state == T_SLEEPING) t->state = T_RUNNABLE;
        }
    }
    release(proc->threadTable.lock); // fixme deadlocks

    // look for killed threads and join them (wait for them to finish their run)
    for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
        if(t->killed)
            kthread_join(t->tid);
    }

    // Close all open files.
    for (fd = 0; fd < NOFILE; fd++) {
        if (proc->ofile[fd]) {
            fileclose(proc->ofile[fd]);
            proc->ofile[fd] = 0;
        }
    }

    begin_op();
    iput(proc->cwd);
    end_op();
    proc->cwd = 0;

    // start critical section for processes
    acquire(&ptable.lock); // fixme panic acquire

    // Parent might be sleeping in wait().
    wakeup1(proc->parent);

    // Pass abandoned children to init.
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->parent == proc) {
            p->parent = initproc;
            if (p->state == ZOMBIE)
                wakeup1(initproc);
        }
    }

    // Jump into the scheduler, never to return.
    proc->state = ZOMBIE;
    // changed #task1.1
//    thread->state = T_UNUSED;
    for (t = proc->threadTable.threads; t < &proc->threadTable.threads[NTHREAD]; t++) {
        if (t->state != T_UNUSED)
            t->state = T_ZOMBIE;
    }
    // changed #end
    sched();
    panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void) {
    struct proc *p;
    struct thread *t; // changed #task1.1
    int havekids, pid;

    acquire(&ptable.lock); // fixme panic acquire
    for (;;) {
        // Scan through table looking for zombie children.
        havekids = 0;
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->parent != proc)
                continue;
            havekids = 1;
            if (p->state == ZOMBIE) {
                // Found one.
                pid = p->pid;
                // changed: move to threads responsibility #task1.1
//        kfree(p->kstack);
//        p->kstack = 0;
//        freevm(p->pgdir);
                // CS for threads
//                acquire(p->threadTable.lock); // fixme deadlock
                for (t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++) {
                    if (t->state == T_ZOMBIE) {
                        if (t->kstack)
                            kfree(t->kstack); // FIXME kfree panic
                        t->kstack = 0;
                        freevm(t->parent->pgdir);
                        t->state = T_UNUSED;
                        t->tid = 0;
                        t->parent = 0;
                        t->name[0] = 0;
                        t->killed = 0;
                    }
                }
//                release(p->threadTable.lock);  // fixme deadlocks
                // end of CS
                // changed #end

                p->state = UNUSED;
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                release(&ptable.lock); // fixme panic acquire
                return pid;
            }
        }

        // No point waiting if we don't have any children.
        if (!havekids || proc->killed) {
            release(&ptable.lock); // fixme panic acquire
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(proc, &ptable.lock);  //DOC: wait-sleep
    }
}

/*
 * DONE: change scheduler in proc.c to support threads:
 * As you have seen in assignment 1, XV6 implements scheduling policy that goes over the list of processes
 * and chooses the next RUNNABLE process in the array. You are expected to change the scheduling so it
 * will run over all the available threads of a specific process before proceeding to the next process. This is
 * similar to the original scheduling policy, just over threads.
 */

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void) {
    if(debug_mode) cprintf("in scheduler\n"); // todo del
    struct proc *p;
    struct thread *t; // changed #task1.1

    for (;;) {
        // Enable interrupts on this processor.
        sti();

        // Loop over process table looking for process to run.
        acquire(&ptable.lock); // fixme panic acquire
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->state != RUNNABLE)
                continue;

            // Switch to chosen process.  It is the process's job
            // to release ptable.lock and then reacquire it
            // before jumping back to us.

            // changed #task1.1
            // loop until find runnable thread
//            cprintf("scheduler: loop until find runnable thread\n"); // todo del
//            for (t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD] && t->state != T_RUNNABLE; t++);
//            if (t >= &p->threadTable.threads[NTHREAD]) {
//                // no runnable threads available in this process, so jump to next process
//                continue;
//            }

            for (t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++) {
                if (t->state == T_RUNNABLE)
                    goto found;
            }
            // no runnable threads at this process, jump to next process
            continue;

            // found runnable thread at this process
            found:
            // changed #end

            proc = p;

            // changed #task1.1
            thread = t;
            switchuvm(t);
            t->state = T_RUNNING;
            // changed #end

            p->state = RUNNING;

            if (debug_mode >= 2 && p->pid != t->tid)
                cprintf("scheduler: running pid=%d tid=%d\n", p->pid, t->tid); // todo del
            swtch(&cpu->scheduler, thread->context); // changed #task1.1
            switchkvm();

            // Process is done running for now.
            // It should change its p->state before coming back.
            thread = 0; // changed #task1.1
            proc = 0;
        }
        release(&ptable.lock); // fixme panic acquire

    }
}

// Enter scheduler.  Must hold only ptable.lock
// and change proc->state.
void
sched(void) {
    int intena;

    if (!holding(&ptable.lock)) // fixme panic acquire
        panic("sched ptable.lock");
    //changed #task1.1
//    if (!holding(proc->threadTable.lock)) { // fixme deadlocks
//        panic("sched proc.threadTable.lock");
//    }
    //changed #end
    if (cpu->ncli != 1)
        panic("sched locks");
    if (proc && proc->state == RUNNING) // changed avoid segfault #task1.1
        panic("sched running");
    // changed #task1.1
    if (thread && thread->state == T_RUNNING) { // changed avoid segfault #task1.1
        panic("sched thread running");
    }
    //changed #end
    if (readeflags() & FL_IF)
        panic("sched interruptible");
    intena = cpu->intena;
//    cprintf("sched: tid: %d try swtch\n", thread->tid); // todo del
    swtch(&thread->context, cpu->scheduler); // changed #task1.1
    cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void) {
    acquire(&ptable.lock);  //DOC: yieldlock // fixme panic acquire
    thread->state = T_RUNNABLE; //changed #task1.1
    proc->state = RUNNABLE;
    sched();
    release(&ptable.lock); // fixme panic acquire
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void) {
    static int first = 1;
    // Still holding ptable.lock from scheduler.
    release(&ptable.lock); // fixme panic acquire

    if (first) {
        // Some initialization functions must be run in the context
        // of a regular process (e.g., they call sleep), and thus cannot
        // be run from main().
        first = 0;
        iinit(ROOTDEV);
        initlog(ROOTDEV);
    }

    // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk) {
    if (proc == 0)
        panic("sleep");

    //changed #task1.1
    if (thread == 0) {
        panic("sleep thread");
    }
    // changed #end

    if (lk == 0)
        panic("sleep without lk");

    // Must acquire ptable.lock in order to
    // change p->state and then call sched.
    // Once we hold ptable.lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup runs with ptable.lock locked),
    // so it's okay to release lk.
    if (lk != &ptable.lock) {  //DOC: sleeplock0
        acquire(&ptable.lock);  //DOC: sleeplock1 // fixme panic acquire
        release(lk); // fixme panic acquire
    }

    // Go to sleep.
    // changed #task1.1
//    proc->chan = chan;
//    proc->state = SLEEPING;
    proc->state = RUNNABLE;
    thread->chan = chan;
    thread->state = T_SLEEPING;
    //changed #end
    sched();

    // Tidy up.
//    proc->chan = 0; // changed #task1.1
    thread->chan = 0; // changed #task1.1

    // Reacquire original lock.
    if (lk != &ptable.lock) {  //DOC: sleeplock2
        release(&ptable.lock); // fixme panic acquire
        acquire(lk); // fixme panic acquire
    }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void wakeup1(void *chan) {
//    if (chan != &ticks)
    struct proc *p;
    // changed #task1.1
    struct thread *t;

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == RUNNABLE || p->state == RUNNING) {
            for(t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++){
                if(t->state == T_SLEEPING && t->chan == chan){
                    t->state = T_RUNNABLE;
                }// if thread sleeping
            }// for thread
        }// if process sleeping
    }// for process
    // changed #end
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan) {
    acquire(&ptable.lock); // fixme panic acquire
    wakeup1(chan);
    release(&ptable.lock); // fixme panic acquire
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid) {
    struct proc *p;
    struct thread *t; // changed #task1.1

    acquire(&ptable.lock); // fixme panic acquire
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->killed = 1;
            // changed #task1.1
            for (t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++) {
                t->killed = 1;
                // wake threads from sleep
                if (t->state == T_SLEEPING) {
                    t->state = T_RUNNABLE;
                }
            }
            release(&ptable.lock); // fixme panic acquire
            return 0;
            // changed #end
        }
    }

    release(&ptable.lock); // fixme panic acquire
    return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void) {
    static char *states[] = {
            [UNUSED]    "unused",
            [EMBRYO]    "embryo",
//            [SLEEPING]  "sleep ",
            [RUNNABLE]  "runble",
            [RUNNING]   "run   ",
            [ZOMBIE]    "zombie"
    };
    int i;
    struct proc *p;
    struct thread *t;
    char *state;
    uint pc[10];

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED)
            continue;
        if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
            state = states[p->state];
        else
            state = "???";
        cprintf("%d %s %s", p->pid, state, p->name);
        // changed #task1.1
//        if (p->state == SLEEPING) {
//            acquire(p->threadTable.lock);// fixme deadlock?
        for (t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++) {
            if (t->state == T_SLEEPING) {
                getcallerpcs((uint *) t->context->ebp + 2, pc);
                for (i = 0; i < 10 && pc[i] != 0; i++)
                    cprintf(" %p", pc[i]);
            }
        }
//            release(p->threadTable.lock);// fixme deadlock?
//            getcallerpcs((uint *) p->context->ebp + 2, pc);
//            for (i = 0; i < 10 && pc[i] != 0; i++)
//                cprintf(" %p", pc[i]);
//        } // if (p->state == SLEEPING)
        //changed #end
        cprintf("\n");
    }
}

// changed: system calls supporting threads for the user space programs #task1.2
// DONE: implement int kthread_create( void*(*start_func)(), void* stack, int stack_size);
/*
 * look in allocproc and copy the code & idea
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
int kthread_create(void *(*start_func)(), void *stack, int stack_size) {
    struct thread *t;
    char *sp;


    if (!thread || !thread->parent)
        return -1; // fail: null cases

    if (stack_size > KSTACKSIZE) {
        cprintf("kthread_create: pid=%d tid=%d: error: %d = stack_size > KSTACKSIZE = %d\n", proc->pid,
                thread->tid, stack_size, KSTACKSIZE);
        return -1;
    }

    acquire(thread->parent->threadTable.lock); // fixme deadlocks?
    for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
        if (t->state == T_UNUSED) {
            goto found_thread;
        }
    }
    release(thread->parent->threadTable.lock); // fixme deadlocks?
    return -1; // fail: all 16 threads are busy

    found_thread:
    acquire(&ptable.lock); // fixme ptable.lock deadlocks?
    t->state = T_EMBRYO;
    t->tid = nexttid++;
    t->parent = thread->parent;
    release(&ptable.lock); // fixme ptable.lock deadlocks?

    release(thread->parent->threadTable.lock); // fixme deadlocks?

    // FIXME: Allocate thread memory space
    if ((t->kstack = kalloc()) == 0) {
        t->state = T_UNUSED;
        return -1;
    }
    sp = t->kstack + KSTACKSIZE;

    // Leave room for trap frame.
    sp -= sizeof *t->tf;
    t->tf = (struct trapframe *) sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= 4;
    *(uint *) sp = (uint) trapret;

    sp -= sizeof *t->context;
    t->context = (struct context *) sp;
    memset(t->context, 0, sizeof *t->context);
    t->context->eip = (uint) forkret;

    // todo is needed in kthread_create?: grow process memory
//    if (growproc(KSTACKSIZE) < 0) {
//        panic("kthread_create growproc failed");
//    }


    // setup new thread trap-frame as current thread trap-frame
    memset(t->tf, 0, sizeof(*t->tf));
    t->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    t->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    t->tf->es = t->tf->ds;
    t->tf->ss = t->tf->ds;
    t->tf->eflags = FL_IF;
    t->tf->esp = (uint) stack + stack_size;
    t->tf->eip = (uint) start_func; // set eip to start from start_func

    acquire(&ptable.lock);
    t->state = T_RUNNABLE;
    release(&ptable.lock);

    if(debug_mode) cprintf("kthread_create: tid %d created, start_func: 0x%x\n", t->tid, start_func); // todo del

    return t->tid;
}

// DONE: implement int kthread_id();
/*
 * Upon success, this function returns the caller thread's id. In case of error,
 * a non-positive error identifier is returned. Remember, thread id and process id are not identical.
 */
int kthread_id(void) {
    if(debug_mode) cprintf("in kthread_id\n"); // todo del
    if (thread)
        return thread->tid; // success

    return -1; // fail
}

// Wake up all threads sleeping on chan.
// The ptable lock must be held.
//void kthread_wakeup1(void *chan) {
//    struct thread *t;
//
//    for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
//        if (t->state == T_SLEEPING && t->chan == chan) {
//            t->state = T_RUNNABLE;
//        }// if thread sleeping
//    }// for thread
//}

// DONE: implement void kthread_exit();
/*
 * This function terminates the execution of the calling thread. If called by a thread (even the main thread)
 * while other threads exist within the same process, it shouldn’t terminate the whole process. If it is the
 * last running thread, process should terminate. Each thread must explicitly call kthread_exit() in order to
 * terminate normally.
 */
void kthread_exit() {
    struct thread *t;

    if(debug_mode) cprintf("in kthread_exit: tid=%d\n", thread->tid); // todo del

    if (!thread)
        panic("try to kthread_exit with current thread null");

    acquire(thread->parent->threadTable.lock); // fixme threadTable.lock deadlock?
    for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
        if (t != T_UNUSED && t != thread)
            goto other_threads_alive;
    }
    // this is the 'last man standing' thread in the parent process
    if(debug_mode) cprintf("kthread_exit: tid=%d: this is the 'last man standing' thread in the parent process\n", thread->tid); // todo del
    release(thread->parent->threadTable.lock); // fixme threadTable.lock deadlock?
    exit();

    other_threads_alive:
    // there are still threads alive in this process
    if(debug_mode) cprintf("kthread_exit: tid=%d: there are still threads alive in this process\n", thread->tid); // todo del
    acquire(&ptable.lock);
    thread->state = T_ZOMBIE; // change current thread state to die
    wakeup1((void *) thread); // wake everybody who sleeps over this thread
    proc->state = RUNNABLE;
    release(thread->parent->threadTable.lock); // fixme threadTable.lock deadlock?

    // go to scheduler, ptable.lock already acquired & proc->state is runnable
    sched();
    panic("thread zombie exit!");
}

//void kthread_sleep(void *chan, struct spinlock *lk) {
//    if (!thread) {
//        panic("kthread_sleep thread");
//    }
//
//    cprintf("in thread sleep, tid: %d\n", thread->tid); // todo del
//
//    if (lk == 0)
//        panic("kthread_sleep without lk");
//
//    // Must acquire ptable.lock in order to
//    // change p->state and then call sched.
//    // Once we hold ptable.lock, we can be
//    // guaranteed that we won't miss any wakeup
//    // (wakeup runs with ptable.lock locked),
//    // so it's okay to release lk.
//    cprintf("thread sleep: tid=%d: switch locks - if (lk != &ptable.lock)\n", thread->tid); // todo del
//    if (lk != &ptable.lock) {  //DOC: sleeplock0
//        acquire(&ptable.lock);  //DOC: sleeplock1 // fixme panic acquire
//        release(lk); // fixme panic acquire
//    }
//
//    // Go to sleep.
//    cprintf("thread sleep: tid=%d: change states\n", thread->tid); // todo del
//    thread->parent->state = RUNNABLE;
//    thread->chan = chan;
//    thread->state = T_SLEEPING;
//    cprintf("thread sleep: tid=%d: sched\n", thread->tid); // todo del
//    sched();
//
//    // Tidy up.
//    thread->chan = 0;
//
//    // Reacquire original lock.
//    cprintf("thread sleep: Reacquire original lock\n"); // todo del
//    if (lk != &ptable.lock) {  //DOC: sleeplock2
//        release(&ptable.lock); // fixme panic acquire
//        acquire(lk); // fixme panic acquire
//    }
//}

// DONE: implement int kthread_join(int thread_id);
/*
 * This function suspends the execution of the calling thread until the target thread (of the same process),
 * indicated by the argument thread_id, terminates. If the thread has already exited (or not exists),
 * execution should not be suspended. If successful, the function returns zero. Otherwise, -1 should be
 * returned to indicate an error.
 */
int kthread_join(int thread_id) {
//    cprintf("in join thread\n"); // todo del
    struct thread *t;

    acquire(thread->parent->threadTable.lock); // fixme threadTable.lock deadlock?
//    cprintf("join: acquire(thread->parent->threadTable.lock)\n"); // todo del
    if (thread_id == thread->tid)
        return -1;// error: thread can't join itself

    // find thread by tid
    for (t = thread->parent->threadTable.threads; t < &thread->parent->threadTable.threads[NTHREAD]; t++) {
        if (t->tid == thread_id)
            goto found;
    }
    // thread_id not found
//    cprintf("join: thread_id not found\n"); // todo del
    release(thread->parent->threadTable.lock); // fixme threadTable.lock deadlock?
    return -1;

    found:
//    cprintf("join: thread_id = %d found\n", thread_id); // todo del
    for (;;) {
        if(t->state == T_UNUSED || t->killed == 1){
            // error: joining illegal thread
//            cprintf("join: tid=%d: error: thread_id = %d t->killed=%d t->state=%d\n", thread->tid, thread_id, t->killed,                    t->state); // todo del
            release(thread->parent->threadTable.lock); // fixme threadTable.lock deadlock?
            return -1;
        }
        if (t->state == T_ZOMBIE) {
            if(debug_mode) cprintf("join: tid=%d: deleting zombie tid = %d\n", thread->tid, t->tid); // todo del
            if (t->kstack)
                kfree(t->kstack);
            t->kstack = 0;
            t->state = T_UNUSED;
            t->tid = 0;
            t->parent = 0;
            t->name[0] = 0;
            t->killed = 0;
            if(debug_mode) cprintf("join: tid=%d: zombie tid = %d deleted [finished]\n", thread->tid, t->tid); // todo del
            release(thread->parent->threadTable.lock); // fixme threadTable.lock deadlock?
            return 0;
        }
//        cprintf("join: tid: %d going to sleep on tid: %d\n", thread->tid, t->tid); // todo del
        sleep(t, thread->parent->threadTable.lock); // fixme threadTable.lock need to sleep on deferment lock?
        if(debug_mode) cprintf("join: tid: %d waked\n", thread->tid); // todo del
    }
}
// changed #end
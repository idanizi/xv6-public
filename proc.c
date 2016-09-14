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
    // changed #end

    // changed: #task1.1
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
    struct thread *t = p->threadTable.threads; // first thread;

    // NOTE: parent of thread its his process
    t->parent = p;
    t->tid = nexttid++;

    // end of critical section process
    release(&ptable.lock); // changed location #task1.1 // fixme panic acquire

    // Allocate kernel stack.
    if ((t->kstack = kalloc()) == 0) {
        t->state = UNUSED;
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

    safestrcpy(t->name, "initcode", sizeof(t->name)); // changed #task1.1 // todo is needed?
    p->cwd = namei("/");

    t->state = T_RUNNABLE; // changed: first thread runnable #task1.1
    p->state = RUNNABLE;
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
 * todo: 1. Fork – should duplicate only the calling thread,
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
    release(&ptable.lock); // fixme panic acquire

    return pid;
}

/*
 * todo: 3. Exit – should kill the process and all of its threads,
 * remember while a single threads executing exit, others threads of the same process might still be running.
 */
// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void) {
    if (proc || thread) { // todo del
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
                        kfree(t->kstack);
                        t->kstack = 0;
                        freevm(t->parent->pgdir);
                        t->state = T_UNUSED;
//                        t->tid = 0;// FIXME thread->tid = 0
                        t->parent = 0; // todo: is needed? #task1.1
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
 * todo: change scheduler in proc.c to support threads:
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
            for (t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD] && t->state != T_RUNNABLE; t++);
            if (t >= &p->threadTable.threads[NTHREAD]) {
                // no runnable threads available in this process, so jump to next process
                continue;
            }
            // changed #end

            proc = p;

            // changed #task1.1
            thread = t;
            switchuvm(t);
            t->state = T_RUNNING;
            // changed #end

            p->state = RUNNING;

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
    proc->chan = chan;
    proc->state = SLEEPING;
    // changed #task1.1
    thread->chan = chan;
    thread->state = T_SLEEPING;
    //changed #end
    sched();

    // Tidy up.
    proc->chan = 0;
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
static void
wakeup1(void *chan) {
//    if (chan != &ticks)
    struct proc *p;
    // changed #task1.1
    struct thread *t;

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == SLEEPING && p->chan == chan) {
            p->state = RUNNABLE;
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
//    if (chan != &ticks)
    acquire(&ptable.lock); // fixme panic acquire
//    if (proc)
//        acquire(proc->threadTable.lock); // fixme deadlock
    wakeup1(chan);
//    if (proc)
//        release(proc->threadTable.lock); // fixme deadlock
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
//            acquire(p->threadTable.lock);// fixme deadlock?
            for(t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++){
                t->killed = 1;
            }
//            release(p->threadTable.lock);// fixme deadlock?
            //changed #end
            // Wake process from sleep if necessary.
            if (p->state == SLEEPING) {
                p->state = RUNNABLE;
                // changed #task1.1
//                acquire(p->threadTable.lock);// fixme deadlock?
                for(t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++){
                    if(t->state == T_SLEEPING){
                        t->state = T_RUNNABLE;
                    }
                }
//                release(p->threadTable.lock);// fixme deadlock?
                // changed #end
            }
            release(&ptable.lock); // fixme panic acquire
            return 0;
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
            [SLEEPING]  "sleep ",
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
        if (p->state == SLEEPING) {
            // changed #task1.1
//            acquire(p->threadTable.lock);// fixme deadlock?
            for(t = p->threadTable.threads; t < &p->threadTable.threads[NTHREAD]; t++){
                getcallerpcs((uint *) t->context->ebp + 2, pc);
                for (i = 0; i < 10 && pc[i] != 0; i++)
                    cprintf(" %p", pc[i]);
            }
//            release(p->threadTable.lock);// fixme deadlock?
//            getcallerpcs((uint *) p->context->ebp + 2, pc);
//            for (i = 0; i < 10 && pc[i] != 0; i++)
//                cprintf(" %p", pc[i]);
            //changed #end
        }
        cprintf("\n");
    }
}

// changed: system calls supporting threads for the user space programs #task1.2
int kthread_create(void *(*start_func)(), void *stack, int stack_size) { // todo implement
    return 0;
}

int kthread_id(void) { // todo implement
    return 0;
}

void kthread_exit() { // todo implement

}

int kthread_join(int thread_id) { // todo implement
    return 0;
}
// changed #end
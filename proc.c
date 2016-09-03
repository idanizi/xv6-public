// changed #task2.2
#include "perf.h"
// changed #end

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

static struct proc *initproc;

int nextpid = 1;

extern void forkret(void);

extern void trapret(void);

static void wakeup1(void *chan);

static int uptime();

void
pinit(void) {
    initlock(&ptable.lock, "ptable");
}

int currentPolicy = UNIFORM_POLICY; // changed #task2.1

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(void) {
    struct proc *p;
    char *sp;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if (p->state == UNUSED)
            goto found;
    release(&ptable.lock);
    return 0;

    found:
    p->state = EMBRYO;
    // changed: The priority of a new processes is 10 / 20 / 1. #task2.1
    if(currentPolicy == UNIFORM_POLICY) {
        p->priority = 10;
    }else if(currentPolicy == DYNAMIC_POLICY){
        p->priority = 20;
    }else{
        p->priority = 1;
    }
    // changed #task2.2
    p->cTime = uptime(); // NOTE: creation time stamp
    // init all time counters to zero
    p->tTime = 0;
    p->sTime = 0;
    p->reTime = 0;
    p->ruTime = 0;
    // changed #end
    p->pid = nextpid++;
    release(&ptable.lock);

    // Allocate kernel stack.
    if ((p->kstack = kalloc()) == 0) {
        p->state = UNUSED;
        return 0;
    }
    sp = p->kstack + KSTACKSIZE;

    // Leave room for trap frame.
    sp -= sizeof *p->tf;
    p->tf = (struct trapframe *) sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= 4;
    *(uint *) sp = (uint) trapret;

    sp -= sizeof *p->context;
    p->context = (struct context *) sp;
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (uint) forkret;

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
    if ((p->pgdir = setupkvm()) == 0)
        panic("userinit: out of memory?");
    inituvm(p->pgdir, _binary_initcode_start, (int) _binary_initcode_size);
    p->sz = PGSIZE;
    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0;  // beginning of initcode.S

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = namei("/");

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
    switchuvm(proc);
    return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void) {
    int i, pid;
    struct proc *np;

    // Allocate process.
    if ((np = allocproc()) == 0)
        return -1;

    // Copy process state from p.
    if ((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0) {
        kfree(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
    }
    np->sz = proc->sz;
    np->parent = proc;
    *np->tf = *proc->tf;

    // Clear %eax so that fork returns 0 in the child.
    np->tf->eax = 0;

    for (i = 0; i < NOFILE; i++)
        if (proc->ofile[i])
            np->ofile[i] = filedup(proc->ofile[i]);
    np->cwd = idup(proc->cwd);

    safestrcpy(np->name, proc->name, sizeof(proc->name));

    pid = np->pid;

    // lock to force the compiler to emit the np->state write last.
    acquire(&ptable.lock);
    np->state = RUNNABLE;
    release(&ptable.lock);

    return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait(&status) to find out it exited.
void exit(int status) { // CHANGED
    struct proc *p;
    int fd;

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

    acquire(&ptable.lock);

    // Parent might be sleeping in wait(&status).
    wakeup1(proc->parent);

    // Pass abandoned children to init.
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->parent == proc) {
            p->parent = initproc;
            if (p->state == ZOMBIE)
                wakeup1(initproc);
        }
    }

    // changed: store the exit status of the terminated process #task1
    if (proc)
        proc->status = status;
    // changed: update process termination time #task2.2
    if (proc)
        proc->tTime = ticks;
    // changed #end

    // Jump into the scheduler, never to return.
    proc->state = ZOMBIE;
    sched();
    panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(int *status) { // changed
    struct proc *p;
    int havekids, pid;

    acquire(&ptable.lock);
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
                kfree(p->kstack);
                p->kstack = 0;
                freevm(p->pgdir);
                p->state = UNUSED;
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                release(&ptable.lock);
                if(status){ // CHANGED: return the exit status of child
                    *status = p->status;
                }
                return pid;
            }
        }

        // No point waiting if we don't have any children.
        if (!havekids || proc->killed) {
            release(&ptable.lock);
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(proc, &ptable.lock);  //DOC: wait-sleep
    }
}

// DONE: Change the code of the scheduler to generate a single random number (between 0 and the total number of the allocated tickets), which will represent a ticket number. The scheduler then will chose the process owning that ticket for execution. #task2.1
// DONE: add a system call: int schedp(int sched_policy_id), which will be used to change the sub-policy used – and will re-distribute the tickets accordingly. #task2.1

//changed #task1.2

// random function
// used by the scheduler
int random(int seed, int nTotalTickets) {
    if (nTotalTickets == 0) {
        nTotalTickets = 1;
    }
    return (RANDOM_NUMBER_1 * seed + RANDOM_NUMBER_2) % nTotalTickets;
}

// DONE: implement a new system call: void priority(int); #task2.1
/*
 * system call: void priority(int);
 * can be used by a process to change its priority.
 */

// change the sub-policy used – and will re-distribute the tickets accordingly
int schedp(int sched_policy_id) {
    struct proc *p = 0;

    // mutex critical section
    acquire(&ptable.lock);

    currentPolicy = sched_policy_id; // set the current policy.
    switch (sched_policy_id) {
        case UNIFORM_POLICY:
            // DONE: implement: Policy 1: Uniform time distribution
            /*
             * achieve a uniform time allocation to the processes (assuming that your implementation
             * of the random number generator achieves a uniform distribution of the return values).
             */
            for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
                p->nTickets = 1;
            }
            break;
        case PRIORITY_POLICY:
            // DONE: implement: Policy 2: Priority scheduling #task2.1
            /*
             * This scheduling policy will take the process priority into consideration while
             * deciding the number of tickets to allocate. For example, given two processes
             * p1 and p2 having priorities 1 and 2 accordingly, process p2 will receive
             * approximately twice the run-time received by p1.
             */
        case DYNAMIC_POLICY:
            // DONE: implement: Policy 3: Dynamic tickets allocation #task2.1
            /*
             * This policy will dynamically reallocate the tickets in response to the process
             * behavior. A newly created process will get 20 tickets. Each time a process performs
             * a blocking system call, it will receive additional 10 tickets (up to maximum of 100
             * tickets) and each time a process ends the quanta without performing a blocking system
             * call, the amount of the tickets owned be the process will be reduced by 1 (to the
             * minimum of 1).
             */
            for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
                p->nTickets = p->priority;
            }
            break;
        default:
            // end option 1/2 of critical section
            release(&ptable.lock);
            return -1;
    }

    // end option 2/2 of critical section
    release(&ptable.lock);
    return 0;
}

// return number of clock cycles of cpu
int uptime()
{
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

//changed #end

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
    // changed: policies #task2.1
    int chosenTicket = 0;
    int nTotalTickets = 0; // note: _Idan_ avoid divide by zero
    uint xTicks = 0;

    schedp(currentPolicy); // TODO: change this location?
    // changed #end

    for (;;) {
        // Enable interrupts on this processor.
        sti();

        // changed #task2.1

        // count total number of tickets - Critical Section
        acquire(&ptable.lock);
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->state == RUNNABLE) {
                nTotalTickets += p->nTickets;
            }
        }
        release(&ptable.lock);

        xTicks = uptime();
        chosenTicket = random(xTicks, nTotalTickets);
        // changed #end

        // Loop over process table looking for process to run.
        acquire(&ptable.lock);

        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->state != RUNNABLE)
                continue;

            // changed #task2.1
            chosenTicket -= p->nTickets;
            if (chosenTicket > 0 && p->state != RUNNABLE)
                continue;
            // changed #end

            // Switch to chosen process.  It is the process's job
            // to release ptable.lock and then reacquire it
            // before jumping back to us.
            proc = p;
            switchuvm(p);
            p->state = RUNNING;
            swtch(&cpu->scheduler, proc->context);
            switchkvm();

            // Process is done running for now.
            // It should change its p->state before coming back.
            proc = 0;
        }
        release(&ptable.lock);

    }
}

// Enter scheduler.  Must hold only ptable.lock
// and change proc->state.
void
sched(void) {
    int intena;

    if (!holding(&ptable.lock))
        panic("sched ptable.lock");
    if (cpu->ncli != 1)
        panic("sched locks");
    if (proc->state == RUNNING)
        panic("sched running");
    if (readeflags() & FL_IF)
        panic("sched interruptible");
    intena = cpu->intena;
    swtch(&proc->context, cpu->scheduler);
    cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void) {
    acquire(&ptable.lock);  //DOC: yieldlock

    // changed give less 1 ticket to process, dynamic policy #task2.1
    if (currentPolicy == DYNAMIC_POLICY && proc) {
        proc->nTickets--;
        if (proc->nTickets < 1) { proc->nTickets = 1; }
    }
    //changed #end

    proc->state = RUNNABLE;
    sched();
    release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void) {
    static int first = 1;
    // Still holding ptable.lock from scheduler.
    release(&ptable.lock);

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

    if (lk == 0)
        panic("sleep without lk");

    // Must acquire ptable.lock in order to
    // change p->state and then call sched.
    // Once we hold ptable.lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup runs with ptable.lock locked),
    // so it's okay to release lk.
    if (lk != &ptable.lock) {  //DOC: sleeplock0
        acquire(&ptable.lock);  //DOC: sleeplock1
        release(lk);
    }

    // Go to sleep.
    proc->chan = chan;
    proc->state = SLEEPING;
    sched();

    // Tidy up.
    proc->chan = 0;

    // Reacquire original lock.
    if (lk != &ptable.lock) {  //DOC: sleeplock2
        release(&ptable.lock);
        acquire(lk);
    }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan) {
    struct proc *p;

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == SLEEPING && p->chan == chan) {
            p->state = RUNNABLE;
        }

        // changed #task1.2
        if(currentPolicy == DYNAMIC_POLICY && p){
            p->nTickets += 10;
            if (p->nTickets > 100) { p->nTickets = 100; }
        }
        // changed #end
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan) {
    acquire(&ptable.lock);
    wakeup1(chan);
    release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid) {
    struct proc *p;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->killed = 1;
            // Wake process from sleep if necessary.
            if (p->state == SLEEPING)
                p->state = RUNNABLE;
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
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
            getcallerpcs((uint *) p->context->ebp + 2, pc);
            for (i = 0; i < 10 && pc[i] != 0; i++)
                cprintf(" %p", pc[i]);
        }
        cprintf("\n");
    }
}

// changed: adding 'int priority(int)' system call function implementation #task2.1
// Sets the priority of the current process
void priority(int priorityNumber){
    if(!proc) // avoid segfault
        return;

    proc->priority = priorityNumber;
}

// changed: adding 'int wait_stat(int* status, struct perf *)' system call function implementation #task2.2

// private function to retrieve process by its pid
struct proc * getProcessByPid(int pid){
    struct proc *p = 0;
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if(p->pid == pid){
            release(&ptable.lock);
            return p;
        }
    }
    release(&ptable.lock);
    return p;
}

// extracting the process times information and presenting it to the user.
int wait_stat(int *status, struct perf * performance) {
    int pid = -1;
    struct proc *p = 0;
    pid = wait(status);
    if (pid != -1) {
        p = getProcessByPid(pid);
        if (p) {
            performance->cTime = p->cTime;
            performance->tTime = p->tTime;
            performance->sTime = p->sTime;
            performance->reTime = p->reTime;
            performance->ruTime = p->ruTime;

            // TODO delete
            cprintf("kernel: proc.c: performance->tTime = %d\n", performance->tTime);
            cprintf("kernel: proc.c: performance->cTime = %d\n", performance->cTime);
            cprintf("kernel: proc.c: performance->reTime = %d\n", performance->reTime);
            cprintf("kernel: proc.c: performance->ruTime = %d\n", performance->ruTime);
            cprintf("kernel: proc.c: performance->sTime = %d\n", performance->sTime);
        } else {
            return -1; // failure
        }
    }
    return pid;
}

// changed: adding 'void policy(int);' system call function implementation #task2.1

// sets the scheduler policy
void policy(int sched_policy_id){
    schedp(sched_policy_id); // NOTE: ignore schedp return value
}

// changed #end
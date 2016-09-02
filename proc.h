// Segments in proc->gdt.
#define NSEGS     7

//changed: constants #task2.1
#define RANDOM_NUMBER_1 12345685L
#define RANDOM_NUMBER_2 58251321L
#define UNIFORM_POLICY 1
#define PRIORITY_POLICY 2
#define DYNAMIC_POLICY 3
//changed #end

// Per-CPU state
struct cpu {
  uchar id;                    // Local APIC ID; index into cpus[] below
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?

  // Cpu-local storage variables; see below
  struct cpu *cpu;
  struct proc *proc;           // The currently-running process.
};

extern struct cpu cpus[NCPU];
extern int ncpu;

// Per-CPU variables, holding pointers to the
// current cpu and to the current process.
// The asm suffix tells gcc to use "%gs:0" to refer to cpu
// and "%gs:4" to refer to proc.  seginit sets up the
// %gs segment register so that %gs refers to the memory
// holding those two variables in the local cpu's struct cpu.
// This is similar to how thread-local variables are implemented
// in thread libraries such as Linux pthreads.
extern struct cpu *cpu asm("%gs:0");       // &cpus[cpunum()]
extern struct proc *proc asm("%gs:4");     // cpus[cpunum()].proc

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// DONE: __Idan:__ add a field to the process control block PCB ( see proc.h – the proc structure ) in order to save an exit status of the terminated process #task1 #wormUp
// DONE: __Idan:__ Next, you have to change all system calls affected by this change ( i.e., exit and wait ) #task1 #wormUp
// DONE: add field 'nTickets' to proc struct #task2.1



// TODO: Write a user space program called policy which accepts a single argument – the policy identifier. The program must update the scheduling policy accordingly. #task2.1


// Per-process state
struct proc {
    uint sz;                     // Size of process memory (bytes)
    pde_t *pgdir;                // Page table
    char *kstack;                // Bottom of kernel stack for this process
    enum procstate state;        // Process state
    int pid;                     // Process ID
    struct proc *parent;         // Parent process
    struct trapframe *tf;        // Trap frame for current syscall
    struct context *context;     // swtch() here to run process
    void *chan;                  // If non-zero, sleeping on chan
    int killed;                  // If non-zero, have been killed
    struct file *ofile[NOFILE];  // Open files
    struct inode *cwd;           // Current directory
    char name[16];               // Process name (debugging)
    int status;                  // CHANGED: process exit status field #tesk1 #wormUp
    int nTickets;                // CHANGED: number of tickets to process #task2.1
    int priority;                // CHANGED: process's priority for the scheduler with priority policy #task2.1 TODO: is needed?
    int cTime;                   // CHANGED: process creation time #task2.2
    int tTime;                   // CHANGED: process termination time #task2.2
    int sTime;                   // CHANGED: the time the process spent on the SLEEPING state #task2.2
    int reTime;                  // CHANGED: the time the process spent on the READY state #task2.2
    int ruTime;                  // CHANGED: the time the process spent on the RUNNING state #task2.2
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

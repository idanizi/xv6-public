#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void) {
    int i;

    for (i = 0; i < 256; i++) SETGATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0);
    SETGATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);

    initlock(&tickslock, "time");
}

void
idtinit(void) {
    lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void trap(struct trapframe *tf)
{
    if (tf->trapno == T_SYSCALL) {
        if (proc->killed)
            exit(0); // changed
        proc->tf = tf;
        syscall();
        if (proc->killed)
            exit(0); // changed
        return;
    }

    switch (tf->trapno) {
        case T_IRQ0 + IRQ_TIMER:
            if (cpu->id == 0) {
                acquire(&tickslock);
                ticks++;
                // DONE: update proc struct after every clock tick #task2.2
                // changed #task2.2
                if(proc && proc->state == RUNNING){ // avoid segfault
                    proc->ruTime++;
                }
                // changed #end
                wakeup(&ticks);
                release(&tickslock);
            }
            lapiceoi();
            break;
        case T_IRQ0 + IRQ_IDE:
            ideintr();
            lapiceoi();
            break;
        case T_IRQ0 + IRQ_IDE + 1:
            // Bochs generates spurious IDE1 interrupts.
            break;
        case T_IRQ0 + IRQ_KBD:
            kbdintr();
            lapiceoi();
            break;
        case T_IRQ0 + IRQ_COM1:
            uartintr();
            lapiceoi();
            break;
        case T_IRQ0 + 7:
        case T_IRQ0 + IRQ_SPURIOUS:
            cprintf("cpu%d: spurious interrupt at %x:%x\n",
                    cpu->id, tf->cs, tf->eip);
            lapiceoi();
            break;

            //PAGEBREAK: 13
        default:
            if (proc == 0 || (tf->cs & 3) == 0) {
                // In kernel, it must be our mistake.
                cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
                        tf->trapno, cpu->id, tf->eip, rcr2());
                panic("trap");
            }
            // In user space, assume process misbehaved.
            cprintf("pid %d %s: trap %d err %d on cpu %d "
                            "eip 0x%x addr 0x%x--kill proc\n",
                    proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip,
                    rcr2());
            proc->killed = 1;
    }

    // Force process exit if it has been killed and is in user space.
    // (If it is still executing in the kernel, let it keep running
    // until it gets to the regular system call return.)
    if (proc && proc->killed && (tf->cs & 3) == DPL_USER)
        exit(0); // changed

    // Force process to give up CPU on clock tick.
    // If interrupts were on while locks held, would need to check nlock.
    if (proc && proc->state == RUNNING && tf->trapno == T_IRQ0 + IRQ_TIMER)
        yield();

    // Check if the process has been killed since we yielded
    if (proc && proc->killed && (tf->cs & 3) == DPL_USER)
        exit(0); // changed
}

//changed #task3.1
void defaultHandler(int signum){
    cprintf("A signal %d was accepted by process %d\n", signum, proc->pid);
    if (proc)
        proc->isHandled = 0;
}

// changed #task3.4
void genericSignalHandler(struct trapframe *tf){
    int i = 0;
    int signum = 0;
    if(proc && ((proc->tf->cs & 3) == DPL_USER) && proc->pending && !proc->isHandled){
        proc->isHandled = 1;
        for(i = 0; i < NUMSIG; i++){
            if(proc->pending & 1<<i){
                signum = i;
                proc->pending ^= 1<<signum; // toggling bit
            }
        }

        // TODO: to init all the handlers to be default (in allocproc proc.c)
        // TODO: user space program to send signals and test it.

        if((int)proc->handlers[signum] == -1){
            defaultHandler(signum);
        }else{
            proc->btf = *(proc->tf);
            proc->tf->esp -= 4;
            memmove((void*) proc->tf->esp, &signum, 4);
            proc->tf->esp -= 4;
            memmove((void*) proc->tf->esp, &proc->retAddress, 4);
            proc->tf->eip = (uint) proc->handlers[signum];
        }
    }
}

//changed #end

// changed: created for #task2.3
#include "types.h"
#include "stat.h"
#include "user.h"
#include "perf.h"

// CONSTANTS
#define CPU_ONLY 10
#define CPU_ONLY_OFFSET 0
#define BLOCKED_ONLY 10
#define BLOCKED_ONLY_OFFSET 10
#define MIXED 10
#define MIXED_OFFSET 20
#define PROCESSES_SIZE CPU_ONLY + BLOCKED_ONLY + MIXED


// linked list for all active processes
//struct pNode{
//    int pid;
//    struct pNode *next;
//};

int areChildrenRunning() { // TODO: implement

}

// user mode program to test OS performance
int main(int argc, char **argv) {
    int i = 0;
    int childPid = 0;
//    struct pNode *firstChildProcess = 0;
    int processes[CPU_ONLY + BLOCKED_ONLY + MIXED];
    struct perf *performance = 0;

    // TODO: fork 30 child processes: 10 processes of each of the following kinds:

    /*
     * TODO: CPU only: the processes will perform a cpu-only time-consuming computation (no blocking
     * system calls are allowed). This computation must take at least 30 ticks (you can use the uptime
     * system call to check if a tick passed).
     * */
    for (i = 0; i < CPU_ONLY; i++) {
        if ((processes[i + CPU_ONLY_OFFSET] = fork()) == 0) {
            // TODO: child code for cpu only
        }
    }

    /*
     * TODO: Blocking only: the processes will perform 30 sequential calls to the sleep system call (each of a
     * single tick).
     */
    for (i = 0; i < BLOCKED_ONLY; i++) {
        if ((processes[i + BLOCKED_ONLY_OFFSET] = fork()) == 0) {
            // TODO: child code for blocking only
        }
    }

    /*
     * TODO: Mixed: the processes will perform 5 sequential iterations of the following steps - cpu-only
     * computation for 5 ticks followed by system call sleep of a single tick.
     */
    for (i = 0; i < MIXED; i++) {
        if ((processes[i + MIXED_OFFSET] = fork()) == 0) {
            // TODO: child code for mixed
        }
    }
    /* TODO: The parent process will wait until all its children exit.
     * For every finished child, the parent process must print the waiting time, running time and turnaround time of each child.
     * In addition averages for these measures must be printed.
     */

    while (areChildrenRunning()) {
        childPid = wait_stat(0, performance);
        if (childPid != -1 && performance) {
            // TODO: complete
        }
    }

    exit(0);
}
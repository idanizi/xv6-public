// changed: created for #task2.3
#include "types.h"
#include "stat.h"
#include "user.h"
#include "perf.h"
#include "spinlock.h"

// CONSTANTS
#define CPU_ONLY 10
#define CPU_ONLY_OFFSET 0
#define BLOCKED_ONLY 10
#define BLOCKED_ONLY_OFFSET 10
#define MIXED 10
#define MIXED_OFFSET 20
#define PROCESSES_SIZE CPU_ONLY + BLOCKED_ONLY + MIXED

// user mode program to test OS performance
int main(int argc, char **argv) {

    printf(1, "in sanity\n");

    int i = 0, j;
    int childPid = 0;
    int waitingAverage = 0;
    int runningAverage = 0;
    int turnaroundAverage = 0;
    int xTicks = 0;
    int status = 0;

    struct perf *performance;
    performance = (struct perf *) malloc(sizeof(struct perf));
    performance->sTime = 1;
    performance->cTime = 2;
    performance->reTime = 3;
    performance->ruTime = 4;
    performance->tTime = 5;

    // TODO: fork 30 child processes: 10 processes of each of the following kinds:
    /*
     * CPU only: the processes will perform a cpu-only time-consuming computation (no blocking
     * system calls are allowed). This computation must take at least 30 ticks (you can use the uptime
     * system call to check if a tick passed).
     * */
    for (i = 0; i < CPU_ONLY; i++) {
        xTicks = uptime();
        childPid = fork();
        if (childPid == 0) {
            // DONE: child code for cpu only
            while (uptime() - xTicks < 30) {}
            exit(0);
        }
    }

    /*
     * TODO: Blocking only: the processes will perform 30 sequential calls to the sleep system call (each of a
     * single tick).
     */
    for (i = 0; i < BLOCKED_ONLY; i++) {
        childPid = fork();
        if (childPid == 0) {
            // TODO: child code for blocking only
            for (j = 0; j < 30; j++) {
                sleep(1);
            }
            exit(0);
        }
    }

    /*
     * TODO: Mixed: the processes will perform 5 sequential iterations of the following steps - cpu-only
     * computation for 5 ticks followed by system call sleep of a single tick.
     */
    for (i = 0; i < MIXED; i++) {
        xTicks = uptime();
        childPid = fork();
        if (childPid == 0) {
            // TODO: child code for mixed
            for (j = 0; j < 5; j++) {
                while (uptime() - xTicks < 5) {}
                sleep(1);
            }
            exit(0);
        }
    }
    /* DONE: The parent process will wait until all its children exit.
     * For every finished child, the parent process must print the waiting time, running time and turnaround time of each child.
     * In addition averages for these measures must be printed.
     */

    // parent waiting for a child process to end
    while ((childPid = wait_stat(&status, performance)) > 0) {
        if (performance) {
            printf(1, "--- performance->sTime: %d\n", performance->sTime);
            printf(1, "--- performance->tTime: %d\n", performance->tTime);
            printf(1, "--- performance->ruTime: %d\n", performance->ruTime);
            printf(1, "--- performance->reTime: %d\n", performance->reTime);
            printf(1, "--- performance->cTime: %d\n", performance->cTime);
            printf(1, "Child pid: %d\n", childPid);
            printf(1, "> waiting time (SLEEPING + RUNNABLE): %d\n", (performance->sTime + performance->reTime));
            printf(1, "> running time (RUNNING): %d\n", performance->ruTime);
            printf(1, "> turnaround time (ZOMBIE - EMBRYO): %d\n", (performance->tTime - performance->cTime));
            waitingAverage += performance->sTime + performance->reTime;
            runningAverage += performance->ruTime;
            turnaroundAverage += performance->tTime - performance->cTime;
        }
    }

    // calculating averages
    waitingAverage /= PROCESSES_SIZE;
    runningAverage /= PROCESSES_SIZE;
    turnaroundAverage /= PROCESSES_SIZE;

    // printing results to user
    printf(1, "waiting average: %d\n", waitingAverage);
    printf(1, "running average: %d\n", runningAverage);
    printf(1, "turnaround average: %d\n", turnaroundAverage);

    // memory free
    if (performance)
        free(performance);
//    if (processList)
//        freeList(processList);

    exit(0);
}
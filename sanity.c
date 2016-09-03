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
struct pNode {
    int pid;
    struct pNode *next;
};

int areChildrenRunning(struct pNode *node) {
    while (node) {
        return 1;
    }
    return 0;
}

// remove child process from process list
void removeChildFromRunningList(int pid, struct pNode *node) {
    struct pNode *nodeToDelete = 0;
    struct pNode *prev = node;
    while (node) {
        if (node->pid == pid) {
            nodeToDelete = node;
            prev->next = node->next;
            if (nodeToDelete)
                free(nodeToDelete);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void addChildToRunningList(int childPid, struct pNode *node) {
    while (node && node->pid && node->next) {
        node = node->next;
    }
    node->next = (struct pNode *)malloc(sizeof(struct pNode));
}

// recursive private function to free node list
void freeList(struct pNode *node) {
    if (!node->next) {
        free(node);
        return;
    }
    freeList(node);
}

// private constructor
struct pNode *newProcessList() {
    struct pNode *processList = (struct pNode *) malloc(sizeof(struct pNode));
    processList->pid = 0;
    processList->next = 0;
    return processList;
}

// user mode program to test OS performance
int main(int argc, char **argv) {

    printf(1, "in sanity\n");

    int i = 0;
    int childPid = 0;
    int waitingAverage = 0;
    int runningAverage = 0;
    int turnaroundAverage = 0;
    int xTicks = 0;

    struct pNode *processList = 0;
    processList = newProcessList();

    struct perf *performance;
    performance = (struct perf *) malloc(sizeof(struct pref *));

    // TODO: fork 30 child processes: 10 processes of each of the following kinds:

    /*
     * CPU only: the processes will perform a cpu-only time-consuming computation (no blocking
     * system calls are allowed). This computation must take at least 30 ticks (you can use the uptime
     * system call to check if a tick passed).
     * */
    for (i = 0; i < CPU_ONLY; i++) {
        if ((childPid = fork()) == 0) {
            // DONE: child code for cpu only
            xTicks = uptime();
            atoi("123");
            atoi("456");
            atoi("789");
            atoi("789");
            atoi("789");
            atoi("789");
            atoi("789");
            atoi("789");
            atoi("789");
            atoi("789");
            atoi("789");
            atoi("789");
            xTicks = uptime() - xTicks;
        }
        addChildToRunningList(childPid, processList);
    }

    printf(1, "checking... xTicks: %d\n", xTicks);

    /*
     * TODO: Blocking only: the processes will perform 30 sequential calls to the sleep system call (each of a
     * single tick).
     */
    for (i = 0; i < BLOCKED_ONLY; i++) {
//        if ((processes[i + BLOCKED_ONLY_OFFSET] = fork()) == 0) {
//            // TODO: child code for blocking only
//        }
    }

    /*
     * TODO: Mixed: the processes will perform 5 sequential iterations of the following steps - cpu-only
     * computation for 5 ticks followed by system call sleep of a single tick.
     */
    for (i = 0; i < MIXED; i++) {
//        if ((processes[i + MIXED_OFFSET] = fork()) == 0) {
//            // TODO: child code for mixed
//        }
    }
    /* DONE: The parent process will wait until all its children exit.
     * For every finished child, the parent process must print the waiting time, running time and turnaround time of each child.
     * In addition averages for these measures must be printed.
     */

    while (areChildrenRunning(processList)) {
        childPid = wait_stat(0, performance); // parent waiting for one child process to end
        removeChildFromRunningList(childPid, processList);
        if (childPid != -1 && performance) {
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
    if (processList)
        freeList(processList);

    exit(0);
}
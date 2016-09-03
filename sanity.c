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

// linked list for all active processes
struct pNode {
    int pid;
    struct pNode *next;
};

int areChildrenRunning(struct pNode *node) {
    if (node && node->pid) {
        return 1;
    }
    return 0;
}

// remove child process from process list
void removeChildFromRunningList(int pid, struct pNode *first) {
    struct pNode *node = first;
    struct pNode *nodeToDelete = 0;
    struct pNode *prev = node;
    while (node->next) {
        if (node->pid == pid) {
            nodeToDelete = node;
            if (prev->next)
                prev->next = node->next;
            if (nodeToDelete)
                free(nodeToDelete);
            return;
        }
        prev = node;
        node = node->next;
    }
}

// private add to end of list
void addChildToRunningList(int childPid, struct pNode *node) {
    while (node && node->pid && node->next) {
        node = node->next;
    }
    if (node->pid == 0) { // first dummy node to overwrite
        node->pid = childPid;
    } else { // end of list
        node->next = (struct pNode *) malloc(sizeof(struct pNode));
        node = node->next;
        node->pid = childPid;
        node->next = 0;
    }
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

void listTest(struct pNode *list) {
    struct pNode *node = list;
    int num = 0;

    printf(1, "testing list\n\n");
    printf(1, "first pid: %d\n", node->pid);
    printf(1, "first next: %d\n", node->next);
    printf(1, "adding to list\n");
    addChildToRunningList(1, list);
    node = list;
    num = 0;
    while (node) {
        num++;
        node = node->next;
    }
    printf(1, "list count: %d\n", num);
    printf(1, "removing from list\n");
    removeChildFromRunningList(1, list);
    node = list;
    num = 0;
    while (node) {
        num++;
        node = node->next;
    }
    printf(1, "list count: %d\n", num);
    printf(1, "end testing list\n\n");
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
    int status = 0;

//    struct pNode *processList = 0;
//    processList = newProcessList();

    struct perf *performance;
    performance = (struct perf *) malloc(sizeof(struct pref *));

    /////////// test list ////////////
//    listTest(processList);


    // TODO: fork 30 child processes: 10 processes of each of the following kinds:
    /*
     * CPU only: the processes will perform a cpu-only time-consuming computation (no blocking
     * system calls are allowed). This computation must take at least 30 ticks (you can use the uptime
     * system call to check if a tick passed).
     * */
    for (i = 0; i < CPU_ONLY; i++) {
        childPid = fork();
        if (childPid == 0) {
            // DONE: child code for cpu only
            xTicks = uptime();
            printf(1, "this is child %d\n", i);
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
            printf(1, "checking... xTicks: %d\n", xTicks);
//            removeChildFromRunningList(getpid(), processList);
            exit(0);
        }
        printf(1, "this is parent %d\n", i);
//        addChildToRunningList(childPid, processList);
    }

    /*
     * TODO: Blocking only: the processes will perform 30 sequential calls to the sleep system call (each of a
     * single tick).
     */
//    for (i = 0; i < BLOCKED_ONLY; i++) {
//        // TODO: child code for blocking only
//    }

    /*
     * TODO: Mixed: the processes will perform 5 sequential iterations of the following steps - cpu-only
     * computation for 5 ticks followed by system call sleep of a single tick.
     */
//    for (i = 0; i < MIXED; i++) {
//        // TODO: child code for mixed
//    }
    /* DONE: The parent process will wait until all its children exit.
     * For every finished child, the parent process must print the waiting time, running time and turnaround time of each child.
     * In addition averages for these measures must be printed.
     */

    while ((childPid = wait_stat(&status, performance)) != -1) {
        printf(1, "test1\n");
         // parent waiting for one child process to end
        printf(1, "test2\n");
        printf(1, "test3\n");
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
//    if (processList)
//        freeList(processList);

    exit(0);
}
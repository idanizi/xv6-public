// changed #task2.2
// Created by idan on 9/20/16.

#include "types.h"
#include "stat.h"
#include "user.h"

// CONSTANTS
#define NSTATE 15
#define USTACK 4000

// red
#define ACTIVE 1
// white
#define QUIESCENT 0
// blue
#define FIRING 15

//#define PURPLE 2
//#define AQUA 3
//#define GRASS 4
//#define BROWN 5
//#define DARK_BLUE 6
//#define GREEN 7
//#define YELLOW 8
//#define NAVY 9
//#define PINK 10
//#define VIOLATE 11
//#define VIOLATE 12
//#define VIOLATE 13
//#define VIOLATE 14





// structs
struct soldier {
    struct soldier *leftNeighbor;
    struct soldier *rightNeighbor;
    int state;
    int futureState;
    int id;
    int tid;

    void *(*func)();

    void *stack;
};

// global variant
int n;
struct soldier *squad;
int mutex;
int cycle;
int visitors;

void done(){
    kthread_mutex_lock(mutex);
    cycle++;
    kthread_mutex_dealloc(mutex);
}

int areAllDone(){
    int ans = 0;
    kthread_mutex_lock(mutex);
    visitors++;
    ans = (cycle == n);
    if(ans){
        cycle = 0;
        print(squad);
    }
    kthread_mutex_dealloc(mutex);
    return ans;
}

struct soldier *getSoldierByTid(int tid) {
    struct soldier *s;
    for (s = squad; s < &squad[n]; s++) {
        if (s->tid == tid)
            return s;
    }
    return 0;
}

// for threads
void regularSoldierTrans() {// todo implement

    int tid = kthread_id();
    struct soldier *s = getSoldierByTid(tid);

    if (!s) {
        printf(1, "Error: no soldier holds tid=%d", tid);
        kthread_exit();
    }

    while (s->state != FIRING) {
        switch (s->state) {
            case QUIESCENT:
                if (s->leftNeighbor == QUIESCENT && s->rightNeighbor == QUIESCENT)
                    s->futureState = QUIESCENT;
                else
                break;
            default:
                while (s->futureState != FIRING) {
                    s->futureState = (s->state + 1) % NSTATE;
                }
                break;
        }
        s->state = s->futureState;
        done();
        while (!areAllDone()){
            sleep(10);
        };
    }

    kthread_exit();
}

void generalTrans() {// todo implement

    int tid = kthread_id();
    struct soldier *s = getSoldierByTid(tid);

    if (!s) {
        printf(1, "Error: no soldier holds tid=%d", tid);
        kthread_exit();
    }

    while (s->state != FIRING) { // todo
    }

    kthread_exit();
}

void firstSoldierTrans() {// todo implement

    int tid = kthread_id();
    struct soldier *s = getSoldierByTid(tid);

    if (!s) {
        printf(1, "Error: no soldier holds tid=%d", tid);
        kthread_exit();
    }

    while (s->state != FIRING) { // todo
    }

    kthread_exit();
}

void print(struct soldier *squad) {
    struct soldier *s = 0;
    for (s = squad; s < &squad[n]; s++) {
        printf(1, "%X ", s->state);
    }
    printf(1, "\n");
}

int main(int argc, char **argv) {

    n = 0;
    squad = 0;
    cycle = 0;
    visitors = 0;

    mutex = kthread_mutex_alloc();

    if (argc < 2) {
        printf(1, "usage: fssp <n>\n");
        exit();
    }

    // initiating by parameters
    n = atoi(argv[1]);
    squad = (struct soldier *) malloc(n * sizeof(struct soldier));

    // init soldiers
    struct soldier *s = 0;
    int i = 0;
    int general = 0;
    int first = 0;
    for (s = squad; s < &squad[n]; s++, i++) {

        // initiating to zeros
        s->state = QUIESCENT;
        s->id = i;
        s->leftNeighbor = 0;
        s->rightNeighbor = 0;
        s->stack = 0;
        s->futureState = 0;

        // setting up by position
        first = i == 0;
        general = i == n - 1;
        if (!first) // not first soldier
            s->leftNeighbor = squad + (i - 1);
        else
            s->func = (void *) &firstSoldierTrans;
        if (!general) // not General
            s->rightNeighbor = squad + (i + 1);
        else {
            s->func = (void *) &generalTrans;
            s->state = ACTIVE;
        }
        if (!general && !first) // regular soldier
            s->func = (void *) &regularSoldierTrans;

        // memo allocation
        stack = (void *) malloc(USTACK);

        // creating thread for this soldier
        s->tid = kthread_create(s->func, s->stack, USTACK);
    }


//    print(squad);

    // wait for all to finish
    for (s = squad; s < &squad[n]; s++) {
        kthread_join(s->tid);
    }

    if (kthread_mutex_dealloc(mutex) < 0)
        printf(1, "Error: mutex dealloc failed!\n");

    if (squad) {
        free(squad);
    }
    exit();
}

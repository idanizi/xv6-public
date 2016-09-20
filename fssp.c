// changed #task2.2
// Created by idan on 9/20/16.

#include "types.h"
#include "stat.h"
#include "user.h"

// CONSTANTS
#define USTACK 4000

#define NSTATE 6
#define Q 'Q'
#define Z 'Z'
#define P 'P'
#define M 'M'
#define R 'R'
#define F 'F'

// structs

struct semaphore {
    int s1;
    int s2;
    int value;
    int wake;
};

void semaphore_init(struct semaphore *sm, int value) {
    sm->s1 = kthread_mutex_alloc();
    sm->s2 = kthread_mutex_alloc();
    sm->value = value;
    sm->wake = 0;
}

int semaphore_delete(struct semaphore *sm) {
    if (kthread_mutex_dealloc(sm->s1) < 0)
        return -1;
    if (kthread_mutex_dealloc(sm->s2) < 0)
        return -1;
    return 0;
}

void semaphore_down(struct semaphore *sm) {
    kthread_mutex_lock(sm->s1);
//    printf(1, "tid=%d: semaphore_down\n", kthread_id()); // todo del
    sm->value--;
    if (sm->value < 0) {
        kthread_mutex_unlock(sm->s1);
        kthread_mutex_lock(sm->s2);
        kthread_mutex_lock(sm->s1);
        sm->wake--;
        if (sm->wake > 0) {
            kthread_mutex_unlock(sm->s2);
        }
    }
    kthread_mutex_unlock(sm->s1);
}

void semaphore_up(struct semaphore *sm) {
    kthread_mutex_lock(sm->s1);
//    printf(1, "tid=%d: semaphore_up\n", kthread_id()); // todo del
    sm->value++;
    if (sm->value <= 0) {
        sm->wake++;
        if (sm->wake == 1) {
            kthread_mutex_unlock(sm->s2);
        }
    }
    kthread_mutex_unlock(sm->s1);
}

struct soldier {
    struct soldier *leftNeighbor;
    struct soldier *rightNeighbor;
    char state;
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
struct semaphore start;
struct semaphore end;
int start_cycle;
int start_full;
int end_cycle;
int end_full;

// Algorithm Database
char _Q[NSTATE][NSTATE] = {
        {Q, P, Q, Q, 0, Q},
        {P, P, 0, 0, 0, P},
        {Q, 0, Q, 0, 0, 0},
        {Q, 0, 0, Q, 0, Q},
        {0, 0, 0, 0, 0, 0},
        {Q, P, Q, Q, 0, 0}
};

char _Z[NSTATE][NSTATE] = {
        {0, 0, Q, P, Q, 0},
        {0, Z, 0, Z, 0, 0},
        {Q, 0, Q, Q, 0, Q},
        {P, Z, Q, F, Q, F},
        {Q, 0, 0, Q, Q, Q},
        {0, Z, Q, F, Q, 0}
};

char _P[NSTATE][NSTATE] = {
        {Z, Z, R, R, 0, 0},
        {Z, 0, Z, Z, 0, 0},
        {R, Z, Z, 0, 0, Z},
        {R, Z, 0, Z, 0, Z},
        {0, 0, 0, 0, 0, 0},
        {Z, 0, Z, Z, 0, 0}
};

char _M[NSTATE][NSTATE] = {
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, R, Z, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0}
};

char _R[NSTATE][NSTATE] = {
        {0, 0, R, P, Z, 0},
        {0, 0, M, R, M, 0},
        {R, M, 0, 0, M, 0},
        {P, R, 0, 0, R, 0},
        {Z, M, M, R, M, 0},
        {0, 0, 0, 0, 0, 0}
};

int stateToIndex(char state) {
    switch (state) {
        case F:
            printf(1, "Error: stateToIndex: looking for state F\n");
            return -1;
        case Q:
            return 0;
        case P:
            return 1;
        case R:
            return 2;
        case Z:
            return 3;
        case M:
            return 4;
        default:
            return 5;
    }
}

char calcFutureState(char self, char left, char right) {
    switch (self) {
        case F:
            return F;
        case Q:
            return _Q[stateToIndex(left)][stateToIndex(right)];
        case P:
            return _P[stateToIndex(left)][stateToIndex(right)];
        case R:
            return _R[stateToIndex(left)][stateToIndex(right)];
        case Z:
            return _Z[stateToIndex(left)][stateToIndex(right)];
        case M:
            return _M[stateToIndex(left)][stateToIndex(right)];
        default:
            printf(1, "Error: calcFutureState - illegal state\n");
            return (char) -1;
    }
}

void print(struct soldier *squad) {
    struct soldier *s = 0;
    for (s = squad; s < &squad[n]; s++) {
        printf(1, "%c ", s->state);
    }
    printf(1, "\n");
}

void start_round() {
    kthread_mutex_lock(mutex);
    start_cycle++;
    kthread_mutex_unlock(mutex);
    semaphore_down(&start);
}

void end_round() {
    kthread_mutex_lock(mutex);
    end_cycle++;
    kthread_mutex_unlock(mutex);
    semaphore_down(&end);
}

int areEverybodyGotToBarrier() {
    int ans = 0;
    kthread_mutex_lock(mutex);
    ans = (start_cycle == n);
    if(ans){
        start_full = 1;
        start_cycle = 0;
        kthread_mutex_unlock(mutex);
        return ans;
    }
    ans = (end_cycle == n);
    if (ans) {
        end_full = 1;
        end_cycle = 0;
        kthread_mutex_unlock(mutex);
        return ans;
    }
    kthread_mutex_unlock(mutex);
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
void regularSoldierTrans() {

    int tid = kthread_id();
    struct soldier *s = getSoldierByTid(tid);

    if (!s) {
        printf(1, "Error: no soldier holds tid=%d", tid);
        kthread_exit();
    }

    while (s->state != F) {

        // start - blocking
        start_round();
        s->futureState = calcFutureState(s->state, s->leftNeighbor->state, s->rightNeighbor->state);
        // end - blocking
        end_round();
        s->state = s->futureState;

    }

    kthread_exit();
}

void generalTrans() {

    int tid = kthread_id();
    struct soldier *s = getSoldierByTid(tid);

    if (!s) {
        printf(1, "Error: no soldier holds tid=%d", tid);
        kthread_exit();
    }

    while (s->state != F) {
        // start - blocking
        start_round();
        s->futureState = calcFutureState(s->state, s->leftNeighbor->state, 0);
        // end - blocking
        end_round();
        s->state = s->futureState;
    }

    kthread_exit();
}

void firstSoldierTrans() {

    int tid = kthread_id();
    struct soldier *s = getSoldierByTid(tid);

    if (!s) {
        printf(1, "Error: no soldier holds tid=%d", tid);
        kthread_exit();
    }

    while (s->state != F) {
        // start - blocking
        start_round();
        s->futureState = calcFutureState(s->state, 0, s->rightNeighbor->state);
        // end - blocking
        end_round();
        s->state = s->futureState;
    }

    kthread_exit();
}

int areEverybodyFiring(struct soldier *squad) {
    struct soldier *s = 0;
    kthread_mutex_lock(mutex);
    for (s = squad; s < &squad[n]; s++) {
        if(s->state != F) {
            kthread_mutex_unlock(mutex);
            return 0;
        }
    }
    kthread_mutex_unlock(mutex);
    return 1;
}

int main(int argc, char **argv) {

    n = 0;
    squad = 0;
    start_cycle = 0;
    start_full = 0;
    end_cycle = 0;
    end_full = 0;
    int finish = 0;

    mutex = kthread_mutex_alloc();

    if (argc < 2) {
        printf(1, "usage: fssp <n>\n");
        exit();
    }

    // initiating by parameters
    n = atoi(argv[1]);
    squad = (struct soldier *) malloc(n * sizeof(struct soldier));
    semaphore_init(&start, n);
    semaphore_init(&end, 0);

    // init soldiers
    struct soldier *s = 0;
    int i = 0;
    int general = 0;
    int first = 0;
    for (s = squad; s < &squad[n]; s++, i++) {

        // initiating to zeros
        s->state = Q;
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
            s->state = P;
        }
        if (!general && !first) // regular soldier
            s->func = (void *) &regularSoldierTrans;

        // memo allocation
        s->stack = (void *) malloc(USTACK);

        // creating thread for this soldier
        s->tid = kthread_create(s->func, s->stack, USTACK);
    }

    // print initial state
    print(squad);

    // main process looking after threads and release them from barrier all together
    while (!finish) {
        while (!areEverybodyGotToBarrier()) {
            sleep(10);
        }

        // print current state
        print(squad);

        if (start_full) {
            start_full = 0;
            for (i = 0; i < n; i++) {
                semaphore_up(&start);
            }
        } else if (end_full) {
            end_full = 0;
            for (i = 0; i < n; i++) {
                semaphore_up(&end);
            }
            finish = areEverybodyFiring(squad);
        } else {
            printf(1, "Error: got to barrier without full\n");
        }
    }

    // wait for all to finish
    for (s = squad; s < &squad[n]; s++) {
        kthread_join(s->tid);
    }

    if (semaphore_delete(&start) < 0 || semaphore_delete(&end) < 0) {
        printf(1, "Error: semaphore delete\n");
    }

    if (kthread_mutex_dealloc(mutex) < 0)
        printf(1, "Error: mutex dealloc failed!\n");

    for(s = squad; s < &squad[n]; s++){
        if(s->stack)
            free(s->stack);
    }

    if (squad) {
        free(squad);
    }
    exit();
}

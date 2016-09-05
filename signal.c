// changed: created for #task3.4
#include "types.h"
#include "stat.h"
#include "user.h"

void mySignalHandler1(int signum) {
    printf(1, "@@@@@ my signal handler test 1 @@@@@\n");
    printf(1, "curr pid: %d, signum: %d\n", getpid(), signum);
    printf(1, "@@@@@           END            @@@@@\n\n");
}

void mySignalHandler2(int signum) {
    printf(1, "@@@@@ my signal handler test 2 @@@@@\n");
    printf(1, "curr pid: %d, signum: %d\n", getpid(), signum);
    printf(1, "@@@@@           END            @@@@@\n\n");
}

void mySignalHandler3(int signum) {
    printf(1, "@@@@@ my signal handler test 3 @@@@@\n");
    printf(1, "curr pid: %d, signum: %d\n", getpid(), signum);
    printf(1, "@@@@@           END            @@@@@\n\n");
}

void mySignalHandler4(int signum) {
    printf(1, "@@@@@ my signal handler test 4 @@@@@\n");
    printf(1, "curr pid: %d, signum: %d\n", getpid(), signum);
    printf(1, "@@@@@           END            @@@@@\n\n");
}

// user space program to test the signal framework
int main(int argc, char **argv) {
//    int parent = 0, child = 0;
//    int status, i;
//    parent = getpid();
//    int pid = 0;
//
//
//    for (i = 0; i < 4; i++) {
//        child = fork();
//        if (child == 0) {
//            // child code
//            signal(1, mySignalHandler1);
//            signal(2, mySignalHandler2);
//            signal(3, mySignalHandler3);
//            sigsend(parent, i);
//            exit(0);
//        }
//        pid = wait(&status);
//        if (pid != -1)
//            printf(1, "child %d dead\n", pid);
//    }

    int pid = getpid();
    printf(1, ">>>>>>>>> PARENT PID: %d\n", pid);

    signal(4, mySignalHandler1);
    printf(1, "signal(4, mySignalHandler1);\n");

    signal(5, mySignalHandler2);
    printf(1, "signal(5, mySignalHandler2);\n");

    signal(6, mySignalHandler3);
    printf(1, "signal(6, mySignalHandler3);\n");

    signal(7, mySignalHandler4);
    printf(1, "signal(7, mySignalHandler4);\n");


    if (fork() == 0) {

        sigsend(pid, 4);
        sigsend(pid, 5);
        sigsend(pid, 6);
        sigsend(pid, 7);

        if (fork() == 0) {

            sigsend(pid, 4);
            sigsend(pid, 5);
            sigsend(pid, 6);
            sigsend(pid, 7);
            exit(0);
        }
        wait(0);
        exit(0);
    }

    wait(0);


//    if (x == 0 && y == 0) {
//        printf(1, "SUCCESS!\n");
//    } else {
//        printf(1, "x=%d y=%d\n", x, y);
//    }

//    while (pid != -1) {
//        pid = wait(&status);
//        if (pid != -1)
//            printf(1, "child %d dead\n", pid);
//    }

    exit(0);
}
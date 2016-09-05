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
    int pid = getpid();
    printf(1, ">>>>>>>>> PARENT PID: %d\n", pid);

    signal(1, mySignalHandler1);
    printf(1, "signal(1, mySignalHandler1);\n");

    signal(2, mySignalHandler2);
    printf(1, "signal(2, mySignalHandler2);\n");

    signal(3, mySignalHandler3);
    printf(1, "signal(3, mySignalHandler3);\n");

    signal(4, mySignalHandler4);
    printf(1, "signal(4, mySignalHandler4);\n");

    signal(5, mySignalHandler4);
    printf(1, "signal(5, mySignalHandler5);\n");


    if (fork() == 0) {

        sigsend(pid, 1);
        sigsend(pid, 2);
        sigsend(pid, 3);
        sigsend(pid, 4);
        sigsend(pid, 5);

        if (fork() == 0) {

            sigsend(pid, 1);
            sigsend(pid, 2);
            sigsend(pid, 3);
            sigsend(pid, 4);
            sigsend(pid, 5);
            exit(0);
        }
        wait(0);
        exit(0);
    }

    wait(0);
    exit(0);
}
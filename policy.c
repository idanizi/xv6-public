// changed: created for #task2.1
#include "types.h"
#include "stat.h"
#include "user.h"

// user mode program to change its process priority
int main(int argc, char **argv) {
    if (argc < 2) {
        printf(2, "usage: policy <1 = UNIFORM / 2 = PRIORITY / 3 = DYNAMIC>\n");
    }

    // system call
    policy(atoi(argv[1]));

    exit(0);
}
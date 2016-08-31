// changed: created for #task2.1
#include "types.h"
#include "stat.h"
#include "user.h"

// user mode program to change its process priority
int main(int argc, char **argv) {
    if (argc < 2) {
        printf(2, "usage: priority <new priority>\n");
    }

    // system call
    priority(atoi(argv[1]));

    exit(0);
}


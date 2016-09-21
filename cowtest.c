#include "types.h"
#include "stat.h"
#include "user.h"

void waitTime(char *message, int *x) {
    printf(1, "%s | X=%d\n", message, *x);
    printf(1, "Please press CTRL+P...\n");
    sleep(1000);
}

int
main(int argc, char *argv[]) {
    int *mem = (int *) malloc(sizeof(int));
    *mem = 1234;
    printf(1, "before fork: mem=0x%x, *mem=%d\n", mem, *mem);
    printf(1, "----------> press CTRL+P...\n");
    sleep(1000);
    if (fork() == 0) {
        printf(1, "child pid%d: mem=0x%x, *mem=%d\n", getpid(), mem, *mem);
        printf(1, "----------> press CTRL+P...\n");
        sleep(1000);

        *mem = 5555;

        printf(1, "child pid%d: after writing to mem, cow should work: mem=0x%x, *mem=%d\n", getpid(), mem, *mem);
        printf(1, "----------> press CTRL+P...\n");
        sleep(1000);

        printf(1, "child pid%d: expecting = %d, result = %d\n", getpid(), 5555, *mem);
        if(mem){
            free(mem);
            mem = 0;
        }
        exit();
    } else {
        wait();
    }
    printf(1, "parent pid%d: expecting = %d, result = %d\n", getpid(), 1234, *mem);
    if (mem) {
        free(mem);
        mem = 0;
    }
    exit();
}

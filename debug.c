// changed created for user space program to call system call debug mode to change debug printing mode
// Created by idan on 9/17/16.

#include "types.h"
#include "stat.h"
#include "user.h"
#include "syscall.h"
#include "param.h"

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf(1, "usage: debug <mode_number>\n");
        exit();
    }

    debug(argv[1]);
    exit();
}


// CHANGED: created for user space programs expose information from the kernel. #task2.2
struct perf {
    int cTime;                   // process creation time
    int tTime;                   // process termination time
    int sTime;                   // the time the process spent on the SLEEPING state
    int reTime;                  // the time the process spent on the READY state
    int ruTime;                  // the time the process spent on the RUNNING state
};
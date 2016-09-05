struct stat;
struct rtcdate;
struct perf;

// system calls
int fork(void);
int exit(int status) __attribute__((noreturn)); // CHANGED
int wait(int *status); // CHANGED
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
void priority(int); // changed #task2.1
int wait_stat(int *status, struct perf *); // changed #task2.2
void policy(int); // changed #task2.1
sighandler_t signal(int signum, sighandler_t handler); // changed #task3.2
int sigsend(int pid, int signum); // changed #task3.3
int sigreturn(void); // changed #task3.4

// ulib.c
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

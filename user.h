struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
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
int kthread_create(void *(*start_func)(), void *stack, int stack_size); // changed #task1.2
int kthread_id(void); // changed #task1.2
void kthread_exit(); // changed #task1.2
int kthread_join(int thread_id); // changed #task1.2
void debug(int mode); // changed
int kthread_mutex_alloc(); // changed #task2.1
int kthread_mutex_dealloc(int mutex_id); // changed #task2.1
int kthread_mutex_lock(int mutex_id); // changed #task2.1
int kthread_mutex_unlock(int mutex_id); // changed #task2.1

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

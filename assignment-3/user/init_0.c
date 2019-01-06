#include<init.h>
static void exit(int);
static int main(void);


void init_start()
{
    int retval = main();
    exit(0);
}

/*Invoke system call with no additional arguments*/
static long _syscall0(int syscall_num)
{
    asm volatile ("int $0x80;"
                  "leaveq;"
                  "retq;"
                  :::"memory");
    return 0;   /*gcc shutup!*/
}

/*Invoke system call with one argument*/

static long _syscall1(int syscall_num, int exit_code)
{
    asm volatile ("int $0x80;"
                  "leaveq;"
                  "retq;"
                  :::"memory");
    return 0;   /*gcc shutup!*/
}
/*Invoke system call with two arguments*/

static long _syscall2(int syscall_num, u64 arg1, u64 arg2)
{
    asm volatile ("int $0x80;"
                  "leaveq;"
                  "retq;"
                  :::"memory");
    return 0;   /*gcc shutup!*/
}

static void exit(int code)
{
    _syscall1(SYSCALL_EXIT, code);
}

static void alarm(int ticks)
{
    _syscall1(SYSCALL_ALARM, ticks);
}

static void reg_handler(int signo, void* handler)
{
    _syscall2(SYSCALL_SIGNAL, signo, (unsigned long) handler);
}

static long getpid()
{
    return(_syscall0(SYSCALL_GETPID));
}

static long write(char *ptr, int size)
{
    return(_syscall2(SYSCALL_WRITE, (u64)ptr, size));
}

static void sleep(int ticks)
{
    _syscall1(SYSCALL_SLEEP, ticks);
}

static void clone(void *th_func, void *user_stack)
{
    _syscall2(SYSCALL_CLONE, (u64) th_func, (u64) user_stack);
}

int main()
{
    void alarm_handler()
    {
        write("ping!!\n", 7);
    }

    reg_handler(SIGALRM, alarm_handler);
    unsigned long i, j;
    unsigned long buff[4096];
    i = getpid();
    write("alarm\n", 6);
    alarm(3);

    sleep(4);
    write("woke up\n", 8);

    sleep(6);
    write("woke up\n", 8);

    for(i = 0; i < 4096; ++i)
        j = buff[i];
    i = 0x100034;
    j = i / (i - 0x100034);
    exit(-5);
    return 0;
}

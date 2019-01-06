#include<init.h>
#include<memory.h>
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
static long _syscall2(int syscall_num, long arg1, int arg2)
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

static long getpid()
{
    return(_syscall0(SYSCALL_GETPID));
}

static long write(char *buf, int length)
{
    return(_syscall2(SYSCALL_WRITE, (long) buf, length));
}

static long expand(u32 size, int flags)
{
    return(_syscall2(SYSCALL_EXPAND, size, flags));
}

static long shrink(u32 size, int flags)
{
    return(_syscall2(SYSCALL_SHRINK, size, flags));
}

int main()
{
    char* ptr = (char*) expand(8, MAP_WR);
	*(ptr + 4096) = 'X';
	write(ptr + 4096, 1);
	shrink(8, MAP_WR);
	*(ptr + 4096) = 'A';
	write(ptr + 4096, 1);
    exit(0);
}

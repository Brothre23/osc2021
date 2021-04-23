#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "schedule.h"
#include "syscall.h"
#include "sysregs.h"

void foo()
{
    while(1)
    {
        for(int i = 0; i < 100000000; i++) 
        {
            if ( i % 10000000 == 0)
                printf("Thread ID: %d %d\n", getpid(), i);
        }
    }
}

void fork_test()
{
    if (fork() == 0)
    {
        while(1)
        {
            for(int i = 0; i < 100000000; i++) 
            {
                if ( i % 10000000 == 0)
                    printf("Child ID: %d %d\n", getpid(), i);
            }
        }
    }
    else
    {
        while(1)
        {
            for(int i = 0; i < 100000000; i++) 
            {
                if ( i % 10000000 == 0)
                    printf("Parent ID: %d %d\n", getpid(), i);
            }
        }
    }
}

int main()
{
    // printf("Hello World!\n\n");
    // shell_start();

    init_uart();
    init_printf(0, putc);
    init_memory();
    init_schedule();
    init_timer();

    // for (int i = 0; i < 5; i++)
        // thread_create(foo);
    thread_create(fork_test);

    unsigned int current_pid = get_current_task();
    struct task_struct *current_task = task_pool[current_pid];
    start_context(&current_task->context);

    // while (1) {};

    return 0;
}
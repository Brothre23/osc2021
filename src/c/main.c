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
    int counter = 6;
    while (counter--)
    {
        for(int i = 0; i < 10000000; i++) 
        {
            if ( i % 2000000 == 0)
                printf("[foo] pid: %d %d\n", getpid(), i);
        }
        // schedule();
    }
    exit();
}

void fork_test()
{
    if (fork() == 0)
    {
        // int counter = 20;
        // while (counter--)
        // {
        //     for(int i = 0; i < 100000000; i++) 
        //     {
        //         if ( i % 10000000 == 0)
        //             printf("child ID: %d %d\n", getpid(), i);
        //     }
        // }
        // exit();
        const char* argv[] = {"argv_test", "-o", "arg2", "meow", 0};
        exec("argv_test.img", argv);
    }
    else
    {
        int counter = 2;
        while (counter--)
        {
            for(int i = 0; i < 10000000; i++) 
            {
                if ( i % 2000000 == 0)
                    printf("[parent] pid: %d %d\n", getpid(), i);
            }
        }
        exit();
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

    for (int i = 0; i < 2; i++)
        thread_create(foo);
    // thread_create(foo);
    thread_create(fork_test);

    unsigned int current_pid = get_current_task();
    struct task_struct *current_task = task_pool[current_pid];
    start_context(&current_task->context);

    // while (1) {};

    return 0;
}
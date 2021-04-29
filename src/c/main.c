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
    for (int i = 0; i < 20; i++)
    {
        int delay = 1000000;
        while(delay--) {}
        printf("[FOO] pid: %d %d\n", getpid(), i);
    }
    exit();
}

void user_test()
{
    const char* argv[] = {"argv_test", "-o", "arg2", 0};
    exec("argv_test.img", argv);
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

    for (int i = 0; i < 5; i++)
        thread_create(foo);
    thread_create(user_test);

    unsigned int current_pid = get_current_task();
    struct task_struct *current_task = task_pool[current_pid];
    start_context(&current_task->context);

    return 0;
}
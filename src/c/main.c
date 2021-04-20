#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "schedule.h"

void foo()
{
    for(int i = 0; i < 10; ++i) 
    {
        printf("Thread ID: %d %d\n", get_current_task(), i);
        delay(1000000);
        schedule();
    }
}

int main()
{
    init_uart();
    init_printf(0, putc);
    init_memory();
    init_timer();
    init_schedule();

    // printf("Hello World!\n\n");
    // shell_start();

    for (int i = 0; i < 5; i++)
        thread_create(foo);

    unsigned int current_pid = get_current_task();
    struct task_struct *current = task_pool[current_pid];
    start_context(&current->context);

    return 0;
}
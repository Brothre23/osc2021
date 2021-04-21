#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "schedule.h"
#include "syscall.h"

void foo()
{
    for(int i = 0; i < 10; ++i) 
    {
        printf("Thread ID: %d %d\n", getpid(), i);
        delay(1000000);
        schedule();
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

    for (int i = 0; i < 5; i++)
        thread_create(foo);

    schedule();
    while (1)
    {

    }

    return 0;
}
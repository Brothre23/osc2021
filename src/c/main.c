#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "schedule.h"
#include "syscall.h"
#include "vfs.h"
#include "string.h"
#include "cpio.h"
#include "sdhost.h"

void vfs_test()
{
    const char *vfs_argv[] = {"vfs_test", 0};
    exec("vfs_test.img", vfs_argv);
}

int main()
{
    init_uart();
    init_printf(0, putc);
    init_memory();
    init_schedule();
    init_rootfs();
    // init_sd();

    printf("Hello World!\n\n");
    shell_start();

    // thread_create(vfs_test);

    // printf("jump to user program\n");

    // unsigned int current_pid = get_current_task();
    // struct task_struct *current_task = task_pool[current_pid];
    // start_context(&current_task->context);

    return 0;
}
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

void foo()
{
    for (int i = 0; i < 20; i++)
    {
        delay(1000000);
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
    init_uart();
    init_printf(0, putc);
    init_memory();
    init_schedule();
    init_rootfs();

    // printf("Hello World!\n\n");
    // shell_start();

    // for (int i = 0; i < 5; i++)
    //     thread_create(foo);
    // thread_create(user_test);

    char file_name[32];
    char *file_content;
    char *ramfs = (char *)0x8000000;
    int file_size;
    struct file *cpio_file;

    while (1)
    {
        strset(file_name, 0, 32);
        file_name[0] = '/';
        file_size = cpio_parse_header(&ramfs, file_name + 1, &file_content);

        if ((strcmp(file_name + 1, "TRAILER!!!") == 0))
            break;

        cpio_file = vfs_open(file_name, O_CREAT);
        vfs_write(cpio_file, file_content, file_size);
        vfs_close(cpio_file);
    }

    // unsigned int current_pid = get_current_task();
    // struct task_struct *current_task = task_pool[current_pid];
    // start_context(&current_task->context);

    return 0;
}
#include "shell.h"
#include "uart.h"
#include "printf.h"
#include "mm.h"
#include "timer.h"
#include "schedule.h"
#include "syscall.h"
#include "sysregs.h"
#include "vfs.h"

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
    init_timer();
    init_rootfs();

    // printf("Hello World!\n\n");
    // shell_start();

    // for (int i = 0; i < 5; i++)
    //     thread_create(foo);
    // thread_create(user_test);

    struct file *test_1 = vfs_open("/test.txt", O_CREAT);
    vfs_write(test_1, "HELLO", 5);
    vfs_close(test_1);

    struct file *test_2 = vfs_open("/test.txt", O_APPEND);
    vfs_write(test_2, "WORLD", 5);
    vfs_close(test_2);

    struct file *test_3 = vfs_open("/test.txt", 0);
    char buffer[10];
    vfs_read(test_2, buffer, 10);
    vfs_close(test_3);

    printf("%s\n", buffer);

    // unsigned int current_pid = get_current_task();
    // struct task_struct *current_task = task_pool[current_pid];
    // start_context(&current_task->context);

    return 0;
}
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

void vfs_test()
{
    // test for single-level file
    char buffer[100];
    strset(buffer, 0, 100);

    int a = open("hello", O_CREAT);
    int b = open("world", O_CREAT);

    write(a, "Hello ", 6);
    write(b, "World!", 6);

    close(a);
    close(b);
    
    b = open("hello", 0);
    a = open("world", 0);

    int size = 0;
    size += read(b, buffer, 100);
    size += read(a, buffer + size, 100);

    close(b);
    close(a);

    printf("\n%s\n", buffer);

    // test for multi-level file
    make_directory("folder");
    change_directory("folder");

    make_directory("folder_in_folder");
    int fd_meow = open("meow", O_CREAT);
    write(fd_meow, "meow_test", 9);
    close(fd_meow);

    char buffer_meow[50];
    strset(buffer_meow, 0, 50);

    fd_meow = open("meow", 0);
    read(fd_meow, buffer_meow, 9);
    close(fd_meow);

    printf("%s\n", buffer_meow);

    // test for reading directory
    int folder = open("./..", 0);
    char **diretories = read_directory(folder);
    for (int i = 0; (char *)diretories[i] != 0;i++)
        printf("%s ", diretories[i]);
    printf("\n");
    close(folder);

    printf("\nYES!\n\n");

    exit();
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

    thread_create(vfs_test);

    unsigned int current_pid = get_current_task();
    struct task_struct *current_task = task_pool[current_pid];
    start_context(&current_task->context);

    return 0;
}
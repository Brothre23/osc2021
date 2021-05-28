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
    init_sd();

    // printf("Hello World!\n\n");
    // shell_start();

    // char file_name[32];
    // char *file_content;
    // char *ramfs = (char *)0x8000000;
    // int file_size;
    // struct file *cpio_file;

    // while (1)
    // {
    //     strset(file_name, 0, 32);
    //     file_name[0] = '/';
    //     file_size = cpio_parse_header(&ramfs, file_name + 1, &file_content);

    //     if ((strcmp(file_name + 1, "TRAILER!!!") == 0))
    //         break;

    //     cpio_file = vfs_open(file_name, O_CREAT);
    //     vfs_write(cpio_file, file_content, file_size);
    //     vfs_close(cpio_file);
    // }

    char mounting_point[] = "/sdp1";
    vfs_make_directory(mounting_point);
    vfs_mount("sdcard", mounting_point, "fat32");

    struct file *test = vfs_open("/sdp1/TEST.TXT", 0);
    printf("%x %s\n", test, test->dentry->name);
    char read_buffer[1024];
    strset(read_buffer, 0, 1024);

    char write_buffer[] = "MEOWMEOW WRITE TEST";
    int write_size = vfs_write(test, write_buffer, strlen(write_buffer));
    printf("%d\n", write_size);
    vfs_close(test);

    test = vfs_open("/sdp1/TEST.TXT", 0);
    int read_size_1 = vfs_read(test, read_buffer, 520);
    int read_size_2 = vfs_read(test, read_buffer + 520, 200);
    printf("%s\n%d %d\n", read_buffer, read_size_1, read_size_2);
    vfs_close(test);

    // int size_1 = vfs_read(test, buffer, 520);
    // int size_2 = vfs_read(test, buffer + 520, 200);
    // printf("%s\n%d\n%d\n", buffer, size_1, size_2);

    // thread_create(vfs_test);

    // printf("jump to user program\n");

    // unsigned int current_pid = get_current_task();
    // struct task_struct *current_task = task_pool[current_pid];
    // start_context(&current_task->context);

    return 0;
}
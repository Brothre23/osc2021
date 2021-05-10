#ifndef SYSCALL_INTERNAL_H
#define SYSCALL_INTERNAL_H

#define SYS_GETPID              0   
#define SYS_UART_READ           1
#define SYS_UART_WRITE          2
#define SYS_EXEC                3
#define SYS_EXIT                4
#define SYS_FORK                5
#define SYS_SCHEDULE            6
#define SYS_ENABLE_CORE_TIMER   7
#define SYS_DISABLE_CORE_TIMER  8
#define SYS_SET_TIMEOUT         9
#define SYS_OPEN                10
#define SYS_CLOSE               11
#define SYS_READ                12
#define SYS_WRITE               13
#define SYS_READ_DIRECTORY      14
#define SYS_MAKE_DIRECTORY      15

#ifndef __ASSEMBLER__

/* for user program */
int getpid();
unsigned int uart_read(char buf[], unsigned int size);
unsigned int uart_write(char buf[], unsigned int size);
int exec(char *program_name, const char *argv[]);
void exit();
int fork();
void schedule();
void enable_core_timer();
void disable_core_timer();
void set_timeout(int second, const char *message);
int open(char *path_name, int flags);
int close(int fd);
int write(int fd, void *buffer, int length);
int read(int fd, void *buffer, int length);
char **read_directory(int fd);
int make_directory(char *path_name);

/* for kernel */
#include "exception.h"

/* functions defined in timer.S */
void sys_enable_core_timer();
void sys_disable_core_timer();
/* functions defined in timer.c */
void sys_set_timeout(struct trapframe *tf);
/* functions defined in schedule.c */
void sys_getpid(struct trapframe *tf);
void sys_exec(struct trapframe *tf);
void sys_exit();
void sys_fork(struct trapframe *tf);
void sys_schedule();
/* functions defined in uart.c */
void sys_uart_read(struct trapframe *tf);
void sys_uart_write(struct trapframe *tf);
/* functions defined in vfs.c */
void sys_open(struct trapframe *tf);
void sys_close(struct trapframe *tf);
void sys_read(struct trapframe *tf);
void sys_write(struct trapframe *tf);
void sys_read_directory(struct trapframe *tf);
void sys_make_directory(struct trapframe *tf);

#endif

#endif
#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_GETPID              0   
#define SYS_UART_READ           1
#define SYS_UART_WRITE          2
#define SYS_EXEC                3
#define SYS_EXIT                4
#define SYS_FORK                5
#define SYS_ENABLE_CORE_TIMER   6
#define SYS_DISABLE_CORE_TIMER  7
#define SYS_SET_TIMEOUT         8

#ifndef __ASSEMBLER__

int getpid();
unsigned int uart_read(char buf[], unsigned int size);
unsigned int uart_write(char buf[], unsigned int size);
int exec(const char *name, const char *argv[]);
void exit();
void fork();
void enable_core_timer();
void disable_core_timer();
void set_timeout(int second, const char *message);

#endif

#endif
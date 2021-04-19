#ifndef SYSCALL_H
#define SYSCALL_H

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
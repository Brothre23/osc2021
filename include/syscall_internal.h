#ifndef SYSCALL_INTERNAL_H
#define SYSCALL_INTERNAL_H

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

#include "exception.h"

/* functions defined in timer.S */
void sys_enable_core_timer();
void sys_disable_core_timer();
/* functions defined in timer.c */
void sys_set_timeout(struct trapframe *tf);
/* functions defined in schedule.c */
void sys_exit();

#endif

#endif
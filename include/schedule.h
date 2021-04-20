#ifndef SCHEDULE_H
#define SCHEDULE_H

#define TASK_POOL_SIZE 64
#define KSTACK_SIZE 4096
#define USTACK_SIZE 4096

struct cpu_context
{
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
};

enum task_state
{
    RUNNING,
    ZOMBIE,
    EXIT
};

struct task_struct
{
    int pid;
    enum task_state state;
    int exit_status;
    struct cpu_context context;
};

/* variables defined in schedule.c */
extern struct task_struct *task_pool[TASK_POOL_SIZE];
extern void *kstack_pool[TASK_POOL_SIZE];
extern void *ustack_pool[TASK_POOL_SIZE];

/* functions defined in schedule.S */
unsigned int get_current_task();
void update_current_task(unsigned int pid);
void context_switch(struct cpu_context *prev, struct cpu_context *next);
void start_context(struct cpu_context *current);

/* functions defined in schedule.c */
void delay(unsigned int count);
void init_schedule();
int thread_create(void (*function)());
void schedule();

#endif
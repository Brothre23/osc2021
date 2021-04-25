#include "schedule.h"
#include "mm.h"
#include "printf.h"
#include "exception.h"
#include "string.h"
#include "cpio.h"
#include "syscall.h"

struct task_struct *task_pool[TASK_POOL_SIZE];
void *kstack_pool[TASK_POOL_SIZE];
void *ustack_pool[TASK_POOL_SIZE];
unsigned int current_pid;

void delay(unsigned int count)
{
    while (count--)
        ;
}

void sys_getpid(struct trapframe *tf)
{
    int pid = get_current_task();
    tf->x[0] = pid;

    return;
}

void sys_exec(struct trapframe *tf)
{
    char *program_name = (char *)tf->x[0];
    void *program_start = cpio_run_program(program_name);

    char **argv = (char **)tf->x[1];

    int argc = 0;
    while (1)
    {
        if ((char *)argv[argc] == 0)
            break;
        argc++;
    }

    char **position = km_allocation(sizeof(char *) * (argc + 1));

    for (int i = argc - 1; i >= 0; i--)
    {
        char *address = (char *)argv[i];
        int length = strlen(argv[i]);
        int round_length = length % 16 > 0 ? (length / 16 + 1) * 16 : length;

        // put in the real string content
        tf->sp_el0 -= round_length;
        for (int j = 0; j < length + 1; j++)
            *((char *)tf->sp_el0 + j) = *(address + j);

        position[i + 1] = (char *)tf->sp_el0;
    }

    // setup argv[0] (program name)
    int length = strlen(program_name);
    int round_length = length % 16 > 0 ? (length / 16 + 1) * 16 : length;

    tf->sp_el0 -= round_length;
    for (int i = 0; i < length + 1; i++)
        *((char *)tf->sp_el0 + i) = program_name[i];

    position[0] = (char *)tf->sp_el0;

    argc++;

    // setup argv[i]
    for (int i = argc; i >= 0; i--)
    {
        tf->sp_el0 -= 8;
        
        if (i == argc)
            *(char **)tf->sp_el0 = 0;
        else
            *(char **)tf->sp_el0 = position[i];
    }

    km_free(position);

    // setup argv
    tf->sp_el0 -= 8;
    *(char ***)tf->sp_el0 = tf->sp_el0 + 8;
    tf->x[1] = *(char ***)tf->sp_el0;

    // setup argc
    tf->sp_el0 -= 16;
    *(int *)tf->sp_el0 = argc;
    tf->x[0] = argc;

    tf->elr_el1 = program_start;

    return;
}

void sys_exit()
{
    unsigned int current_pid = get_current_task();
    struct task_struct *current = task_pool[current_pid];

    current->state = ZOMBIE;
    sys_schedule();
}

void sys_fork(struct trapframe *tf)
{
    int parent_pid = get_current_task();

    int child_pid = thread_create((void (*)())NULL);
    struct task_struct *child = task_pool[child_pid];

    char *child_kstack = kstack_pool[child_pid] + KSTACK_SIZE;
    char *parent_kstack = kstack_pool[parent_pid] + KSTACK_SIZE;
    char *child_ustack = ustack_pool[child_pid] + USTACK_SIZE;
    char *parent_ustack = ustack_pool[parent_pid] + USTACK_SIZE;

    unsigned long kstack_offset = parent_kstack - (char *)tf;           // how many bytes the kernel stack of the parent thread have used
    unsigned long ustack_offset = parent_ustack - (char *)tf->sp_el0;   // how many bytes the user stack of the parent thread have used

    for (unsigned long i = 0; i < kstack_offset; i++)
        *(child_kstack - i) = *(parent_kstack - i);
    for (unsigned long i = 0; i < ustack_offset; i++)
        *(child_ustack - i) = *(parent_ustack - i);

    // setup the kernel stack of the child thread
    child->context.sp = (unsigned long)child_kstack - kstack_offset;
    // setup the user stack of the child thread
    struct trapframe *child_tf = (struct trapframe *)child->context.sp;
    child_tf->sp_el0 = (unsigned long)child_ustack - ustack_offset;

    // setup return values
    tf->x[0] = child_pid;
    child_tf->x[0] = 0;

    return;
}

int thread_create(void (*function)())
{
    struct task_struct *new_task = NULL;
    int pid = -1;

    for (int i = 0; i < TASK_POOL_SIZE; i++)
    {
        if (task_pool[i] == NULL)
        {
            new_task = km_allocation(sizeof(struct task_struct));
            pid = i;
            break;
        }
    }

    if (pid == -1)
    {
        printf("Failed to create a new task!\n");
        return -1;
    }

    new_task->state = RUNNING;
    new_task->pid = pid;
    new_task->need_schedule = 0;
    new_task->quota = TASK_QUOTA;

    new_task->context.lr = (unsigned long)return_to_user_code;
    // remember to reserve some space for the trapframe!!!!!
    new_task->context.sp = (unsigned long)km_allocation(KSTACK_SIZE) + (KSTACK_SIZE - TRAPFRAME_SIZE);
    new_task->context.fp = new_task->context.sp;

    struct trapframe *new_task_tf = (struct trapframe *)new_task->context.sp;
    new_task_tf->sp_el0 = (unsigned long)km_allocation(USTACK_SIZE) + USTACK_SIZE;
    new_task_tf->elr_el1 = (unsigned long)function;

    task_pool[pid] = new_task;
    kstack_pool[pid] = (void *)new_task->context.sp - (KSTACK_SIZE - TRAPFRAME_SIZE);
    ustack_pool[pid] = (void *)new_task_tf->sp_el0 - (USTACK_SIZE - TRAPFRAME_SIZE);

    return pid;
}

void kill_zombie()
{
    enable_core_timer();

    while (1)
    {
        for (int i = 1; i < TASK_POOL_SIZE; i++)
        {
            if (task_pool[i]->state == ZOMBIE)
            {
                printf("process ID: %d killed!\n", i);

                km_free(kstack_pool[i]);
                kstack_pool[i] = NULL;
                km_free(ustack_pool[i]);
                ustack_pool[i] = NULL;
                km_free(task_pool[i]);
                task_pool[i] = NULL;
            }
        }
        // for(int i = 0; i < 100000000; i++) 
        // {
        //     if ( i % 20000000 == 0)
        //         printf("kill_zombie()\n");
        // }
        // schedule();
    }
}

void meow()
{
    enable_core_timer();
    while (1)
    {
    }
}

void sys_schedule()
{
    int prev_pid = get_current_task();
    int next_pid = prev_pid;

    struct task_struct *prev = task_pool[prev_pid];
    struct task_struct *next = task_pool[(++next_pid) % TASK_POOL_SIZE];

    while (next == NULL || next->state != RUNNING)
    {
        next_pid = (next_pid + 1) % TASK_POOL_SIZE;
        next = task_pool[next_pid];
    }

    update_current_task(next_pid);
    context_switch(&prev->context, &next->context);
}

void init_schedule()
{
    for (int i = 0; i < TASK_POOL_SIZE; i++)
        task_pool[i] = NULL;

    int pid = thread_create(kill_zombie);
    update_current_task(pid);
}

void task_preemption()
{
    struct task_struct *current = task_pool[get_current_task()];
    if (current->need_schedule)
    {
        current->quota = TASK_QUOTA;
        current->need_schedule = 0;
        sys_schedule();
    }
}

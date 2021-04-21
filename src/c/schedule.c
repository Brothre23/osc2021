#include "schedule.h"
#include "mm.h"
#include "printf.h"
#include "exception.h"

struct task_struct *task_pool[TASK_POOL_SIZE];
void *kstack_pool[TASK_POOL_SIZE];
void *ustack_pool[TASK_POOL_SIZE];
unsigned int current_pid;

unsigned int get_current_task()
{
    return current_pid;
}

void update_current_task(unsigned int pid)
{
    current_pid = pid;
}

void delay(unsigned int count)
{
    while(count--);
}

void sys_getpid(struct trapframe *tf)
{
    int pid = get_current_task();
    tf->x[0] = pid;
    return;
}

void sys_exit()
{
    unsigned int current_pid = get_current_task();
    struct task_struct *current = task_pool[current_pid];

    current->state = ZOMBIE;
    schedule();
}

int thread_create(void (*function)())
{
    struct task_struct *new_task = NULL;
    int pid = -1;

    for (int i = 1; i < TASK_POOL_SIZE; i++)
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

    new_task->context.lr = (unsigned long)function;
    new_task->context.sp = (unsigned long)km_allocation(KSTACK_SIZE) + (KSTACK_SIZE - 16);  // to ensure 16-byte alignment
    new_task->context.fp = new_task->context.sp;                                            // fp points to the end of a stack

    task_pool[pid] = new_task;
    kstack_pool[pid] = (void *)new_task->context.sp - (KSTACK_SIZE - 16);

    return pid;
}

void kill_zombie()
{
    while (1)
    {
        for (int i = 1; i < TASK_POOL_SIZE; i++)
        {
            if (task_pool[i]->state == ZOMBIE)
            {
                printf("process ID: %d killed!\n", i);
                task_pool[i]->state = EXIT;

                km_free(kstack_pool[i]);
                task_pool[i] = NULL;
                km_free(ustack_pool[i]);
                ustack_pool[i] = NULL;
                km_free(task_pool[i]);
                ustack_pool[i] = NULL;
            }
        }
        schedule();
    }
}

void schedule()
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
    for (int i = 1; i < TASK_POOL_SIZE; i++)
        task_pool[i] = NULL;

    // int pid = thread_create(kill_zombie);
    update_current_task(0);
}

void task_preemption()
{
    struct task_struct *current = task_pool[get_current_task()];
    if (current->need_schedule)
    {
        printf("schedule\n");
        current->quota = TASK_QUOTA;
        current->need_schedule = 0;
        schedule();
    }
    return;
}

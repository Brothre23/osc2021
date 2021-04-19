#include "schedule.h"
#include "mm.h"

struct task_struct *task_pool[TASK_POOL_SIZE];
void *kstack_pool[KSTACK_SIZE];
void *ustack_pool[USTACK_SIZE];

int privilege_task_create(void (*function)()) 
{
    struct task_struct *new_task;
    unsigned int pid;

    for (int i = 0; i < TASK_POOL_SIZE; i++) 
    {
        if (task_pool[i] == NULL) 
        {
            new_task = km_allocation(sizeof(struct task_struct));
            pid = i;
            break;
        }
    }

    new_task->state = RUNNING;
    new_task->pid = pid;
    new_task->context.lr = (unsigned long)function;
    new_task->context.sp = km_allocation(KSTACK_SIZE) + (KSTACK_SIZE - 16);     // to ensure 16-bytes alignment
    new_task->context.fp = new_task->context.sp - (KSTACK_SIZE - 16);           // fp points to the end of a stack

    task_pool[pid] = new_task;
    kstack_pool[pid] = new_task->context.sp;

    return pid;
}

void kill_zombie() 
{
    while (1) 
    {
        for (int i = 0; i < TASK_POOL_SIZE; i++) 
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

void init_schedule()
{
    for (int i = 0; i < TASK_POOL_SIZE; i++)
        task_pool[i] = NULL;
    
}

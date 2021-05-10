#include "vfs.h"
#include "tmpfs.h"
#include "printf.h"
#include "mm.h"
#include "string.h"
#include "exception.h"
#include "syscall.h"
#include "schedule.h"

struct mount* rootfs;

void init_rootfs() 
{   
    struct filesystem* tmpfs = (struct filesystem*)km_allocation(sizeof(struct filesystem));
    tmpfs->name = (char*)km_allocation(sizeof(char) * 6);
    strcpy(tmpfs->name, "tmpfs");
    register_filesystem(tmpfs);

    tmpfs->setup_mount = tmpfs_setup_mount;
    rootfs = (struct mount*)km_allocation(sizeof(struct mount));
    tmpfs->setup_mount(tmpfs, rootfs);
}

int register_filesystem(struct filesystem* fs) 
{
    if (strcmp(fs->name, "tmpfs") == 0) 
    {
        printf("registering tmpfs\n");
        return tmpfs_register();
    }
    return -1;
}

struct file *construct_file(struct dentry *target_dentry)
{
    struct file *target_file = (struct file *)km_allocation(sizeof(struct file));

    target_file->f_ops = target_dentry->vnode->f_ops;
    target_file->dentry = target_dentry;
    target_file->f_position = 0;

    return target_file;
}

struct file *vfs_open(char *path_name, int flags)
{
    if (path_name[0] != '/')
    {
        printf("Does not support relative paths!\n");
        return NULL;
    }

    if (strcmp(path_name, "/") == 0)
    {
        struct file *target_file = construct_file(rootfs->root);
        return target_file;
    }

    struct dentry *target_dentry, *child_dentry;
    char component_name[32];
    parse_path_name(&target_dentry, component_name, path_name);

    int file_exist = target_dentry->vnode->v_ops->lookup(target_dentry, &child_dentry, component_name);
    if (file_exist == 0)
    {
        struct file *target_file = construct_file(child_dentry);
        if (flags & O_APPEND)
            target_file->f_position = target_file->dentry->vnode->f_size;
        return target_file;
    }
    else
    {
        if (flags & O_CREAT)
        {
            target_dentry->vnode->v_ops->create(target_dentry, &child_dentry, component_name);
            struct file *target_file = construct_file(child_dentry);
            return target_file;
        }
        else
            return NULL;
    }
}

int vfs_close(struct file* file) 
{
    km_free((void*)file);
    return 0;
}

int vfs_read(struct file* file, void* buffer, unsigned int length) 
{
    if (file->dentry->type != FILE) 
    {
        printf("Read from non-regular file!\n");
        return -1;
    }
    return file->f_ops->read(file, buffer, length);
}

int vfs_write(struct file* file, void* buffer, unsigned int length) 
{
    if (file->dentry->type != FILE)
    {
        printf("Write to non-regular file!\n");
        return -1;
    }
    return file->f_ops->write(file, buffer, length);
}

char **vfs_read_directory(struct dentry *dentry)
{
    if (dentry->type != DIRECTORY)
    {
        printf("LS on non-directory!");
        return (char **)NULL;
    }
    return dentry->vnode->v_ops->read_directory(dentry);
}

int vfs_make_directory(char *path_name)
{
    if (path_name[0] != '/')
    {
        printf("Does not support relative paths!\n");
        return -1;
    }

    struct dentry *target_dentry, *child_dentry;
    char component_name[32];
    parse_path_name(&target_dentry, component_name, path_name);

    return target_dentry->vnode->v_ops->make_directory(target_dentry, &child_dentry, component_name);
}

void sys_open(struct trapframe *tf)
{
    char *path_name = (char *)tf->x[0];
    int flags = tf->x[1];

    struct file *file = vfs_open(path_name, flags);
    if (file == NULL)
    {
        tf->x[0] = -1;
        return;
    }

    int fd;
    struct task_struct *current_task = task_pool[get_current_task()];

    if (current_task->opened_file.next_fd < current_task->opened_file.max_size)
    {
        fd = current_task->opened_file.next_fd;
        current_task->opened_file.next_fd++;
    }
    else
    {
        int found = 0;
        
        for (int i = 0; i < current_task->opened_file.max_size; i++)
        {
            if (current_task->opened_file.fd_table[i] == NULL)
            {
                found = 1;
                fd = i;
                break;
            }
        }

        if (!found)
        {
            int new_table_size = current_task->opened_file.max_size * 2;
            struct file **new_fd_table = km_allocation(sizeof(struct file *) * new_table_size);
            for (int i = 0; i < current_task->opened_file.max_size; i++)
                new_fd_table[i] = current_task->opened_file.fd_table[i];
            for (int i = current_task->opened_file.max_size; i < new_table_size; i++)
                new_fd_table[i] = NULL;

            km_free((void *)current_task->opened_file.fd_table);
            current_task->opened_file.fd_table = new_fd_table;
            current_task->opened_file.max_size = new_table_size;

            fd = current_task->opened_file.next_fd;
            current_task->opened_file.next_fd++;
        }
    }

    current_task->opened_file.fd_table[fd] = file;
    tf->x[0] = fd;

    return;
}

void sys_close(struct trapframe *tf)
{
    int fd = tf->x[0];
    if (fd < 0)
    {
        tf->x[0] = -1;
        return;
    }

    struct task_struct *current_task = task_pool[get_current_task()];
    struct file *file = current_task->opened_file.fd_table[fd];

    tf->x[0] = vfs_close(file);
    current_task->opened_file.fd_table[fd] = NULL;

    return;
}

void sys_read(struct trapframe *tf)
{
    int fd = tf->x[0];
    char *buffer = (char *)tf->x[1];
    if (fd < 0)
    {
        tf->x[0] = -1;
        return;
    }
    unsigned int length = (unsigned int)tf->x[2];

    struct task_struct *current_task = task_pool[get_current_task()];
    struct file *file = current_task->opened_file.fd_table[fd];
    if (file == NULL)
    {
        tf->x[0] = -1;
        return;
    }
    tf->x[0] = vfs_read(file, buffer, length);
    
    return;
}

void sys_write(struct trapframe *tf)
{
    int fd = tf->x[0];
    char *buffer = (char *)tf->x[1];
    if (fd < 0)
    {
        tf->x[0] = -1;
        return;
    }
    unsigned int length = (unsigned int)tf->x[2];

    struct task_struct *current_task = task_pool[get_current_task()];
    struct file *file = current_task->opened_file.fd_table[fd];
    if (file == NULL)
    {
        tf->x[0] = -1;
        return;
    }
    tf->x[0] = vfs_write(file, buffer, length);
    
    return;
}

void sys_read_directory(struct trapframe *tf)
{
    int fd = tf->x[0];
    if (fd < 0)
    {
        tf->x[0] = -1;
        return;
    }

    struct task_struct *current_task = task_pool[get_current_task()];
    struct file *file = current_task->opened_file.fd_table[fd];
    if (file == NULL)
    {
        tf->x[0] = -1;
        return;
    }
    tf->x[0] = (unsigned long)vfs_read_directory(file->dentry);

    return;
}

void sys_make_directory(struct trapframe *tf)
{
    char *path_name = (char *)tf->x[0];
    tf->x[0] = vfs_make_directory(path_name);
}

int parse_path_name_recursive(struct dentry *parent, struct dentry **target, char *component_name, char *path_name)
{
    strset(component_name, 0, 32);
    int i = 0;
    while (path_name[i])
    {
        if (path_name[i] == '/')
            break;

        component_name[i] = path_name[i];
        i++;
    }

    // the last character in path_name is '/'
    if (strcmp(component_name, "") == 0)
    {
        *target = parent;
        return 0;
    }
    else if (strcmp(component_name, ".") == 0)
        return parse_path_name_recursive(parent, target, component_name, path_name + i + 1);
    else if (strcmp(component_name, "..") == 0)
    {
        if (parent->parent == NULL)
            return 0;
        return parse_path_name_recursive(parent->parent, target, component_name, path_name + i + 1);
    }
    else 
    {
        int result = parent->vnode->v_ops->lookup(parent, target, component_name);
        if (result == -1)
        {
            // a new file or a new directory
            if (!path_name[i])
            {
                *target = parent;
                return 0;
            }
            // component_name is in the middle of path_name and does not exist
            else
                return -1;
        }
        else
        {
            // an existing directory
            if ((*target)->type == DIRECTORY)
            {
                parent = *target;
                return parse_path_name_recursive(parent, target, component_name, path_name + i + 1);
            }
            // an existing file
            else
            {
                *target = parent;
                return 0;
            }
        }
    }

    return 0;
}

int parse_path_name(struct dentry **target, char *component_name, char *path_name)
{
    if (path_name[0] == '/')
    {
        struct dentry *root_node = rootfs->root;
        return parse_path_name_recursive(root_node, target, component_name, path_name + 1);
    }
    else {}

    return 0;
}

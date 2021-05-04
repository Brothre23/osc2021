#include "vfs.h"
#include "tmpfs.h"
#include "printf.h"
#include "mm.h"
#include "string.h"

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
        printf("registering tmpfs...\n");
        return tmpfs_register();
    }
    return -1;
}

struct file *construct_file(struct vnode *target_node)
{
    struct file *target_file = (struct file *)km_allocation(sizeof(struct file));
    
    target_file->f_ops = target_node->f_ops;
    target_file->vnode = target_node;
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

    struct vnode *root_node = rootfs->root->vnode;
    struct vnode *target_node;

    int file_exist = root_node->v_ops->lookup(root_node, &target_node, path_name + 1);
    if (file_exist == 0)
    {
        struct file *target_file = construct_file(target_node);
        if (flags & O_APPEND)
            target_file->f_position = target_file->vnode->f_size + 1;
        return target_file;
    }
    else
    {
        if (flags & O_CREAT)
        {
            root_node->v_ops->create(root_node, &target_node, path_name + 1);
            struct file *target_file = construct_file(target_node);
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

int vfs_write(struct file* file, void* buffer, unsigned int length) 
{
    if (file->vnode->dentry->type != FILE) 
    {
        printf("Write to non-regular file!\n");
        return -1;
    }
    return file->f_ops->write(file, buffer, length);
}

int vfs_read(struct file* file, void* buffer, unsigned int length) 
{
    if (file->vnode->dentry->type != FILE) 
    {
        printf("Read from non-regular file!\n");
        return -1;
    }
    return file->f_ops->read(file, buffer, length);
}
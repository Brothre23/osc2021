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
        printf("Register TMPFS\n");
        return tmpfs_register();
    }
    return -1;
}

struct file *vfs_open(const char *pathname, int flags)
{

}

int vfs_close(struct file* file) 
{
    km_free((void*)file);
    return 0;
}

int vfs_write(struct file* file, const void* buffer, unsigned int length) 
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
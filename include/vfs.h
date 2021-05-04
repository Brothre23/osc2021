#ifndef VFS_H
#define VFS_H

#include "list.h"

#define O_CREAT     0
#define O_APPEND    1

extern struct mount *rootfs;

enum dentry_type
{
    FILE,
    DIRECTORY
};

struct vnode
{
    struct vnode_operations *v_ops;
    struct file_operations *f_ops;
    // dentry may be a list
    struct dentry *dentry;
    void *internal;
};

struct dentry 
{    
    char name[32];
    struct list_head list;
    struct list_head children;
    struct dentry* parent;
    struct vnode* vnode;
    enum dentry_type type;
};

struct file
{
    struct vnode* vnode; 
    unsigned int f_position;        // The next read/write position of this opened file
    struct file_operations *f_ops;
};

struct mount
{
    struct dentry *root;
    struct filesystem *fs;
};

struct filesystem
{
    char *name;
    int (*setup_mount)(struct filesystem *fs, struct mount *mount);
};

struct file_operations
{
    int (*write)(struct file *file, const void *buffer, unsigned int length);
    int (*read)(struct file *file, void *buffer, unsigned int length);
};

struct vnode_operations
{
    int (*lookup)(struct vnode *directory, struct vnode **target, char *component_name);
    int (*create)(struct vnode *directory, struct vnode **target, char *component_name);
};

void init_rootfs();
int register_filesystem(struct filesystem *fs);
struct file *vfs_open(const char *tatget_path, int flags);
int vfs_close(struct file *file);
int vfs_write(struct file *file, const void *buffer, unsigned int length);
int vfs_read(struct file *file, void *buffer, unsigned int length);

#endif
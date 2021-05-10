#ifndef VFS_H
#define VFS_H

#include "list.h"

#define O_NORMAL                0
#define O_CREAT                 1
#define O_APPEND                2
#define INITIAL_FD_TABLE_SIZE   32

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
    unsigned int f_size;
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
    struct dentry *dentry;
    unsigned int f_position; // The next read/write position of this opened file
    struct file_operations *f_ops;
};

struct task_file
{
    int max_size;
    int next_fd;
    struct file **fd_table;
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
    int (*write)(struct file *file, void *buffer, unsigned int length);
    int (*read)(struct file *file, void *buffer, unsigned int length);
};

struct vnode_operations
{
    int (*lookup)(struct dentry *parent, struct dentry **target, char *component_name);
    int (*create)(struct dentry *parent, struct dentry **target, char *component_name);
    char **(*read_directory)(struct dentry *parent);
    int (*make_directory)(struct dentry *parent, struct dentry **child, char *path_name);
};

void init_rootfs();
int register_filesystem(struct filesystem *fs);
struct file *vfs_open(char *path_name, int flags);
int vfs_close(struct file *file);
int vfs_read(struct file *file, void *buffer, unsigned int length);
int vfs_write(struct file *file, void *buffer, unsigned int length);
char **vfs_read_directory(struct dentry *parent);
int vfs_make_directory(char *path_name);

#endif
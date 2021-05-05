#ifndef TMPFS_H
#define TMPFS_H

#include "vfs.h"

extern struct vnode_operations* tmpfs_v_ops;
extern struct file_operations* tmpfs_f_ops;

#define INITIAL_BUFFER_SIZE 256

struct tmpfs_internal
{
    unsigned int buffer_size;
    char *content;
};

int tmpfs_register();
int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount);

// vnode operations
int tmpfs_lookup(struct vnode *directory, struct vnode **target, char *component_name);
int tmpfs_create(struct vnode *directory, struct vnode **target, char *compenent_name);

// file operations
int tmpfs_read(struct file *file, void *buffer, unsigned int length);
int tmpfs_write(struct file *file, void *buffer, unsigned int length);

#endif
#ifndef TMPFS_H
#define TMPFS_H

#include "vfs.h"

#define INITIAL_BUFFER_SIZE 256

struct tmpfs_internal
{
    unsigned int buffer_size;
    char *content;
};

int tmpfs_mount(struct dentry **mounting_dentry, const char *device);
int tmpfs_register();
int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount, const char *device);

// vnode operations
int tmpfs_lookup(struct dentry *parent, struct dentry **target, char *component_name);
int tmpfs_create(struct dentry *parent, struct dentry **target, char *compenent_name);
char **tmpfs_read_directory(struct dentry *directory);
int tmpfs_make_directory(struct dentry *parent, struct dentry **child, char *path_name);

// file operations
int tmpfs_read(struct file *file, void *buffer, unsigned int length);
int tmpfs_write(struct file *file, void *buffer, unsigned int length);

#endif
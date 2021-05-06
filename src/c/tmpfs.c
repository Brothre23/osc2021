#include "vfs.h"
#include "tmpfs.h"
#include "mm.h"
#include "list.h"
#include "string.h"
#include "printf.h"

struct vnode_operations* tmpfs_v_ops;
struct file_operations* tmpfs_f_ops;

struct vnode *tmpfs_create_vnode(struct dentry *dentry)
{
    struct vnode *vnode = (struct vnode *)km_allocation(sizeof(struct vnode));

    vnode->dentry = dentry;
    vnode->f_ops = tmpfs_f_ops;
    vnode->v_ops = tmpfs_v_ops;
    vnode->f_size = 0;

    return vnode;
}

struct dentry *tmpfs_create_dentry(struct dentry *parent, char *name, int type)
{
    struct dentry *dentry = (struct dentry *)km_allocation(sizeof(struct dentry));
    
    strcpy(name, dentry->name);
    dentry->parent = parent;
    list_init_head(&dentry->list);
    list_init_head(&dentry->children);
    
    if (parent != NULL)
        list_add_tail(&dentry->list, &parent->children);

    dentry->vnode = tmpfs_create_vnode(dentry);
    dentry->type = type;

    return dentry;
}

int tmpfs_register() 
{
    tmpfs_v_ops = (struct vnode_operations*)km_allocation(sizeof(struct vnode_operations));
    tmpfs_v_ops->lookup = tmpfs_lookup;
    tmpfs_v_ops->create = tmpfs_create;
    tmpfs_f_ops = (struct file_operations*)km_allocation(sizeof(struct file_operations));
    tmpfs_f_ops->read = tmpfs_read;
    tmpfs_f_ops->write = tmpfs_write;

    return 0;
}

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount) 
{
    mount->fs = fs;
    mount->root = tmpfs_create_dentry(NULL, "/", DIRECTORY);
    return 0;
}

int tmpfs_lookup(struct vnode *directory, struct vnode **target, char *component_name)
{
    if (strcmp(component_name, "") == 0)
    {
        *target = directory;
        return -1;
    }

    struct list_head *p = &directory->dentry->children;
    list_for_each(p, &directory->dentry->children)
    {
        struct dentry *dentry = list_entry(p, struct dentry, list);
        if (!strcmp(dentry->name, component_name))
        {
            *target = dentry->vnode;
            return 0;
        }
    }

    *target = NULL;
    return -1;
}

int tmpfs_create(struct vnode* directory, struct vnode **target, char *compenent_name)
{
    struct tmpfs_internal *internal = (struct tmpfs_internal *)km_allocation(sizeof(struct tmpfs_internal));
    internal->buffer_size = INITIAL_BUFFER_SIZE;
    internal->content = (char *)km_allocation(INITIAL_BUFFER_SIZE);

    struct dentry *child_dentry = tmpfs_create_dentry(directory->dentry, compenent_name, FILE);
    child_dentry->vnode->internal = (void *)internal;
    
    *target = child_dentry->vnode;
    
    return 0;
}

int tmpfs_read(struct file *file, void *buffer, unsigned int length)
{
    struct tmpfs_internal *interal = (struct tmpfs_internal *)file->vnode->internal;

    char *target = buffer;
    char *source = &interal->content[file->f_position];

    unsigned int i;
    for (i = 0; i < length && i + file->f_position < file->vnode->f_size; i++)
        target[i] = source[i];

    file->f_position += i;

    return i;
}

int tmpfs_write(struct file *file, void *buffer, unsigned int length)
{
    struct tmpfs_internal *interal = (struct tmpfs_internal *)file->vnode->internal;

    if (length + file->f_position > interal->buffer_size)
    {
        int new_size = 1;
        while(new_size < length)
            new_size *= 2;

        char *enlarged_content = km_allocation(new_size);
        for (int i = 0; i < interal->buffer_size; i++)
            enlarged_content[i] = interal->content[i];
            
        km_free(interal->content);

        interal->content = enlarged_content;
        interal->buffer_size = new_size;
    }

    char *target = &interal->content[file->f_position];
    char *source = buffer;

    unsigned int i;
    for (i = 0; i < length; i++)
        target[i] = source[i];

    file->f_position += i;
    if (file->f_position > file->vnode->f_size)
        file->vnode->f_size = file->f_position;

    return i;
}

#include "vfs.h"
#include "tmpfs.h"
#include "mm.h"
#include "list.h"
#include "string.h"

struct vnode_operations* tmpfs_v_ops;
struct file_operations* tmpfs_f_ops;

struct vnode *tmpfs_create_vnode(struct dentry *dentry)
{
    struct vnode *vnode = (struct vnode *)km_allocation(sizeof(struct vnode));

    vnode->dentry = dentry;
    vnode->f_ops = tmpfs_f_ops;
    vnode->v_ops = tmpfs_v_ops;

    return vnode;
}

struct dentry *tmpfs_create_dentry(struct dentry *parent, char *name, int type)
{
    struct dentry *dentry = (struct dentry *)km_allocation(sizeof(struct dentry));
    strcpy(dentry->name, name);

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
    tmpfs_v_ops->create = tmpfs_create;
    tmpfs_v_ops->lookup = tmpfs_lookup;
    // tmpfs_f_ops = (struct file_operations*)km_allocation(sizeof(struct file_operations));
    // tmpfs_f_ops->read = tmpfs_read;
    // tmpfs_f_ops->write = tmpfs_write;

    return 0;
}

int tmpfs_setup_mount(struct filesystem* fs, struct mount* mount) 
{
    mount->fs = fs;
    mount->root = tmpfs_create_dentry(NULL, "/", DIRECTORY);
    return 0;
}

int tmpfs_create(struct vnode* directory, struct vnode **target, char *compenent_name)
{
    struct tmpfs_internal *internal = (struct tmpfs_internal *)km_allocation(sizeof(struct tmpfs_internal));
    internal->file_size = 0;
    internal->buffer_size = INITIAL_BUFFER_SIZE;
    internal->content = (char *)km_allocation(INITIAL_BUFFER_SIZE);

    struct dentry *child_dentry = tmpfs_create_dentry(directory->dentry, compenent_name, FILE);
    child_dentry->vnode->internal = (void *)internal;

    *target = child_dentry->vnode;
    return 0;
}

int tmpfs_lookup(struct vnode *directory, struct vnode **target, char *component_name)
{
    if (strcmp(component_name, "") == 0)
    {
        *target = directory;
        return 0;
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

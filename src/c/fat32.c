#include "fat32.h"
#include "sdhost.h"
#include "mm.h"
#include "printf.h"
#include "string.h"

struct fat32_metadata fat32_metadata;
struct vnode_operations *fat32_v_ops = NULL;
struct file_operations *fat32_f_ops = NULL;

unsigned int get_cluster_block_index(unsigned int cluster_index)
{
    return fat32_metadata.data_region_block_index +
           (cluster_index - fat32_metadata.first_cluster) * fat32_metadata.sector_per_cluster;
}

unsigned int get_fat_block_index(unsigned int cluster_index)
{
    return fat32_metadata.fat_region_block_index + (cluster_index / FAT_ENTRY_PER_BLOCK);
}

struct vnode *fat32_create_vnode(struct dentry *dentry)
{
    struct vnode *vnode = (struct vnode *)km_allocation(sizeof(struct vnode));

    vnode->f_ops = fat32_f_ops;
    vnode->v_ops = fat32_v_ops;
    vnode->f_size = 0;

    return vnode;
}

struct dentry *fat32_create_dentry(struct dentry *parent, char *name, int type)
{
    struct dentry *dentry = (struct dentry *)km_allocation(sizeof(struct dentry));

    strcpy(name, dentry->name);
    dentry->parent = parent;
    list_init_head(&dentry->list);
    list_init_head(&dentry->children);

    if (parent != NULL)
        list_add_tail(&dentry->list, &parent->children);

    dentry->vnode = fat32_create_vnode(dentry);
    dentry->type = type;
    dentry->mounting_point = NULL;
    dentry->is_mounted = 0;

    return dentry;
}

int fat32_mount(struct dentry **mounting_dentry, const char *device)
{
    char sector[BLOCK_SIZE];
    read_block(0, sector);

    struct mbr_partition root_partition;
    for (int i = 0; i < 16; i++)
        ((unsigned char *)&root_partition)[i] = sector[446 + i];

    read_block(root_partition.starting_sector, sector);
    struct fat32_boot_sector *boot_sector = (struct fat32_boot_sector *)sector;

    fat32_metadata.data_region_block_index = root_partition.starting_sector +
                                             boot_sector->n_sectors_per_fat_32 * boot_sector->n_file_alloc_tabs +
                                             boot_sector->n_reserved_sectors;
    fat32_metadata.fat_region_block_index = root_partition.starting_sector + boot_sector->n_reserved_sectors;
    fat32_metadata.n_fat = boot_sector->n_file_alloc_tabs;
    fat32_metadata.sector_per_fat = boot_sector->n_sectors_per_fat_32;
    fat32_metadata.first_cluster = boot_sector->root_directory_start_cluster_number;
    fat32_metadata.sector_per_cluster = boot_sector->logical_sector_per_cluster;

    struct filesystem *fat32 = (struct filesystem *)km_allocation(sizeof(struct filesystem));
    fat32->name = (char *)km_allocation(sizeof(char) * 5);
    strcpy("fat32", fat32->name);
    register_filesystem(fat32);

    fat32->setup_mount = fat32_setup_mount;
    struct mount *mount = (struct mount *)km_allocation(sizeof(struct mount));
    fat32->setup_mount(fat32, mount, device);

    struct fat32_internal *internal = (struct fat32_internal *)km_allocation(sizeof(struct fat32_internal));
    internal->first_cluster = boot_sector->root_directory_start_cluster_number;

    (*mounting_dentry)->is_mounted = 1;
    (*mounting_dentry)->mounting_point = mount;
    (*mounting_dentry)->mounting_point->root->parent = *mounting_dentry;
    (*mounting_dentry)->mounting_point->root->vnode->internal = (void *)internal;

    return 0;
}

int fat32_register()
{
    if (fat32_f_ops != NULL && fat32_v_ops != NULL)
        return 0;

    fat32_v_ops = (struct vnode_operations *)km_allocation(sizeof(struct vnode_operations));
    fat32_v_ops->lookup = fat32_lookup;
    fat32_v_ops->create = NULL;
    fat32_v_ops->read_directory = NULL;
    fat32_v_ops->make_directory = NULL;

    fat32_f_ops = (struct file_operations *)km_allocation(sizeof(struct file_operations));
    fat32_f_ops->read = fat32_read;
    fat32_f_ops->write = NULL;

    return 0;
}

int fat32_setup_mount(struct filesystem *fs, struct mount *mount, const char *device)
{
    mount->fs = fs;
    mount->device = km_allocation(sizeof(char) * strlen(device));
    strcpy(device, mount->device);
    mount->root = fat32_create_dentry(NULL, "/", DIRECTORY);
    return 0;
}

int fat32_lookup(struct dentry *parent, struct dentry **target, char *component_name)
{
    struct list_head *p = &parent->children;
    list_for_each(p, &parent->children)
    {
        struct dentry *dentry = list_entry(p, struct dentry, list);
        if (!strcmp(dentry->name, component_name))
        {
            *target = dentry;
            return 0;
        }
    }

    if (fat32_load_dentry(parent, component_name) == -1)
    {
        *target = NULL;
        return -1;
    }

    p = &parent->children;
    list_for_each(p, &parent->children)
    {
        struct dentry *dentry = list_entry(p, struct dentry, list);
        if (!strcmp(dentry->name, component_name))
        {
            *target = dentry;
            return 0;
        }
    }

    *target = NULL;
    return -1;
}

int fat32_load_dentry(struct dentry *parent, char *component_name)
{
    struct fat32_internal *internal = (struct fat32_internal *)parent->vnode->internal;
    unsigned char sector[BLOCK_SIZE];
    unsigned int dentry_cluster = get_cluster_block_index(internal->first_cluster);
    read_block(dentry_cluster, sector);

    struct fat32_dentry *sector_dentries = (struct fat32_dentry *)sector;

    int found = -1;
    for (int i = 0; sector_dentries[i].name[0] != 0; i++)
    {
        if (sector_dentries[i].name[0] == 0xE5)
            continue;

        char file_name[13];
        int length = 0;

        for (int j = 0; j < 8; j++)
        {
            char c = sector_dentries[i].name[j];
            if (c == ' ')
                break;
            file_name[length++] = c;
        }
        file_name[length++] = '.';
        for (int j = 0; j < 3; j++)
        {
            char c = sector_dentries[i].ext[j];
            if (c == ' ')
                break;
            file_name[length++] = c;
        }
        file_name[length++] = '\0';

        if (strcmp(file_name, component_name) == 0)
            found = 0;

        struct dentry *dentry;
        if (sector_dentries[i].attribute == 0x10)
            dentry = fat32_create_dentry(parent, file_name, DIRECTORY);
        else
            dentry = fat32_create_dentry(parent, file_name, FILE);

        struct fat32_internal *child_internal = (struct fat32_internal *)km_allocation(sizeof(struct fat32_internal));
        child_internal->first_cluster = ((sector_dentries[i].cluster_high) << 16) | (sector_dentries[i].cluster_low);
        child_internal->dentry_cluster = dentry_cluster;

        dentry->vnode->internal = child_internal;
        dentry->vnode->f_size = sector_dentries[i].size;
    }

    return found;
}

int fat32_read(struct file *file, void *buffer, unsigned int length)
{
    struct fat32_internal *internal = (struct fat32_internal *)file->dentry->vnode->internal;
    unsigned long long current_cluster = internal->first_cluster;
    int remaining_length = length;
    int fat[FAT_ENTRY_PER_BLOCK];
    char internal_buffer[BLOCK_SIZE];
    int one_time_size = 0, total_size = 0;

    // get the right cluster to start with
    for (int i = 0; i < file->f_position / BLOCK_SIZE; i++)
    {
        read_block(get_fat_block_index(current_cluster), fat);
        current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
    }

    // each iteration reads in 512 bytes
    while (remaining_length > 0 && current_cluster != END_OF_CLUSTER)
    {
        read_block(get_cluster_block_index(current_cluster), internal_buffer);

        // check if it's over 512 bytes
        one_time_size = (remaining_length < BLOCK_SIZE) ? remaining_length : BLOCK_SIZE;
        // check if it exceeds the boundary of the file
        one_time_size = (file->f_position + one_time_size > file->dentry->vnode->f_size) 
                        ? (file->dentry->vnode->f_size - file->f_position) 
                        : one_time_size;

        remaining_length -= one_time_size;

        for (int i = 0; i < one_time_size; i++)
            ((char *)buffer + total_size)[i] = internal_buffer[file->f_position % BLOCK_SIZE + i];

        file->f_position += one_time_size;
        total_size += one_time_size;

        if (remaining_length > 0)
        {
            read_block(get_fat_block_index(current_cluster), fat);
            current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
        }
    }

    return total_size;
}

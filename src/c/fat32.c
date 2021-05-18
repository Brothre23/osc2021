#include "fat32.h"
#include "sdhost.h"
#include "mm.h"

struct fat32_metadata fat32_metadata;
struct vnode_operations *fat32_v_ops = NULL;
struct file_operations *fat32_f_ops = NULL;

int fat32_mount(struct dentry **mounting_dentry, char *device)
{   
    // read MBR
    char sector[BLOCK_SIZE];
    read_block(0, sector);

    struct mbr_partition root_partition;
    for (int i = 0; i < 16; i++)
        ((unsigned char *)&root_partition)[i] = sector[446 + i];

    read_block(root_partition.starting_sector, sector);
    struct fat32_boot_sector *boot_sector = (struct fat32_boot_sector *)sector;

    fat32_metadata.data_region_blk_idx = root_partition.starting_sector +
                                         boot_sector->n_sectors_per_fat_32 * boot_sector->n_file_alloc_tabs +
                                         boot_sector->n_reserved_sectors;
    fat32_metadata.fat_region_blk_idx = root_partition.starting_sector + boot_sector->n_reserved_sectors;
    fat32_metadata.n_fat = boot_sector->n_file_alloc_tabs;
    fat32_metadata.sector_per_fat = boot_sector->n_sectors_per_fat_32;
    fat32_metadata.first_cluster = boot_sector->root_dir_start_cluster_num;
    fat32_metadata.sector_per_cluster = boot_sector->logical_sector_per_cluster;

    struct filesystem* fat32 = (struct filesystem*)km_allocation(sizeof(struct filesystem));
    fat32->name = (char*)km_allocation(sizeof(char) * 6);
    strcpy(fat32->name, device);
    register_filesystem(fat32);

    fat32->setup_mount = fat32_setup_mount;
    struct mount *mount = (struct mount*)km_allocation(sizeof(struct mount));
    fat32->setup_mount(fat32, mount);

    (*mounting_dentry)->is_mounted = 1;
    (*mounting_dentry)->mounting_point = mount;
    (*mounting_dentry)->mounting_point->root->parent = mounting_dentry;
}

int fat32_register()
{
    if (fat32_f_ops == NULL && fat32_v_ops == NULL)
    {
        fat32_v_ops = (struct vnode_operations*)km_allocation(sizeof(struct vnode_operations));
        fat32_v_ops->lookup = NULL;
        fat32_v_ops->create = NULL;
        fat32_v_ops->read_directory = NULL;
        fat32_v_ops->make_directory = NULL;

        fat32_f_ops = (struct file_operations*)km_allocation(sizeof(struct file_operations));
        fat32_f_ops->read = NULL;
        fat32_f_ops->write = NULL;
    }

    return 0;
}

int fat32_setup_mount(struct filesystem* fs, struct mount* mount) 
{
    return 0;
}

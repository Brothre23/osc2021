#ifndef FAT32_H
#define FAT32_H

#include "vfs.h"

#define BLOCK_SIZE 512

int fat32_mount(struct dentry **mounting_dentry, char *device);
int fat32_register();
int fat32_setup_mount(struct filesystem *fs, struct mount *mount, char *device);

struct mbr_partition
{
    unsigned char status_flag;             //0x0
    unsigned char partition_begin_head;    //0x1
    unsigned short partition_begin_sector; //0x2-0x3
    unsigned char partition_type;          //0x4
    unsigned char partition_end_head;      //0x5
    unsigned short partition_end_sector;   //0x6-0x7
    unsigned int starting_sector;          //0x8-0xB
    unsigned int number_of_sector;         //0xC-0xF
};

struct fat32_boot_sector
{
    char jump[3]; // 0x0
    char oem[8];  // 0x3

    // BIOS Parameter Block
    unsigned short bytes_per_logical_sector;  // 0xB-0xC
    unsigned char logical_sector_per_cluster; // 0xD
    unsigned short n_reserved_sectors;        // 0xE-0xF
    unsigned char n_file_alloc_tabs;          // 0x10
    unsigned short n_max_root_dir_entries_16; // 0x11-0x12
    unsigned short n_logical_sectors_16;      // 0x13-0x14
    unsigned char media_descriptor;           // 0x15
    unsigned short logical_sector_per_fat_16; // 0x16-0x17

    // DOS3.31 BPB
    unsigned short physical_sector_per_track; // 0x18-0x19
    unsigned short n_heads;                   // 0x1A-0x1B
    unsigned int n_hidden_sectors;            // 0x1C-0x1F
    unsigned int n_sectors_32;                // 0x20-0x23

    // FAT32 Extended BIOS Parameter Block
    unsigned int n_sectors_per_fat_32;               // 0x24-0x27
    unsigned short mirror_flag;                      // 0x28-0x29
    unsigned short version;                          // 0x2A-0x2B
    unsigned int root_dir_start_cluster_num;         // 0x2C-0x2F
    unsigned short fs_info_sector_num;               // 0x30-0x31
    unsigned short boot_sector_bak_first_sector_num; // 0x32-0x33
    unsigned int reserved[3];                        // 0x34-0x3F
    unsigned char physical_drive_num;                // 0x40
    unsigned char unused;                            // 0x41
    unsigned char extended_boot_signature;           // 0x42
    unsigned int volume_id;                          // 0x43-0x46
    unsigned char volume_label[11];                  // 0x47-0x51
    unsigned char fat_system_type[8];                // 0x52-0x59
} __attribute__((packed));

struct fat32_dentry
{
    unsigned char name[8];           // 0x0-0x7
    unsigned char ext[3];            // 0x8-0xA
    unsigned char attr;              // 0xB
    unsigned char reserved;          // 0xC
    unsigned char create_time[3];    // 0xD-0xF
    unsigned short create_date;      // 0x10-0x11
    unsigned short last_access_date; // 0x12-0x13
    unsigned short cluster_high;     // 0x14-0x15
    unsigned int ext_attr;           // 0x16-0x19
    unsigned short cluster_low;      // 0x1A-0x1B
    unsigned int size;               // 0x1C-0x1F
} __attribute__((packed));

struct fat32_metadata
{
    unsigned int fat_region_blk_idx;
    unsigned int n_fat;
    unsigned int sector_per_fat;
    unsigned int data_region_blk_idx;
    unsigned int first_cluster;
    unsigned char sector_per_cluster;
};

#endif
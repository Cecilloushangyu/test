#ifndef __KS_FATFS_H
#define __KS_FATFS_H

#include <stdint.h>
#include "ff.h"
#include "diskio.h"

/* 为每个设备定义一个物理编号 */
#define EMMC		0     // 预留EMMC使用
#define SDCARD		1     // 预留SD卡使用
#define SRAM		2     //SRAM 测试


#define FATFS_UNMOUNT   	0
#define FATFS_MOUNTED       1

typedef struct
{ 
  uint8_t                 is_initialized;
  uint8_t                 volume_name[5];
  uint8_t                 current_path[512];
}Disk_Def;


struct diskio_drv
{
    DSTATUS (*initialize)   (BYTE);                             /*!< Initialize Disk Drive  */
    DSTATUS (*status)       (BYTE);                             /*!< Get Disk Status        */
    DRESULT (*read)         (BYTE, BYTE *, DWORD, UINT);        /*!< Read Sector(s)         */
    DRESULT (*write)        (BYTE, const BYTE *, DWORD, UINT);  /*!< Write Sector(s)        */
    DRESULT (*ioctl)        (BYTE, BYTE, void *);               /*!< I/O control operation  */
};

struct disk_dev
{
    uint8_t                 state;
    const struct diskio_drv *drv;
    uint8_t                 lun;
};

struct disk_mnt
{
    struct disk_dev     dev[FF_VOLUMES];
    volatile uint8_t    num;
};



typedef void (*FatfsMountStatusCbk)(U32 card_type, U32 status);


int ks_fatfs_init (void);
int ks_fatfs_mount(const char *path, struct diskio_drv *drv, uint8_t *drive);
int ks_fatfs_unmount(const char *path, uint8_t drive);
int ks_fatfs_add_mount_change_callback(FatfsMountStatusCbk cb_func);




#if 0

int ks_fsfat_init(uint8_t dev,char* path);
int ks_fsfat_deinit(uint8_t dev,char* path);

int ks_fsfat_link(uint8_t dev,char* path);
int ks_fsfat_mount(char* path);
int ks_fsfat_unmount(char* path);
int ks_fsfat_unlink(uint8_t dev,char* path);
#endif

#endif



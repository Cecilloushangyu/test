#ifndef _FATFS_HAL_H_
#define _FATFS_HAL_H_

#include "stdint.h"
#include "diskio.h"		/* FatFs lower layer API */
#include "ff.h"
#include "ks_os.h"
#include "diskio.h"	

DSTATUS emmc_fatfs_status (
	BYTE pdrv		/* 物理编号 */
);


DSTATUS emmc_fatfs_initialize (
	BYTE pdrv				/* 物理编号 */
);

DRESULT emmc_fatfs_read (
	BYTE pdrv,		/* 设备物理编号(0..) */
	BYTE *buff,		/* 数据缓存区 */
	DWORD sector,	/* 扇区首地址 */
	UINT count		/* 扇区个数(1..128) */
);


DRESULT emmc_fatfs_write (
	BYTE pdrv,			  /* 设备物理编号(0..) */
	const BYTE *buff,	/* 欲写入数据的缓存区 */
	DWORD sector,		  /* 扇区首地址 */
	UINT count			  /* 扇区个数(1..128) */
);

DRESULT emmc_fatfs_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 */
	void *buff		/* 写入或者读取数据地址指针 */
);

DSTATUS sd_fatfs_status (
	BYTE pdrv		/* 物理编号 */
);


DSTATUS sd_fatfs_initialize (
	BYTE pdrv				/* 物理编号 */
);

DRESULT sd_fatfs_read (
	BYTE pdrv,		/* 设备物理编号(0..) */
	BYTE *buff,		/* 数据缓存区 */
	DWORD sector,	/* 扇区首地址 */
	UINT count		/* 扇区个数(1..128) */
);


DRESULT sd_fatfs_write (
	BYTE pdrv,			  /* 设备物理编号(0..) */
	const BYTE *buff,	/* 欲写入数据的缓存区 */
	DWORD sector,		  /* 扇区首地址 */
	UINT count			  /* 扇区个数(1..128) */
);

DRESULT sd_fatfs_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 */
	void *buff		/* 写入或者读取数据地址指针 */
);

#endif

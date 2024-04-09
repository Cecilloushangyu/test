
#include "stdint.h"
#include "diskio.h"		/* FatFs lower layer API */
#include "ff.h"
#include "ks_os.h"
#include "ks_fatfs.h"
#include "ks_driver.h"
#include "ks_mmcsd.h"
#include "ks_mem.h"
//#define BLOCK_SIZE                4096
#define BLOCK_SIZE                512

#define SRAM_DEVICE_SIZE  (BLOCK_SIZE*120) 

static  uint8_t* SRAM_DEVICE_ADDR;

DSTATUS emmc_fatfs_status (
	BYTE pdrv		/* 物理编号 */
)
{

	DSTATUS status = STA_NOINIT;
	mmcsd_info info;
	int ret;
	ret = ks_driver_mmcsd_get_info(&info);
	if(ret==0 && info.card_type == CARD_TYPE_MMC )
	{
		status &= ~STA_NOINIT;
	}
	else
	{
		status = STA_NOINIT;
	}
	
	return status;
}

DSTATUS sd_fatfs_status (
	BYTE pdrv		/* 物理编号 */
)
{

	DSTATUS status = STA_NOINIT;
	mmcsd_info info;
	int ret;
	ret = ks_driver_mmcsd_get_info(&info);
	if(ret==0 && info.card_type == CARD_TYPE_SD )
	{
		status &= ~STA_NOINIT;
	}
	else
	{
		status = STA_NOINIT;
	}

	return status;
}


DSTATUS emmc_fatfs_initialize (
	BYTE pdrv				/* 物理编号 */
){
	uint16_t i;
	 DSTATUS status = STA_NOINIT;	  

	status = emmc_fatfs_status(pdrv);
	if(status == STA_NOINIT){
		//ks_driver_mmcsd_init(CARD_TYPE_MMC);
	}

	return status;

}

DSTATUS sd_fatfs_initialize (
	BYTE pdrv				/* 物理编号 */
){
	uint16_t i;
	  DSTATUS status = STA_NOINIT;	  

	status= sd_fatfs_status(pdrv);
	if(status == STA_NOINIT){
		//ks_driver_mmcsd_init(CARD_TYPE_MMC);
	}

	return status;

}


DRESULT emmc_fatfs_read (
	BYTE pdrv,		/* 设备物理编号(0..) */
	BYTE *buff,		/* 数据缓存区 */
	DWORD sector,	/* 扇区首地址 */
	UINT count		/* 扇区个数(1..128) */
)
{
	DRESULT status = RES_PARERR;
	uint32_t ret;

	ret = ks_driver_mmcsd_read(CARD_TYPE_MMC,sector,buff,count);
	if(ret == count){
		status = RES_OK;
	}
	return status;

}


DRESULT sd_fatfs_read (
	BYTE pdrv,		/* 设备物理编号(0..) */
	BYTE *buff,		/* 数据缓存区 */
	DWORD sector,	/* 扇区首地址 */
	UINT count		/* 扇区个数(1..128) */
)
{
	DRESULT status = RES_PARERR;
	uint32_t ret;

	ret = ks_driver_mmcsd_read(CARD_TYPE_SD,sector,buff,count);
	if(ret == count){
		status = RES_OK;
	}
	return status;

}


DRESULT emmc_fatfs_write (
	BYTE pdrv,			  /* 设备物理编号(0..) */
	const BYTE *buff,	/* 欲写入数据的缓存区 */
	DWORD sector,		  /* 扇区首地址 */
	UINT count			  /* 扇区个数(1..128) */
)
{
	uint32_t write_addr;
	uint32_t ret;
	DRESULT status = RES_PARERR;
	if (!count) {
		return RES_PARERR;		/* Check parameter */
	}
	//kprintf("disk_write %d %d \r\n",sector,count);
	ret = ks_driver_mmcsd_write(CARD_TYPE_MMC,sector,buff,count);
	if(ret == count){
		status = RES_OK;
	}
	return status;

}

DRESULT sd_fatfs_write (
	BYTE pdrv,			  /* 设备物理编号(0..) */
	const BYTE *buff,	/* 欲写入数据的缓存区 */
	DWORD sector,		  /* 扇区首地址 */
	UINT count			  /* 扇区个数(1..128) */
)
{
	uint32_t write_addr;
	uint32_t ret;
	DRESULT status = RES_PARERR;
	if (!count) {
		return RES_PARERR;		/* Check parameter */
	}
	//ks_os_printf(0,"disk_write %d %d \r\n",sector,count);
	ret = ks_driver_mmcsd_write(CARD_TYPE_SD,sector,buff,count);
	if(ret == count){
		status = RES_OK;
	}
	return status;

}

DRESULT emmc_fatfs_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 */
	void *buff		/* 写入或者读取数据地址指针 */
)
{
	mmcsd_info info;
	uint32_t ret;

	DRESULT status = RES_PARERR;
	ret = ks_driver_mmcsd_get_info(&info);
	if(ret==0 && info.card_type == CARD_TYPE_MMC )
	{
		switch (cmd) {
		/* 扇区数量： */
		case GET_SECTOR_COUNT:
		  *(DWORD * )buff = info.sector_count;	
		break;
		/* 扇区大小  */
		case GET_SECTOR_SIZE :
		  *(WORD * )buff = info.bytes_per_sector;
		break;
		/* 同时擦除扇区个数 */
		case GET_BLOCK_SIZE :
		  *(DWORD * )buff = info.erase_size/info.bytes_per_sector;
		break;		  
		}
		status = RES_OK;
	}
	
	return status;

}

DRESULT sd_fatfs_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 */
	void *buff		/* 写入或者读取数据地址指针 */
)
{
	mmcsd_info info;
	uint32_t ret;

	DRESULT status = RES_PARERR;
	ret = ks_driver_mmcsd_get_info(&info);
	if(ret==0 && info.card_type == CARD_TYPE_SD )
	{
		switch (cmd) {
		/* 扇区数量： */
		case GET_SECTOR_COUNT:
		  *(DWORD * )buff = info.sector_count;	
		break;
		/* 扇区大小  */
		case GET_SECTOR_SIZE :
		  *(WORD * )buff = info.bytes_per_sector;
		break;
		/* 同时擦除扇区个数 */
		case GET_BLOCK_SIZE :
		  *(DWORD * )buff = info.erase_size/info.bytes_per_sector;
		break;		  
		}
		status = RES_OK;
	}
	
	return status;

}

#if 0

DSTATUS mmcsd_fatfs_status (
	BYTE pdrv		/* 物理编号 */
)
{

	DSTATUS status = STA_NOINIT;
	mmcsd_info info;
	int ret;
	switch (pdrv) {
		case SDCARD:	/* SD CARD */
			ret = ks_driver_mmcsd_get_info(&info);
			if(ret==0 && info.card_type == CARD_TYPE_SD )
			{
				status &= ~STA_NOINIT;
			}
			else
			{
				status = STA_NOINIT;
			}
			break;
	
    	case EMMC:	
			ret = ks_driver_mmcsd_get_info(&info);
			if(ret==0 && info.card_type == CARD_TYPE_MMC )
			{
				status &= ~STA_NOINIT;
			}
			else
			{
				status = STA_NOINIT;
			}

		break;
		
		case SRAM:      
	      if(SRAM_DEVICE_ADDR!=NULL)
	      {
	        status &= ~STA_NOINIT;
	      }
	      else
	      {
	        status = STA_NOINIT;;
	      }
			break;

		default:
			status = STA_NOINIT;
	}
	return status;
}


DSTATUS mmcsd_fatfs_initialize (
	BYTE pdrv				/* 物理编号 */
)
{
  uint16_t i;
	DSTATUS status = STA_NOINIT;	
	switch (pdrv) {
		case SDCARD:	      
			status=disk_status(SDCARD);
			if(status == STA_NOINIT){
				//ks_driver_mmcsd_init(CARD_TYPE_SD);
			}
			break;
	
    	case EMMC:	   
			status=disk_status(EMMC);
			if(status == STA_NOINIT){
				//ks_driver_mmcsd_init(CARD_TYPE_MMC);
			}
			break;
			
		case SRAM:   
	
		   if(SRAM_DEVICE_ADDR==NULL){
			  SRAM_DEVICE_ADDR = ks_mem_heap_malloc(SRAM_DEVICE_SIZE);
		  	}
  	   		status=disk_status(SRAM);
			break;
      
		default:
			status = STA_NOINIT;
	}
	return status;
}



DRESULT mmcsd_fatfs_read (
	BYTE pdrv,		/* 设备物理编号(0..) */
	BYTE *buff,		/* 数据缓存区 */
	DWORD sector,	/* 扇区首地址 */
	UINT count		/* 扇区个数(1..128) */
)
{
	DRESULT status = RES_PARERR;
	uint32_t ret;

	
	//kprintf("disk_read %d %d \r\n",sector,count);
	switch (pdrv) {
		case SDCARD:	
			ret = ks_driver_mmcsd_read(CARD_TYPE_SD,sector,buff,count);
			if(ret == count){
				status = RES_OK;
			}
			break;
			
        case EMMC:	
			ret = ks_driver_mmcsd_read(CARD_TYPE_MMC,sector,buff,count);
			if(ret == count){
				status = RES_OK;
			}
			break;
			
		case SRAM:{
			uint32_t BufferSize = count*BLOCK_SIZE; 
			uint8_t *pSramAddress = (uint8_t *) (SRAM_DEVICE_ADDR + (sector*BLOCK_SIZE)); 
	
			for(; BufferSize != 0; BufferSize--)
			{
				*buff++ = *(__IO uint8_t *)pSramAddress++;  
			} 
	  
	      	status = RES_OK;
		}
		break;
    
		default:
			status = RES_PARERR;
	}
	return status;
}


DRESULT mmcsd_fatfs_write (
	BYTE pdrv,			  /* 设备物理编号(0..) */
	const BYTE *buff,	/* 欲写入数据的缓存区 */
	DWORD sector,		  /* 扇区首地址 */
	UINT count			  /* 扇区个数(1..128) */
)
{
	uint32_t write_addr;
	uint32_t ret;
	DRESULT status = RES_PARERR;
	if (!count) {
		return RES_PARERR;		/* Check parameter */
	}
	//kprintf("disk_write %d %d \r\n",sector,count);

	switch (pdrv) {
		case SDCARD:	   
		ret = ks_driver_mmcsd_write(CARD_TYPE_SD,sector,buff,count);
		if(ret == count){
			status = RES_OK;
		}
		break;
	
		case EMMC:	  
		ret = ks_driver_mmcsd_write(CARD_TYPE_MMC,sector,buff,count);
		if(ret == count){
			status = RES_OK;
		}
		break;
	

		case SRAM:
      	{
			uint32_t BufferSize = count*BLOCK_SIZE; 
			uint8_t *pSramAddress = (uint8_t *) (SRAM_DEVICE_ADDR + (sector*BLOCK_SIZE)); 

			for(; BufferSize != 0; BufferSize--)
			{
				*(__IO uint8_t *)pSramAddress++ = *buff++;    
			} 

	      	status = RES_OK;
	  	}
		break;
    
		default:
			status = RES_PARERR;
	}
	return status;
}




DRESULT mmcsd_fatfs_ioctl (
	BYTE pdrv,		/* 物理编号 */
	BYTE cmd,		  /* 控制指令 */
	void *buff		/* 写入或者读取数据地址指针 */
)
{
	mmcsd_info info;
	uint32_t ret;

	DRESULT status = RES_PARERR;
	switch (pdrv) {
		case SDCARD:	/* SD CARD */
			ret = ks_driver_mmcsd_get_info(&info);
			if(ret==0 && info.card_type == CARD_TYPE_SD )
			{
				switch (cmd) {
		        /* 扇区数量： */
		        case GET_SECTOR_COUNT:
		          *(DWORD * )buff = info.sector_count;	
		        break;
		        /* 扇区大小  */
		        case GET_SECTOR_SIZE :
		          *(WORD * )buff = info.bytes_per_sector;
		        break;
		        /* 同时擦除扇区个数 */
		        case GET_BLOCK_SIZE :
		          *(DWORD * )buff = info.erase_size/info.bytes_per_sector;
		        break;        
				}
			    status = RES_OK;
			}
			break;
    
		case EMMC:	
			ret = ks_driver_mmcsd_get_info(&info);
			if(ret==0 && info.card_type == CARD_TYPE_MMC )
			{
				switch (cmd) {
		        /* 扇区数量： */
		        case GET_SECTOR_COUNT:
		          *(DWORD * )buff = info.sector_count;	
		        break;
		        /* 扇区大小  */
		        case GET_SECTOR_SIZE :
		          *(WORD * )buff = info.bytes_per_sector;
		        break;
		        /* 同时擦除扇区个数 */
		        case GET_BLOCK_SIZE :
		          *(DWORD * )buff = info.erase_size/info.bytes_per_sector;
		        break;        
				}
			    status = RES_OK;
			}
			break;
    
		case SRAM:
			switch (cmd) {
	        /* 扇区数量：1536*4096/1024/1024=6(MB) */
	        case GET_SECTOR_COUNT:
	          *(DWORD * )buff = 1536;	
			  //*(DWORD * )buff = SRAM_DEVICE_SIZE/4096;
	        break;
	        /* 扇区大小  */
	        case GET_SECTOR_SIZE :
	          *(WORD * )buff = BLOCK_SIZE;
	        break;
	        /* 同时擦除扇区个数 */
	        case GET_BLOCK_SIZE :
	          *(DWORD * )buff = 1;
	        break;        
	      }
	      status = RES_OK;
		break;
    
		default:
			status = RES_PARERR;
	}
	return status;
}
#endif

__attribute__ ((weak)) DWORD get_fattime(void) {
	/* 返回当前时间戳 */
	return	  ((DWORD)(2015 - 1980) << 25)	/* Year 2015 */
			| ((DWORD)1 << 21)				/* Month 1 */
			| ((DWORD)1 << 16)				/* Mday 1 */
			| ((DWORD)0 << 11)				/* Hour 0 */
			| ((DWORD)0 << 5)				  /* Min 0 */
			| ((DWORD)0 >> 1);				/* Sec 0 */
}


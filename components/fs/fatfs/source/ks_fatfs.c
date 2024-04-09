#include "string.h"
#include "ks_fatfs.h"
#include "ks_printf.h"
#include "ks_shell.h"
#include "sys/errno.h"
#include "sys/fcntl.h"
#include "ks_vfs.h"
#include "fatfs_hal.h"
#include "ks_mmcsd.h"


#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef POINTER_ASSERT
#define POINTER_ASSERT(p) \
    if(p == NULL) \
    { \
        return -EINVAL; \
    }
#endif




typedef struct {
    int fatfs_inited ;
	uint32_t card_type;
	uint32_t mount_status;


} fatfs_ctx;


fatfs_ctx g_fatfs_ctx;

struct disk_mnt disk;

 typedef struct  FatFsMountStatusListCb
 {
	 struct list_node  cb_list;
	 FatfsMountStatusCbk cbfunc;
	 int is_used;
 }FatFsMountStatusListCb;
 
 FatFsMountStatusListCb s_fatfs_mount_statuscb[4];
 struct list_node  g_FatFsMountStatusChangeCallBackList;  

 static int ret_to_errno(FRESULT result)
 {
	 int err = 0;
 
	 switch (result)
	 {
	 case FR_OK:
		 return 0;
 
	 case FR_NO_PATH:
		 err = ENOTDIR;
		 break;
 
	 case FR_NO_FILE:
		 err = ENOENT;
		 break;
 
	 case FR_NO_FILESYSTEM:
		 err = ENODEV;
		 break;
 
	 case FR_TOO_MANY_OPEN_FILES:
		 err = ENFILE;
		 break;
 
	 case FR_INVALID_NAME:
		 err = ENAMETOOLONG;
		 break;
 
	 case FR_INVALID_PARAMETER:
	 case FR_INVALID_OBJECT:
	 case FR_INT_ERR:
		 err = EINVAL;
		 break;
 
	 case FR_INVALID_DRIVE:
	 case FR_NOT_ENABLED:
		 err = ENXIO;
		 break;
 
	 case FR_EXIST:
		 err = EEXIST;
		 break;
 
	 case FR_DISK_ERR:
	 case FR_NOT_READY:
		 err = EIO;
		 break;
 
	 case FR_WRITE_PROTECTED:
		 err = EROFS;
		 break;
 
	 case FR_LOCKED:
		 err = EBUSY;
		 break;
 
	 case FR_DENIED:
		 err = EISDIR;
		 break;
 
	 case FR_MKFS_ABORTED:
		 err = EBUSY;
		 break;
 
	 case FR_NOT_ENOUGH_CORE:
		 err = ENOMEM;
		 break;
 
	 case FR_TIMEOUT:
		 err = ETIMEDOUT;
		 break;
 
	 default:
		 err = EIO;
		 break;
	 }
 
	 return -err;
 }

static int fatfs_flags_get (int oflags)
{
    int flags = 0;

    switch (oflags & O_ACCMODE)
    {
    case O_RDONLY:
        flags |= FA_READ;
        break;
    case O_WRONLY:
        flags |= FA_WRITE;
        break;
    case O_RDWR:
        flags |= FA_READ | FA_WRITE;
        break;
    default:
        break;
    }

    if (oflags & O_CREAT)
    {
        flags |= FA_OPEN_ALWAYS;
    }

    if ((oflags & O_CREAT) && (oflags & O_EXCL))
    {
        flags |= FA_CREATE_NEW;
    }

    if (oflags & O_TRUNC)
    {
        flags |= FA_CREATE_ALWAYS;
    }

    if (oflags & O_APPEND)
    {
        flags |= FA_READ | FA_WRITE | FA_OPEN_APPEND;
    }

    return flags;
}


/**
  * @brief  Links a compatible diskio driver/lun id and increments the number of active
  *         linked drivers.
  * @note   The number of linked drivers (volumes) is up to 10 due to FatFs limits.
  * @param  drv: pointer to the disk IO Driver structure
  * @param  lun : only used for USB Key Disk to add multi-lun management
            else the parameter must be equal to 0
  * @retval Returns -1 in case of failure, otherwise return the drive (0 to volumes).
  */
static int fatfs_link_driver(const struct diskio_drv *drv, uint8_t lun)
{
    int ret = -1;
    int i;

    if(disk.num >= FF_VOLUMES)
        return ret;

    for(i = 0; i < FF_VOLUMES; i++)
    {
        if(disk.dev[i].drv != 0)
            continue;

        disk.dev[disk.num].state = 0;
        disk.dev[disk.num].drv = drv;
        disk.dev[disk.num].lun = lun;
        disk.num++;
        return i;
    }
    return ret;
}

/**
  * @brief  Unlinks a diskio driver and decrements the number of active linked
  *         drivers.
  * @param  drive: the disk drive (0 to volumes)
  * @param  lun : not used
  * @retval Returns -1 in case of failure, otherwise return the drive (0 to volumes).
  */
static int fatfs_unlink_driver(uint8_t drive, uint8_t lun)
{
    int ret = -1;

    if(disk.num >= 1 && drive < FF_VOLUMES)
    {
        if(disk.dev[drive].drv != 0)
        {
            disk.dev[drive].state= 0;
            disk.dev[drive].drv = 0;
            disk.dev[drive].lun = 0;
            disk.num--;
            return drive;
        }
    }

    return ret;
}

int fatfs_register(const struct diskio_drv *drv)
{
    return fatfs_link_driver(drv, 0);
}

int fatfs_unregister(uint8_t drive)
{
    return fatfs_unlink_driver(drive, 0);
}


static int ks_fatfs_open (struct file *file, const char *path_in_mp, int flags)
{
    FRESULT res;
    FIL     *fp;
    FILINFO info = {0};

    fp = (FIL *) malloc (sizeof(FIL));
    if (fp == NULL)
    {
        PRINT_ERR ("fail to malloc memory in FATFS, <malloc.c> is needed,"
                   "make sure it is added\n");
        return -EINVAL;
    }

    if (!(flags & O_CREAT) && (flags & O_TRUNC))
    {
        res = f_stat(path_in_mp, &info);
        if(res != FR_OK)
        {
            free(fp);
            return res;
        }
    }

    res = f_open (fp, path_in_mp, fatfs_flags_get (flags));
    if(res == FR_OK)
    {
        file->f_data = (void *) fp;
    }
    else
    {
        free(fp);
    }
    if (FR_LOCKED == res)
    {
        int err = 0;
        err = EACCES;
        return -err;
    }
    else
    {
        return ret_to_errno(res);
    }
}

static int ks_fatfs_close (struct file *file)
{
    FRESULT res;
    FIL     *fp = (FIL *)file->f_data;

    POINTER_ASSERT(fp);

    res = f_close(fp);
    if(res == FR_OK)
    {
        free(fp);
        file->f_data = NULL;
    }

    return ret_to_errno(res);
}

static ssize_t ks_fatfs_read (struct file *file, char *buff, size_t bytes)
{
    ssize_t size = 0;
    FRESULT res;
    FIL     *fp = (FIL *)file->f_data;

    if (buff == NULL || bytes == 0)
        return -EINVAL;

    POINTER_ASSERT(fp);
    res = f_read (fp, buff, bytes, (UINT *)&size);
    if(res != FR_OK)
    {
        PRINT_ERR ("failed to read, res=%d\n", res);
        return ret_to_errno(res);
    }
    return size;
}

static ssize_t ks_fatfs_write (struct file *file, const char *buff, size_t bytes)
{
    ssize_t  size = 0;
    FRESULT  res;
    FIL     *fp = (FIL *)file->f_data;

    if (buff == NULL || bytes == 0)
        return -EINVAL;

    POINTER_ASSERT(fp);
    res = f_write (fp, buff, bytes, (UINT *)&size);
    if(res != FR_OK || size == 0)
    {
        PRINT_ERR ("failed to write, res=%d\n", res);
        return ret_to_errno(res);
    }
    return size;
}

static off_t ks_fatfs_lseek (struct file *file, off_t off, int whence)
{
    FIL *fp = (FIL *)file->f_data;

    POINTER_ASSERT(fp);
    switch (whence)
    {
    case 0: // SEEK_SET
        break;
    case 1: // SEEK_CUR
        off += f_tell(fp);
        break;
    case 2: // SEEK_END
        off += f_size(fp);
        break;
    default:
    	ret_to_errno(FR_INVALID_PARAMETER);
        return -1;
    }
    
    if (off < 0)
    {
        return ret_to_errno(FR_INVALID_PARAMETER);
    }
    
    FRESULT res = f_lseek(fp, off);
    if (res == FR_OK)
    {

    	return off;
    }
    else
        return ret_to_errno(res);
}

int ks_fatfs_stat (struct mount_point *mp, const char *path_in_mp, struct stat *stat)
{
    FRESULT res;
    FILINFO info = {0};

    memset(stat, 0, sizeof(*stat));
    res = f_stat(path_in_mp, &info);
    if (res == FR_OK)
    {
        stat->st_size = info.fsize;
        if (info.fattrib & AM_DIR)
        {
            stat->st_mode = S_IFDIR;
        }
        else
        {
            stat->st_mode = S_IFREG;
        }
    }

    return ret_to_errno(res);
}


static int ks_fatfs_unlink (struct mount_point *mp, const char *path_in_mp)
{
    FRESULT res = f_unlink(path_in_mp);
    if (FR_NO_PATH == res)
    {
        int err = 0;
        err = ENOENT;
        return -err;
    }
    else
    {
    	return ret_to_errno(res);
    }

}


static int ks_fatfs_rename (struct mount_point *mp, const char *path_in_mp_old,
                             const char *path_in_mp_new)
{
    FRESULT res = f_rename(path_in_mp_old, path_in_mp_new);
    return ret_to_errno(res);
}

static int ks_fatfs_sync (struct file *file)
{
    FIL *fp = (FIL *)file->f_data;
    FRESULT res;

    POINTER_ASSERT(fp);

    res = f_sync(fp);
    return ret_to_errno(res);
}

static int ks_fatfs_opendir (struct dir *dir, const char *path)
{
    FRESULT  res;
    DIR     *dp;

    dp = (DIR *) malloc (sizeof (DIR));

    if (dp == NULL)
    {
        PRINT_ERR ("fail to malloc memory in SPIFFS, <malloc.c> is needed,"
                   "make sure it is added\n");
        return -ENOMEM;
    }

    res = f_opendir(dp, path);
    if(res != FR_OK)
    {
        free(dp);
        return ret_to_errno(res);
    }

    dir->d_data   = dp;
    dir->d_offset = 0;

    return FR_OK;
}

static int ks_fatfs_readdir (struct dir *dir, struct dirent *dent)
{
    FRESULT  res;
    DIR     *dp = (DIR *) dir->d_data;
    FILINFO  e;
    int     len;

    POINTER_ASSERT(dp);

    res = f_readdir(dp, &e);
    if (res != FR_OK)
    {
        return ret_to_errno(res);
    }

    len = MIN(sizeof(e.fname), VFS_MAX_DIR_NAME_LEN+1) - 1;
    strncpy ((char *)dent->name, (const char *) e.fname, len);
    dent->name [len] = '\0';
    dent->size = e.fsize;

    if (e.fattrib == AM_DIR)
    {
        dent->type = VFS_TYPE_DIR;
    }
    else
    {
        dent->type = VFS_TYPE_FILE;
    }

    return FR_OK;
}

static int ks_fatfs_closedir (struct dir *dir)
{
    FRESULT  res;
    DIR     *dp = (DIR *) dir->d_data;

    POINTER_ASSERT(dp);

    res = f_closedir (dp);
    if(res == FR_OK)
    {
        free (dp);
        dir->d_data = NULL;
    }

    return ret_to_errno(res);
}

static int ks_fatfs_mkdir(struct mount_point *mp, const char *path)
{
    FRESULT res = f_mkdir(path);
    if (FR_NO_PATH == res)
    {
        int err = 0;
        err = ENOENT;
        return -err;
    }
    else
    {
    	return ret_to_errno(res);
    }
}


static struct file_ops ks_fatfss =
{
    ks_fatfs_open,
    ks_fatfs_close,
    ks_fatfs_read,
    ks_fatfs_write,
    ks_fatfs_lseek,
    ks_fatfs_stat,
    ks_fatfs_unlink,
    ks_fatfs_rename,
    NULL,               /* ioctl not supported for now */
    ks_fatfs_sync,
    ks_fatfs_opendir,
    ks_fatfs_readdir,
    ks_fatfs_closedir,
    ks_fatfs_mkdir
};

static struct file_system fatfs_fs =
{
    "fatfs",
    &ks_fatfss,
    NULL,
    0
};


static struct diskio_drv emmc_drv =
{
    emmc_fatfs_initialize,
    emmc_fatfs_status,
    emmc_fatfs_read,
    emmc_fatfs_write,
    emmc_fatfs_ioctl
};

static struct diskio_drv sd_drv =
{
    sd_fatfs_initialize,
    sd_fatfs_status,
    sd_fatfs_read,
    sd_fatfs_write,
    sd_fatfs_ioctl
};
	
extern	void fs_shell_cmd_init();

void fatfs_mount_detect_notify(uint32_t card_type,	uint32_t status)
{

	FatFsMountStatusListCb * pcallback;
	char *name;

	list_for_every_entry( &g_FatFsMountStatusChangeCallBackList, pcallback, FatFsMountStatusListCb, cb_list )
	{	
		if(pcallback->is_used&&pcallback->cbfunc!=NULL){
			pcallback->cbfunc(card_type,status);
		}						
	}
}


void ks_fatfs_mmcsd_status_change_cbk(U32 card_type, U32 card_status){

	//PRINT_INFO ("mmcsd_status_change_cbk  card_type %d  card_status  %d \r\n",card_type,card_status);
    static int8_t drive = -1;
	int ret;
	g_fatfs_ctx.card_type = card_type;
	
	if (card_status == MMCSD_HOST_PLUGED)
	{
		//PRINT_INFO("mmcsd	pluged \r\n");
		if(card_type == CARD_TYPE_MMC){
			ret = ks_fatfs_mount("/fatfs/", &emmc_drv, (uint8_t *)&drive); 
		}else{
			ret = ks_fatfs_mount("/fatfs/", &sd_drv, (uint8_t *)&drive); 
		}
		
		if (ret < 0) {
		 	PRINT_ERR ("failed to mount fatfs! \r\n");
		}else{
			fs_shell_cmd_init();
			g_fatfs_ctx.mount_status = FATFS_MOUNTED;
			fatfs_mount_detect_notify(card_type,FATFS_MOUNTED);
		}

	}
	else if (card_status == MMCSD_HOST_UNPLUGED)
	{
	 	//PRINT_INFO("mmcsd	unpluged\r\n");
		if(card_type == CARD_TYPE_MMC){
			ret = ks_fatfs_unmount("/fatfs/", (uint8_t )drive);
		}else{
			ret = ks_fatfs_unmount("/fatfs/",  (uint8_t )drive) ;
		}
		
		if (ret < 0) {
		 	PRINT_ERR ("failed to unmount fatfs! \r\n");
		}else{
			//fs_shell_cmd_init();
			g_fatfs_ctx.mount_status = FATFS_UNMOUNT;
			fatfs_mount_detect_notify(card_type,FATFS_UNMOUNT);
		}
	}

}

int ks_fatfs_init (void)
{

    int8_t drive = -1;

    if (g_fatfs_ctx.fatfs_inited)
    {
        return 0;
    }

    if (ks_vfs_fs_init () != VFS_OK)
    {
        return VFS_NOK;
    }

    if (ks_vfs_fs_register (&fatfs_fs) != VFS_OK)
    {
        PRINT_ERR ("failed to register fs! \r\n");
        return VFS_NOK;
    }
	
	list_initialize(&g_FatFsMountStatusChangeCallBackList);

	ks_driver_mmcsd_add_status_change_callback(ks_fatfs_mmcsd_status_change_cbk);

    g_fatfs_ctx.fatfs_inited = 1;



    return VFS_OK;
}


static FATFS *fatfs_ptr = NULL;

int ks_fatfs_mount(const char *path, struct diskio_drv *drv, uint8_t *drive)
{
    int s_drive;
    char dpath[10] = {0};
    int ret = -1;
    BYTE *work_buff = NULL;
    FRESULT res;
    FATFS   *fs = NULL;

    s_drive = fatfs_register(drv);
    if(s_drive < 0)
    {
        PRINT_ERR("failed to register diskio! \r\n");
        return s_drive;
    }
    fs = (FATFS *) malloc (sizeof (FATFS));
    if (fs == NULL)
    {
        PRINT_ERR ("fail to malloc memory in FATFS, <malloc.c> is needed,"
                   "make sure it is added \r\n");
        goto err;
    }
    memset(fs, 0, sizeof(FATFS));
    sprintf(dpath, "%d:/", s_drive);
    res = f_mount(fs, (const TCHAR *)dpath, 1);
    if(res == FR_NO_FILESYSTEM)
    {
        work_buff = (BYTE *)malloc(FF_MAX_SS);
        if(work_buff == NULL)
            goto err_free;
        memset(work_buff, 0, FF_MAX_SS);
        res = f_mkfs((const TCHAR *)dpath, FM_ANY, 0, work_buff, FF_MAX_SS);
        if(res == FR_OK)
        {
            res = f_mount(NULL, (const TCHAR *)dpath, 1);
            res = f_mount(fs, (const TCHAR *)dpath, 1);
        }
        free(work_buff);
    }
    if(res != FR_OK)
    {
        PRINT_ERR("failed to mount fatfs, res=%d! \r\n", res);
        goto err_free;
    }

    ret = ks_vfs_fs_mount ("fatfs", path, fs);

    if (ret == VFS_OK)
    {
        PRINT_INFO ("fatfs mount at %s done! \r\n", path);
        *drive = s_drive;
        fatfs_ptr = fs;
        return VFS_OK;
    }

    PRINT_ERR ("failed to mount! \r\n");

err_free:
    if(fs != NULL)
        free(fs);
err:
    fatfs_unregister(s_drive);
    return ret;
}

int ks_fatfs_unmount(const char *path, uint8_t drive)
{
    char dpath[10] = {0};

    sprintf(dpath, "%d:/", drive);
    fatfs_unregister(drive);
    f_mount(NULL, (const TCHAR *)dpath, 1);
    ks_vfs_fs_unmount(path);
    if (fatfs_ptr)
    {
        free(fatfs_ptr);
        fatfs_ptr = NULL;
    }

    return 0;
}


int ks_fatfs_add_mount_change_callback(FatfsMountStatusCbk cb_func)
{

	FatFsMountStatusListCb* precvcb = NULL;
	int count = sizeof(s_fatfs_mount_statuscb)/sizeof(s_fatfs_mount_statuscb[0]);
	for(int i = 0;i<count;i++)
	{
		if(s_fatfs_mount_statuscb[i].is_used==0)
		{
			precvcb = &s_fatfs_mount_statuscb[i];
			precvcb->is_used=1;
			break;
		}
	}

	if(precvcb!=NULL){
		precvcb->cbfunc=cb_func;
		list_add_tail( &(g_FatFsMountStatusChangeCallBackList), &(precvcb->cb_list) );
		if(g_fatfs_ctx.fatfs_inited)
		cb_func(g_fatfs_ctx.card_type,g_fatfs_ctx.mount_status);
		return 0 ;
	}else{
		return -1;
	}

}



#if 0


int init_flag_fatfs = 0;


int ks_fatfs_link(uint8_t dev,char* path)
{
	path[0] = dev + '0';
	path[1] = ':';
	//path[2] = '/';
	path[2] = 0;
	
	return 0;
}


int ks_fatfs_unlink(uint8_t dev,char* path){

	if(path[0] == dev + '0'){
		path[0] = 0;
		path[1] = 0;
		path[2] = 0;
		return 0;
	}
	return -1;
}


int ks_fatfs_mount(char* path){

	FATFS fs;	

	kprintf("ks_fatfs_mount %s \r\n",path);

	FRESULT res = f_mount(&fs,path,1);	  

	/* 如果没有文件系统就格式化创建创建文件系统 */
	if(res == FR_NO_FILESYSTEM)
	{
		 kprintf("no filesystem format...\r\n");
		/* 格式化 */
		 res=f_mkfs(path,0,0);						 
		 
		 if(res == FR_OK)
		 {
			 kprintf("format success\r\n");
			/* 格式化后，先取消挂载 */
			 res = f_mount(NULL,path,1);			 
			/* 重新挂载	 */ 		 
			 res = f_mount(&fs,path,1);
		 }
		 else
		 {
			 kprintf("format failure %d \r\n",res);
		 }
	}
	else if(res!=FR_OK)
	{
		kprintf("mount failure(%d)\r\n",res);
	}
	else
	{
		kprintf("mount ok\r\n");
	}
	return 0;

}


int ks_fatfs_unmount(char* path){

	FRESULT res = f_mount(NULL,path,1);

	if(res != FR_OK)
	{
		kprintf("mount failure(%d)\r\n",res);
		return -1;
	}

	return 0;
}



int ks_fatfs_init(uint8_t dev,char* path)
{
	int ret ;
	if(init_flag_fatfs == 0){
		
		ks_shell_add_cmds(fatfs_cmds, sizeof(fatfs_cmds) / sizeof(cmd_proc_t));

		ks_fatfs_link(dev,g_fatfs_volume_name);
		
		ret =  ks_fatfs_mount(g_fatfs_volume_name);
		if(ret==FR_OK){
			sprintf(g_fatfs_current_path,"%s",g_fatfs_volume_name);
			strcpy(path,g_fatfs_volume_name);
		}

		init_flag_fatfs = 1;
		
		return ret;
	}
	
	return 0;
}


int ks_fatfs_deinit(uint8_t dev,char* path)
{
	int ret ;
	if(init_flag_fatfs == 1){

		ks_fatfs_unmount(g_fatfs_volume_name);
		
		ks_fatfs_unlink(dev,g_fatfs_volume_name);

		ks_shell_remove_cmds(fatfs_cmds, sizeof(fatfs_cmds) / sizeof(cmd_proc_t));

		init_flag_fatfs = 0;

	}

	return 0;
		
}
	


#endif 





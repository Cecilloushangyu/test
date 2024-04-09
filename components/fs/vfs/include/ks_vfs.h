
#ifndef _VFS_H
#define _VFS_H

#include <stdio.h>
#include <stdint.h>
#include "sys/stat.h"
#include "ks_os.h"
#include "ks_mem.h"
#include "ks_printf.h"

#define VFS_MAX_DIR_NAME_LEN                 255
#define VFS_MAX_FILE_NAME_LEN               16
#define VFS_FS_MAX_NAME_LEN                 VFS_MAX_FILE_NAME_LEN
#define VFS_MAX_FILES                       8


#define DIR_PATH_LEN        256
#define DIR_CUR_PATH_LEN    256
#define RETURN_BUF_LEN      2


#define PRINT_ERR kprintf
#define PRINT_INFO kprintf


#define malloc   ks_mem_heap_malloc
#define free   ks_mem_heap_free

struct file;
struct mount_point;
struct dir;
struct dirent;

typedef int                                 ssize_t;
typedef long                                off_t;



#define  FAT_FS   0
#define  NFS_FS   1


#define FATFS_ROOT_PATH          "/fatfs/0:"



struct file_ops
{
    int     (*open)     (struct file *, const char *, int);
    int     (*close)    (struct file *);
    ssize_t (*read)     (struct file *, char *, size_t);
    ssize_t (*write)    (struct file *, const char *, size_t);
    off_t   (*lseek)    (struct file *, off_t, int);
    int     (*stat)     (struct mount_point *, const char *, struct stat *);
    int     (*unlink)   (struct mount_point *, const char *);
    int     (*rename)   (struct mount_point *, const char *, const char *);
    int     (*ioctl)    (struct file *, int, unsigned long);
    int     (*sync)     (struct file *);
    int     (*opendir)  (struct dir *, const char *);
    int     (*readdir)  (struct dir *, struct dirent *);
    int     (*closedir) (struct dir *);
    int     (*mkdir)    (struct mount_point *, const char *);
};

struct file_system
{
    const char            fs_name [VFS_FS_MAX_NAME_LEN];
    struct file_ops     * fs_fops;
    struct file_system  * fs_next;
    volatile uint32_t     fs_refs;
};

struct mount_point
{
    struct file_system  * m_fs;
    struct mount_point  * m_next;
    const char          * m_path;
    volatile uint32_t     m_refs;
    OSHandle             m_mutex;
    void                * m_data;   /* used by fs private data for this mount point (like /sdb1, /sdb2), */
};

#define FILE_STATUS_NOT_USED        0
#define FILE_STATUS_INITING         1
#define FILE_STATUS_READY           2
#define FILE_STATUS_CLOSING         3

#define VFS_TYPE_FILE               0
#define VFS_TYPE_DIR                1



#define  VFS_OK   0
#define  VFS_NOK   1


struct file
{
    struct file_ops    * f_fops;
    uint32_t               f_flags;
    uint32_t               f_status;
    off_t                f_offset;
    struct mount_point * f_mp;      /* can get private mount data here */
    uint32_t               f_owner;   /* the task that openned this file */
    void               * f_data;
};

struct dirent
{
    char                 name [VFS_MAX_DIR_NAME_LEN+1];
    uint32_t               type;
    uint32_t               size;
};

struct dir
{
    struct mount_point * d_mp;      /* can get private mount data here */
    struct dirent        d_dent;
    off_t                d_offset;
    void               * d_data;
};


typedef struct {
    S32 fd;
    S32 flags;
    U32 status;
	S32 current_errno;
    struct file *file;
    char curPath[DIR_CUR_PATH_LEN];
    char curFullPath[DIR_PATH_LEN];
	char rootFullPath[DIR_PATH_LEN];
} Vfs_Ctx;

extern int     ks_vfs_open (const char *, int);
extern int     ks_vfs_close (int);
extern ssize_t ks_vfs_read (int, char *, size_t);
extern ssize_t ks_vfs_write (int, const void *, size_t);
extern off_t   ks_vfs_lseek (int, off_t, int);
extern int     ks_vfs_stat (const char *, struct stat *);
extern int     ks_vfs_unlink (const char *);
extern int     ks_vfs_rename (const char *, const char *);
extern int     ks_vfs_ioctl (int, int, ...);
int            ks_vfs_sync (int fd);
struct dir    *ks_vfs_opendir (const char * path);
struct dirent *ks_vfs_readdir (struct dir * dir);
int            ks_vfs_closedir (struct dir * dir);
int            ks_vfs_mkdir (const char * path, int mode);

extern int     ks_vfs_fs_register (struct file_system *);
extern int     ks_vfs_fs_unregister (struct file_system *);
extern int     ks_vfs_fs_mount (const char *, const char *, void *);
extern int     ks_vfs_fs_unmount (const char *);
extern int     ks_vfs_fs_init (void);

int ks_vfs_init (     const char   filesystemtype);

extern int      open (const char *path, int flags,...);
extern int      close (int fd);
extern ssize_t  read (int fd, void *buff, size_t bytes);
extern ssize_t  write (int fd, const void *buff, size_t bytes);
extern off_t    lseek (int fd, off_t off, int whence);
extern int      stat (const char *path, struct stat *stat);
extern int      unlink (const char *path);
extern int      rename (const char *oldpath, const char *newpath);
extern int      fsync (int fd);
extern struct dir *opendir (const char *path);
extern struct dirent *readdir (struct dir *dir);
extern int      closedir (struct dir *dir);
extern int      mkdir (const char *path, int mode);

#endif

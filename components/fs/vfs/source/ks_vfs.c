
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "sys/errno.h"
#include "sys/fcntl.h"
#include "ks_vfs.h"
#include "ks_os.h"
#include "ks_fatfs.h"

struct file          files [VFS_MAX_FILES];
OSHandle         fs_mutex ;
struct mount_point *mount_points = NULL;
struct file_system *file_systems = NULL;



Vfs_Ctx g_vfs_ctx =
{
    .curPath = FATFS_ROOT_PATH,
    .curFullPath = FATFS_ROOT_PATH,
	.rootFullPath = FATFS_ROOT_PATH

};

#define VFS_ERRNO_SET(err)                  (g_vfs_ctx.current_errno = (err))

static int _file_2_fd (struct file *file)
{
    return file - files;
}

static struct file *_fd_2_file (int fd)
{
    return &files [fd];
}

static struct file *ks_vfs_file_get (void)
{
    int i;

    /* protected by fs_mutex */

    for (i = 0; i < VFS_MAX_FILES; i++)
    {
        if (files[i].f_status == FILE_STATUS_NOT_USED)
        {
            files[i].f_status = FILE_STATUS_INITING;
            return &files[i];
        }
    }

    return NULL;
}

static void ks_vfs_file_put(struct file *file)
{
    file->f_flags  = 0;
    file->f_fops   = NULL;
    file->f_data   = NULL;
    file->f_mp     = NULL;
    file->f_offset = 0;
    file->f_owner  = -1;

    file->f_status = FILE_STATUS_NOT_USED;
}

struct mount_point *ks_vfs_mp_find (const char *path, const char **path_in_mp)
{
    struct mount_point *mp = mount_points;
    struct mount_point *best_mp = NULL;
    int                  best_matches = 0;

    if (path == NULL)
    {
        return NULL;
    }
    if (path_in_mp != NULL)
    {
        *path_in_mp = NULL;
    }

    while (mp != NULL)
    {
        const char      *m_path  = mp->m_path;  /* mount point path */
        const char      *i_path  = path;        /* input path */
        int              matches = 0;
        const char      *t;

        do
        {
            while (*m_path == '/') m_path++;
            while (*i_path == '/') i_path++;

            t = strchr (m_path, '/');

            if (t == NULL)
            {
                /*
                 * m_path now is as follows:
                 * 1) string like "abc"
                 * 2) empty string "\0"
                 * if it is empty string, means current mp matched
                 */

                t = strchr (m_path, '\0');

                if (t == m_path)
                {
                    break;
                }
            }

            if (strncmp (m_path, i_path, t - m_path) != 0)
            {
                goto next;  /* this mount point do not match, check next */
            }

            /*
             * if m_path is "abc", i_path maybe:
             * 1) "abc"
             * 2) "abc/"
             * 3) "abcd..."
             * if it is not 1) or 2), this mp does not match, just goto next one
             */

            i_path  += (t - m_path);

            if ((*i_path != '\0') && (*i_path != '/'))
            {
                goto next;
            }

            matches += (t - m_path);
            m_path  += (t - m_path);
        } while (*m_path != '\0');

        if (matches > best_matches)
        {
            best_matches = matches;
            best_mp      = mp;
            while (*i_path == '/') i_path++;

            if (path_in_mp != NULL)
            {
                *path_in_mp  = i_path;
            }
        }

next:
        mp = mp->m_next;
    }

    return best_mp;
}

int ks_vfs_open (const char *path, int flags)
{
    struct file         *file = NULL;
    int                  fd = -1;
    const char          *path_in_mp = NULL;
    struct mount_point *mp;

    if (path == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        return fd;
    }

    /* can not open dir */

    if (path [strlen (path) - 1] == '/')
    {
        VFS_ERRNO_SET (EINVAL);
        return fd;
    }

    /* prevent fs/mp being removed while opening */

    if (VFS_OK != ks_os_mutex_enter(fs_mutex))
    {
        VFS_ERRNO_SET (EAGAIN);
        return fd;
    }

    file = ks_vfs_file_get ();

    if (file == NULL)
    {
        VFS_ERRNO_SET (ENFILE);
        goto err_post_exit;
    }

    mp = ks_vfs_mp_find (path, &path_in_mp);

    if ((mp == NULL) || (path_in_mp == NULL) || (*path_in_mp == '\0') ||
            (mp->m_fs->fs_fops->open == NULL))
    {
        VFS_ERRNO_SET (ENOENT);
        goto err_post_exit;
    };

    if (VFS_OK != ks_os_mutex_enter (mp->m_mutex))
    {
        VFS_ERRNO_SET (EAGAIN);
        goto err_post_exit;
    }

    ks_os_mutex_leave (fs_mutex);

    file->f_flags  = flags;
    file->f_offset = 0;
    file->f_data   = NULL;
    file->f_fops   = mp->m_fs->fs_fops;
    file->f_mp     = mp;
    file->f_owner  = ks_os_thread_get_current_thread_handle();

    if (file->f_fops->open (file, path_in_mp, flags) == 0)
    {
        mp->m_refs++;
        fd = _file_2_fd (file);
        file->f_status = FILE_STATUS_READY;     /* file now ready to use */
    }
    else
    {
        ks_vfs_file_put (file);
    }

    ks_os_mutex_leave (mp->m_mutex);

    return fd;

err_post_exit:

    ks_os_mutex_leave (fs_mutex);

    if ((fd < 0) && (file != NULL))
    {
        ks_vfs_file_put (file);
    }

    return fd;
}

/* attach to a file and then set new status */

static struct file *_ks_vfs_attach_file (int fd, uint32_t status)
{
    struct file         *file = NULL;

    if ((fd < 0) || (fd >= VFS_MAX_FILES))
    {
        VFS_ERRNO_SET (EBADF);
        return file;
    }

    file = _fd_2_file (fd);

    /*
     * Prevent file closed after the checking of:
     *
     *     if (file->f_status == FILE_STATUS_READY)
     *
     * Because our files are not privated to one task, it may be operated
     * by every task.
     * So we should take the mutex of current mount point before operating it,
     * but for now we don't know if this file is valid (FILE_STATUS_READY), if
     * this file is not valid, the f_mp may be incorrect. so
     * we must check the status first, but this file may be closed/removed
     * after the checking if the senquence is not correct.
     *
     * Consider the following code:
     *
     * ks_vfs_attach_file (...)
     * {
     *     if (file->f_status == FILE_STATUS_READY)
     *     {
     *         while (VFS_MuxPend (file->f_mp->m_mutex, VFS_WAIT_FOREVER) != VFS_OK);
     *
     *         return file;
     *     }
     * }
     *
     * It is not safe:
     *
     * If current task is interrupted by an IRQ just after the checking and then
     * a new task is swapped in and the new task just closed this file.
     *
     * So <fs_mutex> is acquire first and then check if it is valid: if not, just
     * return NULL (which means fail); If yes, the mutex for current mount point
     * is qcquired. And the close operation will also set task to
     * FILE_STATUS_CLOSING to prevent other tasks operate on this file (and also
     * prevent other tasks pend on the mutex of this mount point for this file).
     * At last <fs_mutex> is released. And return the file handle (struct file *).
     *
     * As this logic used in almost all the operation routines, this routine is
     * made to reduce the redundant code.
     */

    while (ks_os_mutex_enter(fs_mutex) != VFS_OK);

    if (file->f_status == FILE_STATUS_READY)
    {
        while (ks_os_mutex_enter (file->f_mp->m_mutex) != VFS_OK);

        if (status != FILE_STATUS_READY)
        {
            file->f_status = status;
        }
    }
    else
    {
        VFS_ERRNO_SET (EBADF);
        file = NULL;
    }

    ks_os_mutex_leave (fs_mutex);

    return file;
}

static struct file *ks_vfs_attach_file (int fd)
{
    return _ks_vfs_attach_file (fd, FILE_STATUS_READY);
}

static struct file *ks_vfs_attach_file_with_status (int fd, int status)
{
    return _ks_vfs_attach_file (fd, status);
}

static uint32_t ks_vfs_detach_file (struct file *file)
{
    return ks_os_mutex_leave (file->f_mp->m_mutex);
}

int ks_vfs_close (int fd)
{
    struct file         *file;
    int                  ret = -1;

    file = ks_vfs_attach_file_with_status (fd, FILE_STATUS_CLOSING);

    if (file == NULL)
    {
        return ret;
    }

    if (file->f_fops->close != NULL)
    {
        ret = file->f_fops->close (file);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    if (0 == ret)
    {
        file->f_mp->m_refs--;
    }

    ks_vfs_detach_file (file);

    ks_vfs_file_put (file);

    return ret;
}

ssize_t ks_vfs_read (int fd, char *buff, size_t bytes)
{
    struct file *file;
    ssize_t       ret = (ssize_t) - 1;

    if (buff == NULL || bytes == 0)
    {
        VFS_ERRNO_SET (EINVAL);
        return ret;
    }

    file = ks_vfs_attach_file (fd);

    if (file == NULL)
    {
        return ret;
    }

    if ((file->f_flags & O_ACCMODE) == O_WRONLY)
    {
        VFS_ERRNO_SET (EACCES);
    }
    else if (file->f_fops->read != NULL)
    {
        ret = file->f_fops->read (file, buff, bytes);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    /* else ret will be -1 */

    ks_vfs_detach_file (file);

    return ret;
}

ssize_t ks_vfs_write (int fd, const void *buff, size_t bytes)
{
    struct file *file;
    ssize_t       ret = -1;

    if (buff == NULL || bytes == 0)
    {
        VFS_ERRNO_SET (EINVAL);
        return ret;
    }

    file = ks_vfs_attach_file (fd);

    if (file == NULL)
    {
        return ret;
    }

    if ((file->f_flags & O_ACCMODE) == O_RDONLY)
    {
        VFS_ERRNO_SET (EACCES);
    }
    else if (file->f_fops->write != NULL)
    {
        ret = file->f_fops->write (file, buff, bytes);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    /* else ret will be -1 */

    ks_vfs_detach_file (file);

    return ret;
}

off_t ks_vfs_lseek (int fd, off_t off, int whence)
{
    struct file *file;
    off_t         ret = -1;

    file = ks_vfs_attach_file (fd);

    if (file == NULL)
    {
        return ret;
    }

    if (file->f_fops->lseek == NULL)
    {
        ret = file->f_offset;
    }
    else
    {
        ret = file->f_fops->lseek (file, off, whence);
    }

    ks_vfs_detach_file (file);

    return ret;
}

int ks_vfs_stat (const char *path, struct stat *stat)
{
    struct mount_point *mp = NULL;
    const char *path_in_mp = NULL;
    int ret = -1;

    if (path == NULL || stat == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        return ret;
    }

    if (VFS_OK != ks_os_mutex_enter(fs_mutex))
    {
        VFS_ERRNO_SET (EAGAIN);
        return ret;
    }

    mp = ks_vfs_mp_find (path, &path_in_mp);

    if ((mp == NULL) || (path_in_mp == NULL) || (*path_in_mp == '\0'))
    {
        VFS_ERRNO_SET (ENOENT);
        ks_os_mutex_leave (fs_mutex);
        return ret;
    }

    if (mp->m_fs->fs_fops->stat != NULL)
    {
        ret = mp->m_fs->fs_fops->stat (mp, path_in_mp, stat);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    ks_os_mutex_leave (fs_mutex);

    return ret;
}

int ks_vfs_unlink (const char *path)
{
    struct mount_point *mp;
    const char          *path_in_mp = NULL;
    int                  ret = -1;

    if (path == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        return ret;
    }

    ks_os_mutex_enter(fs_mutex);   /* prevent the file open/rename */

    mp = ks_vfs_mp_find (path, &path_in_mp);

    if ((mp == NULL) || (path_in_mp == NULL) || (*path_in_mp == '\0') ||
            (mp->m_fs->fs_fops->unlink == NULL))
    {
        VFS_ERRNO_SET (ENOENT);
        goto out;
    }

    ret = mp->m_fs->fs_fops->unlink (mp, path_in_mp);

out:
    ks_os_mutex_leave (fs_mutex);

    return ret;
}

int ks_vfs_rename (const char *old, const char *new)
{
    struct mount_point *mp_old;
    struct mount_point *mp_new;
    const char          *path_in_mp_old = NULL;
    const char          *path_in_mp_new = NULL;
    int                  ret = -1;

    if (old == NULL || new == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        return ret;
    }

    ks_os_mutex_enter(fs_mutex);   /* prevent file open/unlink */

    mp_old = ks_vfs_mp_find (old, &path_in_mp_old);

    if(path_in_mp_old == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        goto out;
    }

    if ((mp_old == NULL) || (*path_in_mp_old == '\0') ||
            (mp_old->m_fs->fs_fops->unlink == NULL))
    {
        VFS_ERRNO_SET (EINVAL);
        goto out;
    }

    mp_new = ks_vfs_mp_find (new, &path_in_mp_new);

    if ((mp_new == NULL) || (path_in_mp_new == NULL) || (*path_in_mp_new == '\0') ||
            (mp_new->m_fs->fs_fops->unlink == NULL))
    {
        VFS_ERRNO_SET (EINVAL);
        goto out;
    }

    if (mp_old != mp_new)
    {
        VFS_ERRNO_SET (EXDEV);
        goto out;
    }

    if (mp_old->m_fs->fs_fops->rename != NULL)
    {
        ret = mp_old->m_fs->fs_fops->rename (mp_old, path_in_mp_old,
                                             path_in_mp_new);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

out:
    ks_os_mutex_leave (fs_mutex);

    return ret;
}

int ks_vfs_ioctl (int fd, int func, ...)
{
    va_list       ap;
    unsigned long arg;
    struct file *file;
    int           ret = -1;

    va_start (ap, func);
    arg = va_arg (ap, unsigned long);
    va_end (ap);

    file = ks_vfs_attach_file (fd);

    if (file == NULL)
    {
        return ret;
    }

    if (file->f_fops->ioctl != NULL)
    {
        ret = file->f_fops->ioctl (file, func, arg);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    ks_vfs_detach_file (file);

    return ret;
}

int ks_vfs_sync (int fd)
{
    struct file *file;
    int           ret = -1;

    file = ks_vfs_attach_file (fd);

    if (file == NULL)
    {
        return ret;
    }

    if (file->f_fops->sync != NULL)
    {
        ret = file->f_fops->sync (file);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    ks_vfs_detach_file (file);

    return ret;
}

struct dir *ks_vfs_opendir (const char *path)
{
    struct mount_point *mp;
    const char          *path_in_mp = NULL;
    struct dir          *dir = NULL;
    int                  ret = -1;

    if (path == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        return NULL;
    }

    dir = (struct dir *) malloc (sizeof (struct dir));

    if (dir == NULL)
    {
        PRINT_ERR ("fail to malloc memory in VFS, <malloc.c> is needed,"
                   "make sure it is added\n");
        VFS_ERRNO_SET (ENOMEM);
        return NULL;
    }

    if (VFS_OK != ks_os_mutex_enter(fs_mutex))
    {
        VFS_ERRNO_SET (EAGAIN);
        free (dir);
        return NULL;
    }

    mp = ks_vfs_mp_find (path, &path_in_mp);

    if (mp == NULL || path_in_mp == NULL)
    {
        VFS_ERRNO_SET (ENOENT);
        ks_os_mutex_leave (fs_mutex);
        free (dir);
        return NULL;
    }

    ret = ks_os_mutex_enter (mp->m_mutex);

    ks_os_mutex_leave (fs_mutex);

    if (ret != VFS_OK)
    {
        VFS_ERRNO_SET (EAGAIN);
        free (dir);
        return NULL;
    }

    if (mp->m_fs->fs_fops->opendir == NULL)
    {
        VFS_ERRNO_SET (ENOTSUP);
        ks_os_mutex_leave (mp->m_mutex);
        free (dir);
        return NULL;
    }

    dir->d_mp     = mp;
    dir->d_offset = 0;

    ret = mp->m_fs->fs_fops->opendir (dir, path_in_mp);

    if (ret == 0)
    {
        mp->m_refs++;
    }
    else
    {
        free (dir);
        dir = NULL;
    }

    ks_os_mutex_leave (mp->m_mutex);

    return dir;
}

struct dirent *ks_vfs_readdir (struct dir *dir)
{
    struct mount_point *mp;
    struct dirent       *ret = NULL;

    if (dir == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        return NULL;
    }

    mp = dir->d_mp;

    if (VFS_OK != ks_os_mutex_enter (mp->m_mutex))
    {
        VFS_ERRNO_SET (EAGAIN);
        return NULL;
    }

    if (dir->d_mp->m_fs->fs_fops->readdir != NULL)
    {
        if (dir->d_mp->m_fs->fs_fops->readdir (dir, &dir->d_dent) == 0)
        {
            ret = &dir->d_dent;
        }
        else
        {
            VFS_ERRNO_SET (EBADF);
        }
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    ks_os_mutex_leave (mp->m_mutex);

    return ret;
}

int ks_vfs_closedir (struct dir *dir)
{
    struct mount_point *mp;
    int                  ret = -1;

    if (dir == NULL)
    {
        VFS_ERRNO_SET (EBADF);
        return -1;
    }

    mp = dir->d_mp;

    if (VFS_OK != ks_os_mutex_enter (mp->m_mutex))
    {
        VFS_ERRNO_SET (EAGAIN);
        return -1;
    }

    if (dir->d_mp->m_fs->fs_fops->closedir != NULL)
    {
        ret = dir->d_mp->m_fs->fs_fops->closedir (dir);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
    }

    if (ret == 0)
    {
        free (dir);
        mp->m_refs--;
    }
    else
    {    
        VFS_ERRNO_SET (EBADF);
    }

    ks_os_mutex_leave (mp->m_mutex);

    return ret;
}

int ks_vfs_mkdir (const char *path, int mode)
{
    struct mount_point *mp;
    const char          *path_in_mp = NULL;
    int                  ret = -1;

    (void) mode;

    if (path == NULL)
    {
        VFS_ERRNO_SET (EINVAL);
        return -1;
    }

    if (VFS_OK != ks_os_mutex_enter(fs_mutex))
    {
        VFS_ERRNO_SET (EAGAIN);
        return -1;
    }

    mp = ks_vfs_mp_find (path, &path_in_mp);

    if ((mp == NULL) || (path_in_mp == NULL) || (*path_in_mp == '\0'))
    {
        VFS_ERRNO_SET (ENOENT);
        ks_os_mutex_leave (fs_mutex);
        return -1;
    }

    ret = ks_os_mutex_enter (mp->m_mutex);

    ks_os_mutex_leave (fs_mutex);

    if (ret != VFS_OK)
    {
        VFS_ERRNO_SET (EAGAIN);
        return -1;
    }

    if (mp->m_fs->fs_fops->mkdir != NULL)
    {
        ret = mp->m_fs->fs_fops->mkdir (mp, path_in_mp);
    }
    else
    {
        VFS_ERRNO_SET (ENOTSUP);
        ret = -1;
    }

    ks_os_mutex_leave (mp->m_mutex);

    return ret;
}

static int ks_vfs_fs_name_check (const char *name)
{
    char ch;
    int  len = 0;

    do
    {
        ch = *name++;

        if (ch == '\0')
        {
            break;
        }

        if ((('a' <= ch) && (ch <= 'z')) ||
                (('A' <= ch) && (ch <= 'Z')) ||
                (('0' <= ch) && (ch <= '9')) ||
                (ch == '_')                  ||
                (ch == '-'))
        {
            len++;

            if (len == VFS_FS_MAX_NAME_LEN)
            {
                return VFS_NOK;
            }

            continue;
        }

        return VFS_NOK;
    } while (1);

    return len == 0 ? VFS_NOK : VFS_OK;
}

static struct file_system *ks_vfs_fs_find (const char *name)
{
    struct file_system *fs;

    for (fs = file_systems; fs != NULL; fs = fs->fs_next)
    {
        if (strncmp(fs->fs_name, name, VFS_FS_MAX_NAME_LEN) == 0)
        {
            break;
        }
    }

    return fs;
}

int ks_vfs_fs_register (struct file_system *fs)
{
    if ((fs == NULL) || (fs->fs_fops == NULL) || (fs->fs_fops->open == NULL))
    {
        return VFS_NOK;
    }

    if (ks_vfs_fs_name_check (fs->fs_name) != VFS_OK)
    {
        return VFS_NOK;
    }

    if (ks_os_mutex_enter(fs_mutex) != VFS_OK)
    {
        return VFS_NOK;
    }

    if (ks_vfs_fs_find (fs->fs_name) != NULL)
    {
        ks_os_mutex_leave (fs_mutex);
        return VFS_NOK;
    }

    fs->fs_next = file_systems;
    file_systems = fs;

    ks_os_mutex_leave (fs_mutex);

    return VFS_OK;
}

int ks_vfs_fs_unregister (struct file_system *fs)
{
    struct file_system *prev;
    int ret = VFS_OK;

    if (fs == NULL)
    {
        return VFS_NOK;
    }

    if (ks_os_mutex_enter(fs_mutex) != VFS_OK)
    {
        return VFS_NOK;
    }

    if (fs->fs_refs > 0)
    {
        goto out;
    }

    if (file_systems == fs)
    {
        file_systems = fs->fs_next;
        goto out;
    }

    prev = file_systems;

    while (prev != NULL)
    {
        if (prev->fs_next == fs)
        {
            break;
        }

        prev = prev->fs_next;
    }

    if (prev == NULL)
    {
        ret = VFS_NOK;
    }
    else
    {
        prev->fs_next = fs->fs_next;
    }

out:
    ks_os_mutex_leave (fs_mutex);

    return ret;
}

int ks_vfs_fs_mount (const char *fsname, const char *path, void *data)
{
    struct file_system *fs;
    struct mount_point *mp;
    const char          *tmp = NULL;

    if (fsname == NULL || path == NULL || path [0] != '/')
    {
        return VFS_NOK;
    }

    ks_os_mutex_enter(fs_mutex);

    fs = ks_vfs_fs_find (fsname);

    if (fs == NULL)
    {
        goto err_post_exit;
    }

    mp = ks_vfs_mp_find (path, &tmp);

    if ((mp != NULL) && (tmp != NULL) && (*tmp == '\0'))
    {
        goto err_post_exit;
    }

    mp = malloc (sizeof (struct mount_point));

    if (mp == NULL)
    {
        PRINT_ERR ("fail to malloc memory in VFS, <malloc.c> is needed,"
                   "make sure it is added\n");
        goto err_post_exit;
    }

    memset (mp, 0, sizeof (struct mount_point));

    mp->m_fs   = fs;
    mp->m_path = path;
    mp->m_data = data;
    mp->m_refs = 0;

    if (VFS_OK != ks_os_mutex_create (&mp->m_mutex,"mount point"))
    {
        goto err_free_exit;
    }

    mp->m_next = mount_points;
    mount_points = mp;

    fs->fs_refs++;

    ks_os_mutex_leave (fs_mutex);

    return VFS_OK;

err_free_exit:
    free (mp);
err_post_exit:
    ks_os_mutex_leave (fs_mutex);
    return VFS_NOK;
}

int ks_vfs_fs_unmount (const char *path)
{
    struct mount_point *mp;
    struct mount_point *prev;
    const char          *tmp = NULL;
    int                  ret = VFS_NOK;

    if (path == NULL)
    {
        return ret;
    }

    ks_os_mutex_enter(fs_mutex);

    mp = ks_vfs_mp_find (path, &tmp);

    if ((mp == NULL) || (tmp == NULL) || (*tmp != '\0') || (mp->m_refs != 0))
    {
        goto post_exit;
    }

    if (mount_points == mp)
    {
        mount_points = mp->m_next;
    }
    else
    {
        for (prev = mount_points; prev != NULL; prev = prev->m_next)
        {
            if (prev->m_next != mp)
            {
                continue;
            }

            prev->m_next = mp->m_next;
            break;
        }
    }

    ks_os_mutex_delete (mp->m_mutex);

    mp->m_fs->fs_refs--;

    free (mp);

    ret = VFS_OK;

post_exit:
    ks_os_mutex_leave (fs_mutex);
    return ret;
}

int ks_vfs_fs_init (void)
{
    if (fs_mutex != 0)
    {
        return VFS_OK;
    }

    if (ks_os_mutex_create (&fs_mutex,"ks_vfs_muxtex") == VFS_OK)
    {
        return VFS_OK;
    }

    PRINT_ERR ("ks_vfs_ks_vfs_init fail!\n");

    return VFS_NOK;
}




int ks_vfs_init (     const char   filesystemtype)
{

    ks_vfs_fs_init ();

	if(filesystemtype == FAT_FS){
		ks_fatfs_init();

	}else{
		return VFS_NOK;
	}
  

    return VFS_OK;
}


#define MAP_TO_POSIX_RET(ret)   ( (ret) < 0 ? -1 : (ret) )

int open (const char *path, int flags,...)
{
    int ret = ks_vfs_open (path, flags);
    return MAP_TO_POSIX_RET(ret);
}

int close (int fd)
{
    int ret = ks_vfs_close (fd);
    return MAP_TO_POSIX_RET(ret);
}

ssize_t read (int fd, void *buff, size_t bytes)
{
    ssize_t ret = ks_vfs_read (fd, buff, bytes);
    return MAP_TO_POSIX_RET(ret);
}

ssize_t write (int fd, const void *buff, size_t bytes)
{
    ssize_t ret = ks_vfs_write (fd, buff, bytes);
    return MAP_TO_POSIX_RET(ret);
}

off_t lseek (int fd, off_t off, int whence)
{
    off_t ret = ks_vfs_lseek (fd, off, whence);
    return MAP_TO_POSIX_RET(ret);
}

int stat (const char *path, struct stat *stat)
{
    int ret = ks_vfs_stat (path, stat);
    return MAP_TO_POSIX_RET(ret);
}

int unlink (const char *path)
{
    int ret = ks_vfs_unlink (path);
    return MAP_TO_POSIX_RET(ret);
}

int rename (const char *oldpath, const char *newpath)
{
    int ret = ks_vfs_rename (oldpath, newpath);
    return MAP_TO_POSIX_RET(ret);
}

int fsync (int fd)
{
    int ret = ks_vfs_sync (fd);
    return MAP_TO_POSIX_RET(ret);
}

struct dir *opendir (const char *path)
{
    return ks_vfs_opendir (path);
}

struct dirent *readdir (struct dir *dir)
{
    return ks_vfs_readdir (dir);
}

int closedir (struct dir *dir)
{
    int ret = ks_vfs_closedir (dir);
    return MAP_TO_POSIX_RET(ret);
}

int mkdir (const char *path, int mode)
{
    int ret = ks_vfs_mkdir (path, mode);
    return MAP_TO_POSIX_RET(ret);
}


#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include "ks_datatypes.h"
#include "ks_vfs.h"
#include "ks_shell.h"
#include "fcntl.h"



extern Vfs_Ctx g_vfs_ctx;


int mkdir_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv)
{
    char tmp_buf[DIR_PATH_LEN] = {0};

    if (argc < 2) {
        ks_shell_printf(ctx->uart_id,"One argument is required at least!\r\n");
        return VFS_NOK;
    }

    sprintf(tmp_buf, "%s/%s", g_vfs_ctx.curFullPath, argv[1]);

    if (mkdir(tmp_buf, 0) == -1) {
        ks_shell_printf(ctx->uart_id,"Mkdir fail.\r\n");
        return VFS_NOK;
    }

    return VFS_OK;
}

int ls_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv)
{
    struct dirent *d_item = NULL;
    struct dir *target = NULL;
    char tmp_buf[DIR_PATH_LEN] = {0};


    sprintf(tmp_buf, "%s", g_vfs_ctx.curFullPath);

    target = opendir(tmp_buf);
    if (target == NULL) {
        ks_shell_printf(ctx->uart_id,"opendir %s  err  !\r\n",tmp_buf);
        return VFS_NOK;
    }

    if (argc == 2) {
        ks_shell_printf(ctx->uart_id,"Name        			Type      Size \r\n");
    }
    do {
        d_item = readdir(target);
        if ((d_item != NULL) && (strlen(d_item->name) > 0)) {
            if (argc == 2) {
                if(strcmp(argv[1], "-l")) {
                    ks_shell_printf(ctx->uart_id,"Argument error! Only support \"-l\"\n");
                    return VFS_NOK;
                }
                if (d_item->type == VFS_TYPE_DIR) {
                    ks_shell_printf(ctx->uart_id,"%-30s  Dir       %-10u\r\n", d_item->name, d_item->size);
                } else {
                    ks_shell_printf(ctx->uart_id,"%-30s  File      %-10u\r\n", d_item->name, d_item->size);
                }
            } else {
                if (d_item->type == VFS_TYPE_DIR) {
                    ks_shell_printf(ctx->uart_id,"%s/    ", d_item->name);
                } else {
                    ks_shell_printf(ctx->uart_id,"%s    ", d_item->name);
                }
            }
        }
    } while((d_item != NULL) && (strlen(d_item->name) > 0));

    ks_shell_printf(ctx->uart_id,"\r\n");
    closedir(target);
    return VFS_OK;
}

int pwd_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv)
{
    ks_shell_printf(ctx->uart_id,"%s\r\n", g_vfs_ctx.curFullPath);
    return VFS_OK;
}

int cd_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv)
{
    char tmp_buf[DIR_PATH_LEN] = {0};
    struct dir *target = NULL;
    char *argPath = NULL;
    char *curPath = NULL;
    S32 pathLen = 0;

    if (argc != 2) {
        ks_shell_printf(ctx->uart_id,"One argument is required!\r\n");
        return VFS_NOK;
    }

    if (argv[1][0] == '/') {
        if (!strcmp(argv[1], "/")) {
            return VFS_NOK;
        }

        target = opendir(argv[1]);
        if(target == NULL) {
            ks_shell_printf(ctx->uart_id,"No such folder.\r\n");
            return VFS_NOK;
        }

        curPath = strrchr(argv[1], '/');
        pathLen = strlen(curPath);

        memset(g_vfs_ctx.curFullPath, 0, strlen(g_vfs_ctx.curFullPath));
        memcpy(g_vfs_ctx.curFullPath, argv[1], strlen(argv[1]));
        memset(g_vfs_ctx.curPath, 0, strlen(g_vfs_ctx.curPath));
        memcpy(g_vfs_ctx.curPath, curPath, pathLen);
    } else if (!strncmp(argv[1], "..", RETURN_BUF_LEN)) {
        g_vfs_ctx.curFullPath[strlen(g_vfs_ctx.curFullPath) - 1] = 0;
        curPath = strrchr(g_vfs_ctx.curFullPath, '/');
        pathLen = strlen(g_vfs_ctx.curFullPath) - strlen(curPath);
        memset(g_vfs_ctx.curFullPath + pathLen, 0, strlen(curPath));

        if (strlen(argv[1]) == RETURN_BUF_LEN || (strlen(argv[1]) == (RETURN_BUF_LEN + 1) && argv[0][2] == '/')) {
            strcat(g_vfs_ctx.curFullPath, "/");
        } else {
            argPath = strchr(argv[1], '/');
            strcat(g_vfs_ctx.curFullPath, argPath);
            if (argPath[strlen(argPath)] != '/') {
                strcat(g_vfs_ctx.curFullPath, "/");
            }
        }

        if (!strcmp(g_vfs_ctx.curFullPath, "/")) {
            return VFS_NOK;
        }

        target = opendir(g_vfs_ctx.curFullPath);

        if(target == NULL) {
            ks_shell_printf(ctx->uart_id,"No such folder.\r\n");
            return VFS_NOK;
        }

        memset(g_vfs_ctx.curPath, 0, strlen(g_vfs_ctx.curPath));
        memcpy(g_vfs_ctx.curPath, curPath, pathLen);
    } else {
        sprintf(tmp_buf, "%s/%s", g_vfs_ctx.curFullPath, argv[1]);
        target = opendir(tmp_buf);
        if(target == NULL) {
            ks_shell_printf(ctx->uart_id,"No such folder.\r\n");
            return VFS_NOK;
        }

        memcpy(g_vfs_ctx.curFullPath,  tmp_buf, strlen(tmp_buf));
        memset(g_vfs_ctx.curPath, 0, strlen(g_vfs_ctx.curPath));
        memcpy(g_vfs_ctx.curPath,  argv[1], strlen(argv[1]));
    }
    closedir(target);
    return VFS_OK;
}

int rm_cmd( cmd_proc_t* ctx,  int argc,  char **argv )
 {	 
	 int ret;
	 char current_path[128];
	 char  *gdir;
	 char  *dir;
	 char  *rm;
	 
	 if ( argc != 2 )
	 {
		 goto usage;
	 }
 
	 ks_shell_printf(ctx->uart_id, "\r\n");
	 strcpy(current_path,g_vfs_ctx.curFullPath);
	 gdir = current_path;
	 if(strcmp(gdir,"/") != 0){
		 dir = strcat(gdir, "/");
	 }else{
		 dir = gdir;
	 }
 
	 rm = strcat(dir, argv[1]);
	 //ks_shell_printf(ctx->uart_id, "rm   %s	\r\n" ,rm);
	 ret = unlink(rm);
	 if(ret==0){
		 ks_shell_printf(ctx->uart_id, "rm %s success! \r\n" ,rm);
	 }
	 else{
		 ks_shell_printf(ctx->uart_id, "rm %s failuer! \r\n" ,rm);
	 }
	 return ret;
 
usage:
	 ks_shell_printf(ctx->uart_id,"usage: %s <file name> \r\n", argv[0] );  
	 return 0;
 }

 int cat_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv )
 {
	 int ret,size;
	 uint8_t rbuffer[101];
	 int fd;

	 char current_path[128];
	 char  *gdir;
	 char  *dir;
	 char  *cat;

	 if(argc != 2)
	 {
		 goto usage;
	 }
 
     struct stat s;
	 ks_shell_printf(ctx->uart_id, "\r\n");
	 strcpy(current_path,g_vfs_ctx.curFullPath);
	 gdir = current_path;
	 if(strcmp(gdir,g_vfs_ctx.rootFullPath) != 0){
		 dir = strcat(gdir, "/");
	 }else{
		 dir = gdir;
	 }
	 cat = strcat(dir, argv[1]);
	 ks_shell_printf(ctx->uart_id, "cat	%s	 \r\n" ,cat);


	 ret = stat(cat, &s);
		
	 /*read*/

	 fd = open(cat, O_RDONLY );
	 if(fd < 0)
	 {
		 ks_shell_printf(ctx->uart_id,"open file error : %d\r\n",ret);
		 return -1;
	 }


	 ks_shell_printf(ctx->uart_id,"file.fsize : %d\r\n",s.st_size);

	 for(int i = 0 ;i<(int)s.st_size;i+=100){
	 	memset(rbuffer,0,sizeof(rbuffer));
		ret = read(fd, rbuffer, 100);
		ks_shell_printf(ctx->uart_id,"%s",rbuffer);
		if(ret<=0) break;
	 }

	 ret = close(fd);

 return 0;
 
	  usage:
	 ks_shell_printf(ctx->uart_id,"usage: %s <file name> \r\n", argv[0] ); 
	 
	 return 0;
 }

 int type_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv )
 {
	 
	 char current_path[128];
	 char  *gdir;
	 char  *dir;
	 char  *type;
	 if(argc != 2)
	 {
		 goto usage;
	 }


	 strcpy(current_path,g_vfs_ctx.curFullPath);
	 gdir = current_path;
	 if(strcmp(gdir,g_vfs_ctx.rootFullPath) != 0){
		 dir = strcat(gdir, "/");
	 }else{
		 dir = gdir;
	 }
	 type = strcat(dir, argv[1]);
	 ks_shell_printf(ctx->uart_id, "create file	%s	 \r\n" ,type);


	 
	 int fd = open(type, O_CREAT|O_RDWR );
	 if(fd < 0 )
	 {
		 ks_shell_printf(ctx->uart_id,"open file error : %d\r\n",fd);
	 }
	 else
	 {
		   close(fd);
	 }

	 return 0;
	 
usage:
	 ks_shell_printf(ctx->uart_id,"usage: %s <file name> \r\n", argv[0] ); 
	 return 0;


 }

 int ctx_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv )
 {

	 ks_shell_printf(ctx->uart_id,"current_errno : %d\r\n",g_vfs_ctx.current_errno);

	 ks_shell_printf(ctx->uart_id,"curFullPath : %s\r\n",g_vfs_ctx.curFullPath);
	 ks_shell_printf(ctx->uart_id,"rootFullPath: %s\r\n",g_vfs_ctx.rootFullPath);
	 return 0;


 }

static cmd_proc_t fs_cmds[] = {

  {.cmd = "ls", .fn = ls_cmd, .next = (void *)0, .help = "fs_list"},
  {.cmd = "pwd", .fn = pwd_cmd, .next = (void *)0, .help = "fs_pwd"},
  {.cmd = "cd", .fn = cd_cmd, .next = (void *)0, .help = "fs_cd"},
  {.cmd = "rm", .fn = rm_cmd, .next = (void *)0, .help = "fs_rm"},
  {.cmd = "mkdir", .fn = mkdir_cmd, .next = (void *)0, .help = "fs_mkdir"},
  {.cmd = "cat", .fn = cat_cmd, .next = (void *)0, .help = "fs_cat"},
  {.cmd = "type", .fn = type_cmd, .next = (void *)0, .help = "fs_type"},
  {.cmd = "ctx", .fn = ctx_cmd, .next = (void *)0, .help = "fs_ctx"},
};

void fs_shell_cmd_init()
{
	ks_shell_add_cmds(fs_cmds, sizeof(fs_cmds) / sizeof(cmd_proc_t));

}




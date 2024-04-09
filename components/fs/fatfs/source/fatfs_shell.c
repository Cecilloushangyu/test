

#include "string.h"
#include "ks_fatfs.h"
#include "ks_printf.h"
#include "ks_shell.h"
#include "sys/errno.h"
#include "sys/fcntl.h"
#include "ks_vfs.h"

char g_fatfs_current_path[1024];

char g_fatfs_volume_name[5];

static int fs_list(cmd_proc_t* ctx,char* path){

  
  FRESULT res;	   //部分在递归过程被修改的变量，不用全局变量  
  FILINFO fno; 
  DIR dir; 
  FIL 	file ;
  int i;		   
  char *fn; 	  // 文件名 
  char current_path[128];
  char *path_dir;
  char *path_file;
  

#if _USE_LFN 
  /* 长文件名支持 */
  /* 简体中文需要2个字节保存一个“字”*/
  static char lfn[_MAX_LFN*2 + 1];	


  fno.lfname = lfn; 
  fno.lfsize = sizeof(lfn); 
#endif 
  //打开目录
  res = f_opendir(&dir, path); 
  if (res == FR_OK) 
  { 
	  i = strlen(path); 
	  for (;;) 
	  { 
		  //读取目录下的内容，再读会自动读下一个文件
		  res = f_readdir(&dir, &fno);							   
		  //为空时表示所有项目读取完毕，跳出
		  if (res != FR_OK || fno.fname[0] == 0) break;    
#if _USE_LFN 
		  fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
		  fn = fno.fname; 
#endif 
		  //点表示当前目录，跳过		   
		  if (*fn == '.') continue;    
		  //目录，递归读取	  

		  //ks_shell_printf(ctx->uart_id, "\r\n\t");
		  ks_shell_printf(ctx->uart_id, "\r\n\t%-50s", fn );
		  
		  ks_shell_printf(ctx->uart_id, "\t");

		  if (fno.fattrib & AM_DIR) 	   
		  {
			  ks_shell_printf(ctx->uart_id, "dir " );

		  }else if(fno.fattrib & AM_ARC){
		  	
			ks_shell_printf(ctx->uart_id, "file");

#if 0
			memset(current_path,0,sizeof(current_path));
			memcpy(current_path, path, strlen(path));
			path_dir = strcat(current_path, "/");
			path_file = strcat(path_dir, fn);

			int ret = f_open(&file, path_file, FA_OPEN_EXISTING | FA_READ);
			if(ret == FR_OK){
				if(file.fsize>1024*1024){
					ks_shell_printf(ctx->uart_id,"\t%dMB",file.fsize/1024/1024);
				}else if(file.fsize>1024){
					ks_shell_printf(ctx->uart_id,"\t%dkB",file.fsize/1024);
				}else{
					ks_shell_printf(ctx->uart_id,"\t%d",file.fsize);
				}
			
			
				f_close(&file);
			}
#endif

		  }else{
			  ks_shell_printf(ctx->uart_id, "\r\n\t?" );

		  }

	  } //for

	  f_closedir(&dir);
 }

  return 0;
}


int  fatfs_fs_chdir(char *pwd,const char *path){
  //go back 
  if(strcmp(path,"..") == 0)
  {
	  char	* ptr = strrchr(pwd,'/');
	  int glen = strlen(pwd);
	  int plen = strlen(ptr);
	  //ks_shell_printf(ctx->uart_id,"glen %d  plen %d	%s \r\n",glen,plen,ptr);
	  pwd[glen-plen] = '\0';
  }
  //go root
  else if (strcmp(path,"/") == 0)
  {
	 memset(pwd,0,sizeof(pwd));
	 strcat(pwd, "/");
  }
  else{
	  if(strcmp(pwd,"/") != 0  )
	  {
		   strcat(pwd, "/");
	  }else{
	  
	  }
	  strcat(pwd, path);
  }

  return 0;

}



 static int fatfs_ls_cmd(cmd_proc_t* ctx,int argc,	   char **argv){

 	ks_shell_printf(ctx->uart_id,"\r\n");

	fs_list(ctx,g_fatfs_current_path);
	
	return 0;

 }

 static int fatfs_pwd_cmd(cmd_proc_t* ctx, int argc,  char **argv )
{
	int ret,size;
	ks_shell_printf(ctx->uart_id,"\r\n");

	ks_shell_printf(ctx->uart_id,"%s",g_fatfs_current_path);

	return 0;
}

 static int fatfs_cd_cmd(cmd_proc_t* ctx, int argc,  char **argv )
 {
	int ret,size;
	DIR dir; 
	ks_shell_printf(ctx->uart_id,"\r\n");
	
	char path[1024];

	if(argc != 2)
	{
	 goto usage;
	}
	
	memset(path,0,sizeof(path));
	memcpy(path,g_fatfs_current_path,strlen(g_fatfs_current_path));
	fatfs_fs_chdir(path,argv[1]);
	//ks_shell_printf(ctx->uart_id,"path %s \r\n",path);
	FRESULT res = f_opendir(&dir, path); 
	if (res != FR_OK) 
	{ 
		ks_shell_printf(ctx->uart_id,"not find dir %s \r\n",argv[1]);
		return -1;
	}
	f_closedir(&dir);

	fatfs_fs_chdir(g_fatfs_current_path,argv[1]);

	fs_list(ctx,g_fatfs_current_path);
	 
	return 0;

 usage:
	  ks_shell_printf(ctx->uart_id,"usage: %s <dir name> \r\n", argv[0] ); 

	 return 0;
 }

 static int fatfs_rm_cmd( cmd_proc_t* ctx,  int argc,  char **argv )
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
	 strcpy(current_path,g_fatfs_current_path);
	 gdir = current_path;
	 if(strcmp(gdir,"/") != 0){
		 dir = strcat(gdir, "/");
	 }else{
		 dir = gdir;
	 }
 
	 rm = strcat(dir, argv[1]);
	 ks_shell_printf(ctx->uart_id, "rm   %s	\r\n" ,rm);
	 ret = f_unlink(rm);
	 if(ret==0){
		 ks_shell_printf(ctx->uart_id, "rm	%s	 success! \r\n" ,rm);
	 }
	 else{
		 ks_shell_printf(ctx->uart_id, "rm	%s	 failuer! \r\n" ,rm);
	 }
	 return ret;
 
usage:
	 ks_shell_printf(ctx->uart_id,"usage: %s <file name> \r\n", argv[0] );  
	 return 0;
 }

 int fatfs_mkdir_cmd( cmd_proc_t* ctx,int argc,  char **argv )
 {	 
	 int ret;
	 char current_path[128];
	 char  *gdir;
	 char  *dir;
	 char  *mkdir;
	 if ( argc != 2 )
	 {
		 goto usage;
	 }
	 
	 ks_shell_printf(ctx->uart_id, "\r\n");
	 strcpy(current_path,g_fatfs_current_path);
	 gdir = current_path;
	 if(strcmp(gdir,"/") != 0){
		  dir = strcat(gdir, "/");
	 }else{
		 dir = gdir;
	 }
	 mkdir = strcat(dir, argv[1]);
 
	 ret = f_mkdir(mkdir);
	 if(ret==0){
		 ks_shell_printf(ctx->uart_id, "mkdir	%s	 success! \r\n" ,mkdir);
	 }
	 else{
		 ks_shell_printf(ctx->uart_id, "mkdir	%s	 failuer! \r\n" ,mkdir);
	 }
	 return ret;
 
usage:
	 ks_shell_printf(ctx->uart_id,"usage: %s <dir name> \r\n", argv[0] );  
	 return 0;
 }

 
 static int fatfs_df_cmd(cmd_proc_t* ctx, int argc,  char **argv )
 {
	int ret,size;
	DWORD fre_clust, fre_sect, tot_sect;
	FIL fnew;													/* 文件对象 */
	FRESULT res;                /* 文件操作结果 */

	FATFS *pfs;
	
	ks_shell_printf(ctx->uart_id,"\r\n");

	/* 获取设备信息和空簇大小 */
	res = f_getfree(g_fatfs_volume_name, &fre_clust, &pfs);

	/* 计算得到总的扇区个数和空扇区个数 */
	tot_sect = (pfs->n_fatent - 2) * pfs->csize;
	fre_sect = fre_clust * pfs->csize;

	/* 打印信息(4096 字节/扇区) */
	ks_shell_printf(ctx->uart_id,"total:%10lu KB  free:%10lu KB\r\n", tot_sect *4, fre_sect *4);


 
	 return 0;
 }



 int fatfs_cat_cmd(struct cmd_proc_t* ctx,  int argc,  char **argv )
 {
	 int ret,size;
	 uint8_t rbuffer[1024];

 	 FIL 	file ;
	 char current_path[128];
	 char  *gdir;
	 char  *dir;
	 char  *cat;
	 UINT   br,bw;		   //读写变量
	 if(argc != 2)
	 {
		 goto usage;
	 }
 
 
	 ks_shell_printf(ctx->uart_id, "\r\n");
	 strcpy(current_path,g_fatfs_current_path);
	 gdir = current_path;
	 if(strcmp(gdir,g_fatfs_volume_name) != 0){
		 dir = strcat(gdir, "/");
	 }else{
		 dir = gdir;
	 }
	 cat = strcat(dir, argv[1]);
	 ks_shell_printf(ctx->uart_id, "cat	%s	 \r\n" ,cat);


	 /*read*/

	 ret = f_open(&file, cat, FA_OPEN_EXISTING | FA_READ);;
	 if(ret != FR_OK)
	 {
		 ks_shell_printf(ctx->uart_id,"open file error : %d\r\n",ret);
		 return -1;
	 }

#if 0
	 ks_shell_printf(ctx->uart_id,"file.fsize : %d\r\n",file.fsize);


	 for(DWORD i =0 ;i<file.fsize;i+=100){
			 ret = f_read(&file, rbuffer, 100, &br);
		 		ks_shell_printf(ctx->uart_id,"%s",rbuffer);
				 if(ret<100) break;
	 }
#endif
	 ret = f_close(&file);

 return 0;
 
	  usage:
	 ks_shell_printf(ctx->uart_id,"usage: %s <file name> \r\n", argv[0] ); 
	 
	 return 0;
 }


 static cmd_proc_t fatfs_cmds[] = {
 
   {.cmd = "ls", .fn = fatfs_ls_cmd, .next = (void *)0, .help = "fs_list"},
   {.cmd = "pwd", .fn = fatfs_pwd_cmd, .next = (void *)0, .help = "fs_pwd"},
   {.cmd = "cd", .fn = fatfs_cd_cmd, .next = (void *)0, .help = "fs_cd"},
   {.cmd = "rm", .fn = fatfs_rm_cmd, .next = (void *)0, .help = "fs_rm"},
   {.cmd = "mkdir", .fn = fatfs_mkdir_cmd, .next = (void *)0, .help = "fs_mkdir"},
   {.cmd = "df", .fn = fatfs_df_cmd, .next = (void *)0, .help = "fs_df"},
   {.cmd = "cat", .fn = fatfs_cat_cmd, .next = (void *)0, .help = "fs_cat"},
 
 };


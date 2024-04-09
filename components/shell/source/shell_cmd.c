
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "shell_api.h"
#include "shell_cmd.h"
#include "ks_section.h"
#include "ks_os.h"
#include "ks_bb.h"
#include "ks_driver.h"
#include "ks_cache.h"


#define VERSION_STR_LEN   30 


char *pdate = __DATE__; 
char *ptime = __TIME__;

#define osKernelSystemId	"ThreadX v6.1.8" 



int shell_check_byteorder()
{
	int a = 1;
	char* p = (char*)&a;
	//返回1表示小端 返回0表示大端
	if(*p == 1)
	return 1;
	else
	return 0;
}

char* shell_strtok(char* strToken, const char* strDelimit) 
{
	static char* text = NULL;
	unsigned char table[32];
	const unsigned char* delimit;
	unsigned char* str;
	char *head;

	if (strToken) text = strToken;


	if (text == NULL) return NULL;
	if (strDelimit == NULL) return text;


	str = (unsigned char*)text;
	delimit = (const unsigned char*)strDelimit;

	for (int i = 0; i < 32; i++) table[i] = 0;
	for (; *delimit; delimit++) {
		table[*delimit >> 3] |= 1 << (*delimit & 7);
	}


	while (*str && (table[*str >> 3] & (1 << (*str & 7)))) str++;
	head = (char*)str;

	for (; *str; str++) {
		if (table[*str >> 3] & (1 << (*str & 7))) {
			*str++ = '\0';
			break;
		}
	}

	text = (char*)str;
	if (*text == '\0') text = NULL;
	return head;
}



char* shell_get_current_os_version (void)
{
    return   osKernelSystemId;
}



char * shell_formate_date(char *pdate)
{
	 static char build_data[32];
	 const char s[2] = " ";
	 char *sday;
	 char *syear;
	 memset(build_data,0,VERSION_STR_LEN);
	 char s_month[4] = "";
	 uint32_t year = 0, day = 0, month = 0, hour = 0;
	 char *str_months = "JanFebMarAprMayJunJulAugSepOctNovDec";
	 strncpy(s_month,pdate,3);
	 strncpy(build_data,pdate+3,strlen(pdate)-3);
 
	 sday = shell_strtok(build_data, s);
	 syear = shell_strtok(NULL, s);
	 ks_shell_str2uint(sday, &day);
	 ks_shell_str2uint(syear, &year);
	 month = 1 + (strstr(str_months, s_month)-str_months)/3;
	 memset(build_data,0,32);
	 sprintf(build_data, "%02d.%02d.%02d", (int)(year%100), (int)month, (int)day);
	 return build_data;
 }
 
 
char * shell_version_formate(char *pdate, char *ptime)
{
	 static char build_version[32];
	 memset(build_version,0,VERSION_STR_LEN);
	 uint32_t hour = 0;
	 ///sscanf(ptime, "%d", &hour);
	 ks_shell_str2uint(ptime, &hour);
	 sprintf(build_version, "V %s.%d ", shell_formate_date(pdate),(int)hour);
	 return build_version;
}
 
char *shell_version_get_app_version(void)
{
	 static char build_app_version[32] ;
	 
	 if(build_app_version[0]==0){
		 strcpy (build_app_version,shell_version_formate(pdate,ptime));  
	 }
	 return build_app_version;
}
 
 
char * shell_version_get_os_version(void)
{
	 static char build_os_version[32];
	 
	 sprintf(build_os_version,shell_get_current_os_version());
 
	 return build_os_version;
 }
 
 
 
char * shell_version_get_work_mode(void)
{
	 if (((uint32_t)shell_version_get_work_mode & 0x01u) == 0x01u)
	 {
		return "THUMB";
	 }else{
		 return "ARM";
	 }
 }
 
char * shell_version_get_endia_mode(void)
{
 
	int mode = shell_check_byteorder();
	if( mode== 1)
	{
	   return "LITTLE_ENDIAN";
	}
	else
	{
	   return "BIG_ENDIAN";
	}
 }
 
 
char * shell_version_get_build_time(){
	 static char build_time[32];
 
	 sprintf(build_time, "%s %s ", shell_formate_date(pdate), ptime);
 
	 return build_time;
 
}
 
static int shell_cmd_version(cmd_proc_t* ctx,int argc,  char **argv)
{
	BBVersionInfo version_info;

	ks_bb_get_version_info(&version_info);
	ks_shell_printf(ctx->uart_id,"\r\n**************************************************\r\n");
	ks_shell_printf(ctx->uart_id,"current project name: %s\r\n", ks_shell_get_project_name());
	ks_shell_printf(ctx->uart_id,"current project buildtime: %s \r\n", shell_version_get_build_time());
	//ks_shell_printf(ctx->uart_id,"current app version: %s\r\n", shell_version_get_app_version());
	//ks_shell_printf(ctx->uart_id,"current os version: %s\r\n", shell_version_get_os_version());
	ks_shell_printf(ctx->uart_id,"current sdk version: %s\r\n", version_info.sdk_version);
	ks_shell_printf(ctx->uart_id,"current fw version: %s\r\n", version_info.fw_version);
	ks_shell_printf(ctx->uart_id,"current ap version: %s\r\n", version_info.ap_version);
	//ks_shell_printf(ctx->uart_id,"current arm workmode: %s\r\n", shell_version_get_work_mode());
	//ks_shell_printf(ctx->uart_id,"current endia mode: %s\r\n", shell_version_get_endia_mode());
	ks_shell_printf(ctx->uart_id,"****************************************************\r\n");

	return 0;
}
 
 
int ks_shell_print_version(int uart_id)
{
	BBVersionInfo version_info;

	ks_bb_get_version_info(&version_info);
	ks_shell_printf(uart_id,"\r\n**************************************************\r\n");
	ks_shell_printf(uart_id,"current project name: %s\r\n", ks_shell_get_project_name());
	ks_shell_printf(uart_id,"current project buildtime: %s \r\n", shell_version_get_build_time());
	//ks_shell_printf(uart_id,"current app version: %s\r\n", shell_version_get_app_version());
	ks_shell_printf(uart_id,"current sdk version: %s\r\n", version_info.sdk_version);
	ks_shell_printf(uart_id,"current fw version: %s\r\n", version_info.fw_version);
	ks_shell_printf(uart_id,"current ap version: %s\r\n", version_info.ap_version);
	//ks_shell_printf(uart_id,"current cpuid : %d\r\n", ks_boot_get_current_cpuid());
	//ks_shell_printf(uart_id,"current os type : %d\r\n", ks_boot_get_current_os_type());
	//ks_shell_printf(uart_id,"current arm workmode: %s\r\n", shell_version_get_work_mode());
	//ks_shell_printf(uart_id,"current endia mode: %s\r\n", shell_version_get_endia_mode());
	//ks_shell_printf(uart_id,"current cpu clock: %d MHZ\r\n", ks_os_get_sys_clock()/1000000);
	ks_shell_printf(uart_id,"****************************************************\r\n");
	return 0;
}

static int shell_cmd_section(cmd_proc_t* ctx,int argc,  char **argv) 
{
 
	 uint32_t code_size = (uint32_t)(&CODE_SECTION_END) - (uint32_t)(&CODE_SECTION_START);
	 uint32_t data_size = (uint32_t)(&DATA_SECTION_END) - (uint32_t)(&DATA_SECTION_START);
	 uint32_t bss_size = (uint32_t)(&BSS_SECTION_END) - (uint32_t)(&BSS_SECTION_START); 
 
//	 uint32_t pool_size = ks_section_get_mem_pool_end() - ks_section_get_mem_pool_start();
//	 uint32_t heap_size = ks_section_get_mem_heap_end() - ks_section_get_mem_heap_start();

	 
	 uint32_t section_size =(uint32_t)(&AP_SECTION_END)- (uint32_t)(&CODE_SECTION_START);
	 
	 ks_shell_printf(ctx->uart_id,"\r\n**************************************************\r\n");
	 ks_shell_printf(ctx->uart_id, "SectionInfo\r\n");
	 ks_shell_printf(ctx->uart_id, "code section start address	%x	end address %x	code_size	%6d (%d K)\r\n",(uint32_t)(&CODE_SECTION_START), (uint32_t)(&CODE_SECTION_END) ,code_size,code_size/1024);
	 ks_shell_printf(ctx->uart_id, "data section start address	%x	end address %x	data_size	%6d (%d K)\r\n",(uint32_t)(&DATA_SECTION_START), (uint32_t)(&DATA_SECTION_END) ,data_size,data_size/1024);
	 ks_shell_printf(ctx->uart_id, "bss  section start address	%x	end address %x	bss_size	%6d (%d K)\r\n",(uint32_t)(&BSS_SECTION_START),(uint32_t)(& BSS_SECTION_END),bss_size,bss_size/1024);
//	 ks_shell_printf(ctx->uart_id, "pool section start address	%x	end address %x	pool_size	%6d (%d K)\r\n",ks_section_get_mem_pool_start(), ks_section_get_mem_pool_end() ,pool_size,pool_size/1024);
//	 ks_shell_printf(ctx->uart_id, "heap section start address	%x	end address %x	heap_size	%6d (%d K)\r\n",ks_section_get_mem_heap_start(), ks_section_get_mem_heap_end() ,heap_size,heap_size/1024);
	 ks_shell_printf(ctx->uart_id, "all　section_start address	%x	end address %x	all_size	%6d (%d K)\r\n",(uint32_t)(&CODE_SECTION_START),(uint32_t)(&AP_SECTION_END),section_size,section_size/1024);

	 ks_shell_printf(ctx->uart_id,"\r\n**************************************************\r\n");

	 return 0 ;
}


static int shell_cmd_help(cmd_proc_t* ctx,int argc, char **argv)
{
    cmd_proc_t *cmd = get_shell_cmds_head();
	int uart_id = ctx->uart_id;
    if (cmd == NULL)
    {
          ks_shell_printf(uart_id, "g_shell_cmds is NULL\r\n");
    }
    if (argc == 1)
    {
        ks_shell_printf(uart_id,"all commands:\r\n");

        do
        {
            ks_shell_printf(uart_id,"  %s\r\n", cmd->cmd);
            cmd = cmd->next;
        } while (cmd);
    }
    else if (argc == 2)
    {
        if (!strcmp(argv[1], "V"))
        {
            do
            {
                ks_shell_printf(uart_id,"%s\r\n", cmd->help);
                cmd = cmd->next;
            } while (cmd);
        }
        else
        {
            while (cmd)
            {
                if (!strcmp(cmd->cmd, argv[1]))
                {
                    ks_shell_printf(uart_id,"  %s\r\n", cmd->help);
                    break;
                }
                cmd = cmd->next;
            }
            if (cmd == NULL)
            {
                return CMD_ERR_PARAM_NOT_EXIST;
            }
        }
    }
    else
        return CMD_ERR_PARAMS_NOT_ENOUGH;

    return CMD_ERR_OK;
}



static int shell_cmd_wreg(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t i;
	int iret;
    uint32_t addr;
    uint32_t value;
    uint32_t len;

   
    len = 0;
    if (argc >= 3 && argc <= 4)
    {
        iret = ks_shell_strhex2uint(argv[1], &addr);
        if (iret != 0)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }

       
        iret =ks_shell_str2uint(argv[2], &value);
        if (iret != 0)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }

        if (argc == 4)
        {
            iret = ks_shell_str2uint(argv[3], &len);
            if (iret != 0)
            {
                return CMD_ERR_PARAMS_FORMAT;
            }
        }
        else
        {
            write_reg((uintptr_t)(addr), value);
            ks_shell_printf(ctx->uart_id,"write reg : %08x << 0x%08x\r\n", addr, value);
            return 0;
        }
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

    //write 4bytes each time
    for (i = 0; i < len; i++)
        write_reg((uintptr_t)(addr + i * sizeof(uint32_t)), value);

    ks_shell_printf(ctx->uart_id,"write reg : %08x - %08x << 0x%08x\r\n", addr, addr + len * sizeof(uint32_t), value);

    return 0;
}

static int shell_cmd_rreg(cmd_proc_t* ctx,int argc,  char **argv)
{
	int nn, multi, iret;
    uint32_t addr;
    uint32_t len;
    uint32_t i;

    len = 1;
	ks_shell_printf(ctx->uart_id,"\r\n");
	
    if (argc >= 2 && argc <= 3)
    {
        iret = ks_shell_strhex2uint(argv[1], &addr);
        if (iret != 0)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }
		
        //addr = addr & ~0x3;

        if (argc == 3)
        {
            iret = ks_shell_str2uint(argv[2], &len);
            if (iret != 0)
            {
                return CMD_ERR_PARAMS_FORMAT;
            }
        }
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

    multi = len / 4;
    for (nn = 0; nn < multi; nn++)
    {
       ks_shell_printf(ctx->uart_id,"0x%08x:  ", addr + nn * 16);
        for (i = 0; i < 4; i++)
        {
           ks_shell_printf(ctx->uart_id,"%08x ", *(uint32_t *)(addr + nn * 16 + i * 4));
        }
       ks_shell_printf(ctx->uart_id,"\r\n");
    }

    len = len - multi * 4;
    if (len)
    {
       ks_shell_printf(ctx->uart_id,"0x%08x:  ", addr + nn * 16);
        for (i = 0; i < len; i++)
        {
           ks_shell_printf(ctx->uart_id,"%08x ", *(uint32_t *)(addr + nn * 16 + i * 4));
        }
    }
   ks_shell_printf(ctx->uart_id,"\r\n");

    return 0;
}

static int shell_cmd_cachei(cmd_proc_t* ctx,int argc,  char **argv)
{
	int nn, multi, iret;
    uint32_t addr;
    uint32_t len;
    uint32_t i;

    len = 1;
    if (argc >= 2 && argc <= 3)
    {
        iret = ks_shell_strhex2uint(argv[1], &addr);
        if (iret != 0)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }

		//addr = addr & ~0x3;

        if (argc == 3)
        {
            iret = ks_shell_str2uint(argv[2], &len);
            if (iret != 0)
            {
                return CMD_ERR_PARAMS_FORMAT;
            }
        }
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

	ks_shell_printf(ctx->uart_id,"ks_cpu_dcache_invalidate addr %x   len %d  \r\n",addr,len);

	 
	ks_cpu_dcache_invalidate((void*)addr,(int)len);

	return 0;
}


static int shell_cmd_cachec(cmd_proc_t* ctx,int argc,  char **argv)
{
	int nn, multi, iret;
    uint32_t addr;
    uint32_t len;
    uint32_t i;

    len = 1;
    if (argc >= 2 && argc <= 3)
    {
        iret = ks_shell_strhex2uint(argv[1], &addr);
        if (iret != 0)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }
        //addr = addr & ~0x3;

        if (argc == 3)
        {
            iret = ks_shell_str2uint(argv[2], &len);
            if (iret != 0)
            {
                return CMD_ERR_PARAMS_FORMAT;
            }
        }
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

	ks_shell_printf(ctx->uart_id,"ks_cpu_dcache_clean addr %x   len %d  \r\n",addr,len);

	 
	ks_cpu_dcache_clean((void*)addr,(int)len);

	return 0;
}




static int shell_cmd_dump(cmd_proc_t* ctx,int argc,  char **argv)
{
	int nn, multi, iret;
    uint32_t addr;
    uint32_t len;
    uint32_t i;

    len = 1;

	ks_shell_printf(ctx->uart_id,"\r\n");
	
    if (argc >= 2 && argc <= 3)
    {
        iret = ks_shell_strhex2uint(argv[1], &addr);
        if (iret != 0)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }
		
        //addr = addr & ~0x3;

        if (argc == 3)
        {
            iret = ks_shell_str2uint(argv[2], &len);
            if (iret != 0)
            {
                return CMD_ERR_PARAMS_FORMAT;
            }
        }
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

	ks_shell_printf(ctx->uart_id,"dump mem addr %x   len %d  \r\n",addr,len);

	 
	ks_shell_printf_dump_hex(ctx->uart_id,(unsigned char * )addr,(int)len);


	return 0;
}

static cmd_proc_t help_cmds = {
	.cmd = "help",	.fn = shell_cmd_help,  .help = "help [cmd-name]"
};


static cmd_proc_t basic_cmds[] = {
    {.cmd = "wreg", .fn = shell_cmd_wreg,  .help = "wreg <addr> <value> [size](word)"},
    {.cmd = "rreg", .fn = shell_cmd_rreg,  .help = "rreg <addr> [size](word)"},
    {.cmd = "cachei", .fn = shell_cmd_cachei,  .help = "cachei <addr> [size](word)"},
	{.cmd = "cachec", .fn = shell_cmd_cachec,  .help = "cachec <addr> [size](word)"},
    {.cmd = "section", .fn = shell_cmd_section, .help = "print section"},
    {.cmd = "ver", .fn = shell_cmd_version,  .help = "print version"},
    {.cmd = "dump", .fn = shell_cmd_dump,  .help = "dump mem"},
};

void shell_cmd_init()
{
	ks_shell_add_cmd(&help_cmds);
	ks_shell_add_cmds(basic_cmds, sizeof(basic_cmds) / sizeof(cmd_proc_t));
}




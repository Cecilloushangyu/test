
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  nvm_demo.c
 *
 * @brief  NVM简单测试例程
 */


#include <string.h>
#include <stdlib.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "ks_shell.h"
#include "middleware/nvm/ks_nvm.h"

#define MAX_FILE_COUNT 1024

PARTITION_HANDLE partition0;
PARTITION_HANDLE partition1;

U8 nvm_read_buffer[4096];
char print_buffer[4096];

const char *file_id_error = "file id error! file_id must be 0~1023\r\n";

static void cmd_nvm_error(int uart_id)
{
    ks_shell_printf(uart_id, "Command format error!\r\n");
    ks_shell_printf(uart_id, "Note that this demo does not support write string with spaces\r\n");
}

static int shell_cmd_nvm(cmd_proc_t *ctx, int argc, char **argv)
{
    int uart_id = ctx->uart_id;
    if (argc == 1)
    {
        ks_shell_printf(uart_id, ctx->help);
        return CMD_ERR_OK;
    }
    else if (argc < 3)
    {
        cmd_nvm_error(uart_id);
        return CMD_ERR_PARAMS_FORMAT;
    }

    U32 partition_id;
    partition_id = strtoul(argv[2], NULL, 10);
    if (partition_id >= 2)
    {
        ks_shell_printf(uart_id, "partition id error! partition_id must be 0 or 1\r\n");
        return CMD_ERR_PARAMS_FORMAT;
    }
    PARTITION_HANDLE handle;
    if (partition_id == 0)
        handle = partition0;
    else
        handle = partition1;

    char *param = argv[1];
    if (strcmp(param, "read") == 0)
    {
        if (argc != 5)
        {
            cmd_nvm_error(uart_id);
            return CMD_ERR_PARAMS_FORMAT;
        }
        U32 file_id;
        U32 length;
        U32 len_out;
        char *ptr;
        file_id = strtoul(argv[3], &ptr, 10);
        length = strtoul(argv[4], &ptr, 10);
        if (file_id >= MAX_FILE_COUNT)
        {
            ks_shell_printf(uart_id, file_id_error);
            return CMD_ERR_PARAMS_FORMAT;
        }
        if (length <= 0 || length > MAX_FILE_SIZE)
        {
            ks_shell_printf(uart_id, "length error! length must be 1~4096\r\n");
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_nvm_read(handle, file_id, nvm_read_buffer, length, &len_out);
        if (ret == 0)
        {
            sprintf(print_buffer, "NVM Read Partition %d File #%d\r\n", partition_id, file_id);
            ks_driver_uart_send_string(uart_id, print_buffer);
            ks_driver_uart_send_buffer(uart_id, nvm_read_buffer, len_out);
            ks_driver_uart_send_string(uart_id, "\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "NVM Read Error!\r\n");
        }
    }
    else if (strcmp(param, "write") == 0)
    {
        if (argc != 5)
        {
            cmd_nvm_error(uart_id);
            return CMD_ERR_PARAMS_FORMAT;
        }
        U32 file_id;
        U32 length;
        char *ptr;
        file_id = strtoul(argv[3], &ptr, 10);
        if (file_id >= MAX_FILE_COUNT)
        {
            ks_shell_printf(uart_id, file_id_error);
            return CMD_ERR_PARAMS_FORMAT;
        }
        length = (int) strlen(argv[4]);
        S32 ret = ks_nvm_write(handle, file_id, argv[4], length);
        if (ret == 0)
        {
            ks_shell_printf(uart_id, "NVM Write OK\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "NVM Write Error!\r\n");
        }
    }
    else if (strcmp(param, "delete") == 0)
    {
        if (argc != 4)
        {
            cmd_nvm_error(uart_id);
            return CMD_ERR_PARAMS_FORMAT;
        }
        U32 file_id;
        char *ptr;
        file_id = strtoul(argv[3], &ptr, 10);
        if (file_id >= MAX_FILE_COUNT)
        {
            ks_shell_printf(uart_id, file_id_error);
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_nvm_delete(handle, file_id);
        if (ret == 0)
        {
            ks_shell_printf(uart_id, "NVM Delete OK\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "NVM Delete Error!\r\n");
        }
    }
    else if (strcmp(param, "format") == 0)
    {
        if (argc != 3)
        {
            cmd_nvm_error(uart_id);
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_nvm_format(handle);
        if (ret == 0)
        {
            ks_shell_printf(uart_id, "NVM Format OK\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "NVM Format Error!\r\n");
        }
    }

    return CMD_ERR_OK;
}

static cmd_proc_t g_shell_nvm_cmd =
        {.cmd = "nvm", .fn = shell_cmd_nvm, .help = "nvm read <partition_id> <file_id> <length>\r\n"
                                                    "nvm write <partition_id> <file_id> <content>\r\n"
                                                    "nvm delete <partition_id> <file_id>\r\n"
                                                    "nvm format <partition_id>\r\n"
                                                    "Note: partition_id can be 0,1; file_id can be 0~1023\r\n"
                                                    "Note: content can not contain spaces\r\n"};

void NvmDemoInit(void)
{
    ks_nvm_create_partition(1536, 448, 1024, &partition0);
    ks_nvm_create_partition(1536 + 448, 64, 1024, &partition1);
    ks_shell_add_cmd(&g_shell_nvm_cmd);
    ks_driver_uart_send_string(0, "Send \"help nvm\" to show usages\r\n");
}


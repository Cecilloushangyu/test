
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  flash_demo.c
 *
 * @brief  flash 测试例程
 */


#include <string.h>
#include <stdlib.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "ks_shell.h"

U8 flash_read_buffer[512*1024];
char print_buffer[4096];

static void cmd_flash_error(int uart_id)
{
    ks_shell_printf(uart_id, "Command format error!\r\n");
    ks_shell_printf(uart_id, "Note that this demo does not support write string with spaces\r\n");
}

static int shell_cmd_flash(cmd_proc_t *ctx, int argc, char **argv)
{
    int uart_id = ctx->uart_id;
    if (argc == 1)
    {
        ks_shell_printf(uart_id, ctx->help);
        return CMD_ERR_OK;
    }
    if (argc != 4)
    {
        cmd_flash_error(uart_id);
        return CMD_ERR_PARAMS_FORMAT;
    }

    unsigned int offset;
    unsigned int size;
    char *ptr;
    offset = strtoul(argv[2], &ptr, 0);

    char *param = argv[1];
    if (strcmp(param, "read") == 0)
    {
        size = strtoul(argv[3], &ptr, 0);
        if (size > sizeof(flash_read_buffer))
        {
            ks_shell_printf(uart_id, "size too large\r\n");
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_flash_read(offset, size, flash_read_buffer);
        if (ret == 0)
        {
            sprintf(print_buffer, "FLASH Read offset %d, size %d\r\n", offset, size);
            ks_driver_uart_send_string(uart_id, print_buffer);
            ks_driver_uart_send_buffer(uart_id, flash_read_buffer, size);
            ks_driver_uart_send_string(uart_id, "\n");
        }
        else
        {
            ks_shell_printf(uart_id, "Flash Read Error %d!\r\n", ret);
        }
    }
    else if (strcmp(param, "write") == 0)
    {
        unsigned int length = strlen(argv[3]);
        S32 ret = ks_flash_write(offset, length + 1, argv[3]);
        if (ret == 0)
        {
            ks_shell_printf(uart_id, "FLASH Write OK\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "Flash Write Error %d!\r\n", ret);
        }
    }
    else if (strcmp(param, "erase") == 0)
    {
        size = strtoul(argv[3], &ptr, 0);
        if ((offset & 0xFFF) || (size & 0xFFF))
        {
            ks_shell_printf(uart_id, "offset and size must be 4K aligned\r\n");
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_flash_erase(offset, size);
        if (ret == 0)
        {
            ks_shell_printf(uart_id, "FLASH Erase OK\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "Flash Erase Error %d!\r\n", ret);
        }
    }
    else if (strcmp(param, "lock") == 0)
    {
        size = strtoul(argv[3], &ptr, 0);
        if ((offset & 0xFFFF) || (size & 0xFFFF))
        {
            ks_shell_printf(uart_id, "offset and size must be 64K aligned\r\n");
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_flash_write_protect_on(offset, size);
        if (ret == 0)
        {
            ks_shell_printf(uart_id, "FLASH Write Protect for the region is ON\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "Flash Lock Error %d!\r\n", ret);
        }
    }
    else if (strcmp(param, "unlock") == 0)
    {
        size = strtoul(argv[3], &ptr, 0);
        if ((offset & 0xFFFF) || (size & 0xFFFF))
        {
            ks_shell_printf(uart_id, "offset and size must be 64K aligned\r\n");
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_flash_write_protect_off(offset, size);
        if (ret == 0)
        {
            ks_shell_printf(uart_id, "FLASH Write Protect for the region is OFF\r\n");
        }
        else
        {
            ks_shell_printf(uart_id, "Flash Unlock Error %d!\r\n", ret);
        }
    }
    else
    {
        cmd_flash_error(uart_id);
        return CMD_ERR_PARAMS_FORMAT;
    }

    return CMD_ERR_OK;
}

static int shell_cmd_otp(cmd_proc_t *ctx, int argc, char **argv)
{
    int uart_id = ctx->uart_id;
    if (argc == 1)
    {
        ks_shell_printf(uart_id, ctx->help);
        return CMD_ERR_OK;
    }
    if (argc != 2 && argc != 3)
    {
        cmd_flash_error(uart_id);
        return CMD_ERR_PARAMS_FORMAT;
    }

    char *param = argv[1];
    if (strcmp(param, "read") == 0)
    {
        if (argc != 3)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }
        int length;
        int len_out;
        char *ptr;
        length = strtol(argv[2], &ptr, 10);
        if (length <= 0 || length > 1024)
        {
            ks_shell_printf(uart_id, "length error\r\n");
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 ret = ks_flash_otp_read(0, flash_read_buffer, length, &len_out);
        if (ret == 0)
        {
            sprintf(print_buffer, "FLASH OTP Read\r\n");
            ks_driver_uart_send_string(uart_id, print_buffer);
            ks_driver_uart_send_buffer(uart_id, flash_read_buffer, len_out);
            ks_driver_uart_send_string(uart_id, "\n");
        }
    }
    else if (strcmp(param, "write") == 0)
    {
        if (argc != 3)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }
        int length;
        char *ptr;
        length = (int) strlen(argv[2]);
        ks_flash_otp_write(0, argv[2], length + 1);
        ks_shell_printf(uart_id, "FLASH OTP Write OK\r\n");
    }
    else if (strcmp(param, "status") == 0)
    {
        if (argc != 2)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }
        char *ptr;
        U32 status;
        S32 ret = ks_flash_otp_get_lock_status(0, &status);
        if (ret == 0)
            ks_shell_printf(uart_id, "FLASH OTP Status is %d\r\n", status);
        else
            ks_shell_printf(uart_id, "FLASH OTP Status READ ERROR: %d\r\n", ret);
    }
    else if (strcmp(param, "lock") == 0)
    {
        if (argc != 3)
        {
            return CMD_ERR_PARAMS_FORMAT;
        }
        unsigned int magic_word = 0;
        char *ptr;
        if (argv[2][0] == '0' && (argv[2][1] == 'x' || argv[2][1] == 'X'))
            magic_word = strtoul(argv[2] + 2, &ptr, 16);
        else
            magic_word = strtoul(argv[2], &ptr, 10);
        if (magic_word != 0xABCD0123)
        {
            ks_shell_printf(uart_id, "Magic word not correct!\r\n");
            return CMD_ERR_PARAMS_FORMAT;
        }
        S32 status = ks_flash_otp_lock(0, magic_word);
        if (status == 0)
            ks_shell_printf(uart_id, "FLASH OTP Status Lock SUCCESS\r\n");
        else
            ks_shell_printf(uart_id, "FLASH OTP Status Lock FAILED\r\n");
    }
    else
    {
        cmd_flash_error(uart_id);
        return CMD_ERR_PARAMS_FORMAT;
    }
    return CMD_ERR_OK;
}

static cmd_proc_t g_shell_flash_cmd =
        {.cmd = "flash", .fn = shell_cmd_flash, .help = "flash read <offset> <size>\r\n"
                                                        "flash write <offset> <content>\r\n"
                                                        "flash erase <offset> <size>\r\n"
                                                        "flash lock <offset> <size>\r\n"
                                                        "flash unlock <offset> <size>\r\n"};
static cmd_proc_t g_shell_otp_cmd =
        {.cmd = "otp", .fn = shell_cmd_otp, .help = "otp read <length>\r\n"
                                                    "otp write <content>\r\n"
                                                    "otp status\r\n"
                                                    "otp lock <magic_word>\r\n"};

void FlashDemoInit(void)
{
    ks_shell_add_cmd(&g_shell_flash_cmd);
    ks_shell_add_cmd(&g_shell_otp_cmd);
    ks_driver_uart_send_string(0, "Send \"help flash\" or \"help otp\" to show usages\r\n");
}


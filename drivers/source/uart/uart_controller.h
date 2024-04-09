/*
 * uart_controller.h
 *
 *  Created on: 2021年9月24日
 *      Author: zhangxin
 */

#ifndef PLATFORM_DRIVER_UART_UART_CONTROLLER_H_
#define PLATFORM_DRIVER_UART_UART_CONTROLLER_H_

#include <ks_uart.h>


S32 _uart_init(int uart_id,int baud,int sel);
S32 _uart_config(int uart_id, int baud, int data_width, int parity, int stop_bits);
S32 _uart_get_baudrate(int uart_id);
S32 _uart_get_stopbit(int uart_id);
S32 _uart_get_datawidth(int uart_id);
S32 _uart_get_parity(int uart_id);
S32 _uart_send_buffer(int uart_id, const U8 *p_data, int len);
S32 _uart_send_string(int uart_id, const char send_string[]);

int _get_tx_buffer_length_for_uart(int uart_id);


int _uart_register_protocol(int uart_id, UartEntry p_parse_input);
int _uart_deregister_protocol(int uart_id, UartEntry p_parse_input);


void _uart_poll_init(int uart_id, int baud,int sel);
void _uart_enter_poll_mode(int uart_id);
void _uart_poll_send_byte(int uart_id, char data);
void _uart_poll_send_string(int uart_id, char send_string[]);
void _uart_poll_send_buffer(int uart_id, char *p_data, int len);
char _uart_poll_rcv_byte(int uart_id);

#endif /* PLATFORM_DRIVER_UART_UART_CONTROLLER_H_ */

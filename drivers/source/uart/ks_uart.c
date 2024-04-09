#include "ks_uart.h"
#include "uart_controller.h"


S32 ks_driver_uart_init(U32 uart_id, U32 baud,U32 sel)
{
	return _uart_init(uart_id,baud,sel);

}

S32 ks_driver_uart_config(U32 uart_id, U32 baud)
{
	return _uart_config(uart_id, baud, _uart_get_datawidth(uart_id),_uart_get_parity(uart_id),_uart_get_stopbit( uart_id));
}

S32 ks_driver_uart_get_baud(U32 uart_id)
{
	return _uart_get_baudrate(uart_id);
}

S32 ks_driver_uart_get_datawidth(U32 uart_id)
{
	return _uart_get_datawidth(uart_id);
}

S32 ks_driver_uart_get_parity(U32 uart_id)
{
	return _uart_get_parity(uart_id);
}

S32 ks_driver_uart_get_stopbit(U32 uart_id)
{
	return _uart_get_stopbit(uart_id);
}

S32 ks_driver_uart_config_ex(U32 uart_id, U32 baud, uart_data_bits_t data_bits,uart_parity_t parity, uart_stop_bits_t stop_bits){
	return _uart_config(uart_id, baud, data_bits,parity,stop_bits);
}

S32 ks_driver_uart_send_buffer(U32 uart_id, const U8 *p_data, S32 len)
{
	return _uart_send_buffer(uart_id, p_data, len);
}


S32 ks_driver_uart_send_string(U32 uart_id, const char *p_string)
{
	return _uart_send_string(uart_id, p_string);
}

S32 ks_driver_uart_get_send_buffer_size(U32 uart_id)
{
	return _get_tx_buffer_length_for_uart(uart_id);
}

void ks_driver_uart_register_protocol(U32 uart_id, UartEntry p_parser)
{
	_uart_register_protocol(uart_id, p_parser);
}

void ks_driver_uart_deregister_protocol(U32 uart_id, UartEntry p_parser)
{
	_uart_deregister_protocol(uart_id, p_parser);
}


void ks_driver_uart_poll_init(U32 uart_id, U32 baud,U32 sel){

	_uart_poll_init(uart_id,baud,sel);

}

void ks_driver_uart_poll_send_buffer(U32 uart_id, char *p_data, S32 len)
{
	_uart_poll_send_buffer(uart_id, p_data, len);
}

void ks_driver_uart_poll_send_byte(U32 uart_id, char data)
{
	_uart_poll_send_byte(uart_id, data);
}

void ks_driver_uart_poll_send_string(U32 uart_id, char send_string[])
{
	_uart_poll_send_string(uart_id, send_string);
}

char ks_driver_uart_poll_rvc_byte(U32 uart_id)
{
	return _uart_poll_rcv_byte(uart_id);
}

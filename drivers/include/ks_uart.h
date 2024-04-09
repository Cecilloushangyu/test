#ifndef INTERFACE_INCLUDE_AP_UART_H_
#define INTERFACE_INCLUDE_AP_UART_H_

#include "ks_datatypes.h"
#include "ks_driver.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----- UART Control Codes: Mode Parameters: Data Bits -----*/
typedef enum {
    UART_DATA_BITS_5               = 5,    ///< 5 Data bits
    UART_DATA_BITS_6,                      ///< 6 Data bit
    UART_DATA_BITS_7,                      ///< 7 Data bits
    UART_DATA_BITS_8,                      ///< 8 Data bits (default)
    UART_DATA_BITS_9                       ///< 9 Data bits
} uart_data_bits_t;

/*----- UART Control Codes: Mode Parameters: Parity -----*/
typedef enum {
    UART_PARITY_NONE               = 0,    ///< No Parity (default)
    UART_PARITY_EVEN,                      ///< Even Parity
    UART_PARITY_ODD,                       ///< Odd Parity
    UART_PARITY_MARK,                      ///< Mark Parity
    UART_PARITY_SPACE,                     ///< Space Parity
} uart_parity_t;

/*----- UART Control Codes: Mode Parameters: Stop Bits -----*/
typedef enum {
    UART_STOP_BITS_1               = 1,    ///< 1 Stop bit (default)
    UART_STOP_BITS_2,                      ///< 2 Stop bits
    UART_STOP_BITS_1_5,                    ///< 1.5 Stop bits
} uart_stop_bits_t;



typedef S32 (*UartEntry)(S32, char);
S32 ks_driver_uart_init(U32 uart_id, U32 baud,U32 sel);
S32 ks_driver_uart_config(U32 uart_id, U32 baud);
S32 ks_driver_uart_config_ex(U32 uart_id, U32 baud, uart_data_bits_t data_bits,uart_parity_t parity, uart_stop_bits_t stop_bits);
S32 ks_driver_uart_get_baud(U32 uart_id);
S32 ks_driver_uart_get_stopbit(U32 uart_id);
S32 ks_driver_uart_get_parity(U32 uart_id);
S32 ks_driver_uart_get_datawidth(U32 uart_id);
S32 ks_driver_uart_send_buffer(U32 uart_id, const U8 *p_data, S32 len);
S32 ks_driver_uart_send_string(U32 uart_id, const char *p_string);
S32 ks_driver_uart_get_send_buffer_size(U32 uart_id);
void ks_driver_uart_register_protocol(U32 uart_id, UartEntry p_parser);
void ks_driver_uart_deregister_protocol(U32 uart_id, UartEntry p_parser);

void ks_driver_uart_poll_init(U32 uart_id, U32 baud,U32 sel);
void ks_driver_uart_poll_send_buffer(U32 uart_index, char *p_data, S32 len);
void ks_driver_uart_poll_send_byte(U32 uart_index, char data);
void ks_driver_uart_poll_send_string(U32 uart_index, char send_string[]);
char ks_driver_uart_poll_rvc_byte(U32 uart_index);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_INCLUDE_AP_UART_H_ */

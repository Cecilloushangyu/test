
#ifndef KS_DRIVER_H_
#define KS_DRIVER_H_
#include <stddef.h>
#include <stdint.h>
#include "ks_datatypes.h"
#include "delos_soc_addr.h"
#include "ks_printf.h"
#include "ks_exception.h"
#include "ks_shell.h"


#define MMU_NO_CACHE_OFFSET 0xC0000000

#define CACHE_LINE_ALIGN_LEN   64 //cacheline 对齐

#define HW_REG_WR(x) 			(*((volatile unsigned int *)(x)))
#define HW_REG_RD( _register_, _value_ ) 	((_value_) = *((volatile unsigned int *)(_register_)))

#define read_reg( addr )  (*((volatile uint32_t *) (addr)))
#define write_reg( addr , value)  ( *(volatile uint32_t *)(addr) = (uint32_t)value)

#define     __IM     volatile const       /*! Defines 'read only' structure member permissions */
#define     __OM     volatile             /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile             /*! Defines 'read / write' structure member permissions */

#define   __IO    volatile             /*!< Defines 'read / write' permissions */
#define   __I     volatile    



/* driver General error codes */
typedef enum {
    DRV_OK = 0,   ///< Unspecified error
    DRV_ERROR = -1,   ///< Unspecified error
    DRV_ERROR_BUSY = -2,                ///< Driver is busy
    DRV_ERROR_TIMEOUT= -3,             ///< Timeout occurred
    DRV_ERROR_UNSUPPORTED = -4,         ///< Operation not supported
    DRV_ERROR_PARAMETER = -5,           ///< Parameter error
    DRV_ERROR_STATUS = -6            ///<  Operation error status
} drv_err_e;

#define TX_DMA_ENABLE (1<<4)
#define RX_DMA_ENABLE (1<<5)

#endif


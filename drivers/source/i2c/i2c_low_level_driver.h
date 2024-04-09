


#ifndef _IIC_H_
#define _IIC_H_


#include <stdint.h>
#include <stdbool.h>
#include "ks_datatypes.h"
#include "ks_i2c.h"
#include "synop_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif


/// definition for iic handle.
typedef void *iic_handle_t;

typedef enum {
    DW_IIC_STANDARDSPEED = 1,
    DW_IIC_FASTSPEED     = 2,
    DW_IIC_FASTPLUSSPEED = 3,
    DW_IIC_HIGHSPEED     = 4
} DW_IIC_SPEED;

/**
\brief IIC Status
*/
typedef struct {
    U32 busy             : 1;        ///< Transmitter/Receiver busy flag
    U32 mode             : 1;        ///< Mode: 0=Slave, 1=Master
    U32 direction        : 1;        ///< Direction: 0=Transmitter, 1=Receiver
    U32 general_call     : 1;        ///< General Call(address 0) indication (cleared on start of next Slave operation)
    U32 arbitration_lost : 1;        ///< Master lost arbitration(in case of multi-masters) (cleared on start of next Master operation)
    U32 bus_error        : 1;        ///< Bus error detected (cleared on start of next Master/Slave operation)
} iic_status_t;

/****** IIC Event *****/
typedef enum {
    IIC_EVENT_TRANSFER_DONE        = 0,  ///< Master/Slave Transmit/Receive finished
    IIC_EVENT_TRANSFER_INCOMPLETE  = 1,  ///< Master/Slave Transmit/Receive incomplete transfer
    IIC_EVENT_SLAVE_TRANSMIT       = 2,  ///< Slave Transmit operation requested
    IIC_EVENT_SLAVE_RECEIVE        = 3,  ///< Slave Receive operation requested
    IIC_EVENT_ADDRESS_NACK         = 4,  ///< Address not acknowledged from Slave
    IIC_EVENT_GENERAL_CALL         = 5,  ///< General Call indication
    IIC_EVENT_ARBITRATION_LOST     = 6,  ///< Master lost arbitration
    IIC_EVENT_BUS_ERROR            = 7,  ///< Bus error detected (START/STOP at illegal position)
    IIC_EVENT_BUS_CLEAR            = 8   ///< Bus clear finished
} iic_event_e;

typedef enum {
    IIC_TXRX = 0,
    IIC_TX   = 1,
    IIC_RX   = 2,
    IIC_EERX = 3
} iic_trans_e;


typedef void (*iic_event_cb_t)(S32 idx, iic_event_e event,void* arg);  ///< Pointer to \ref iic_event_cb_t : IIC Event call back.

iic_handle_t _iic_get_handle(int32_t idx);

int32_t _iic_register_event_cb(iic_handle_t handle,iic_event_cb_t cb_event,void* arg);

/**
  \brief       Initialize IIC Interface . \n
               1. Initializes the resources needed for the IIC interface 
  \param[in]   handle  iic handle to operate.
  \return      0 for success, negative for error code
*/
void _iic_initialize(iic_handle_t handle,i2c_dev_t *i2c);

/**
  \brief       De-initialize IIC Interface. stops operation and releases the software resources used by the interface
  \param[in]   handle  iic handle to operate.
  \return      0 for success, negative for error code
*/
int32_t _iic_uninitialize(iic_handle_t handle);



/**
  \brief       config iic attributes.
  \param[in]   handle    iic handle to operate.
  \param[in]   mode      iic mode \ref iic_mode_e. if negative, then this attribute not changed.
  \param[in]   speed     iic speed \ref iic_speed_e. if negative, then this attribute not changed.
  \param[in]   addr_mode iic address mode \ref iic_address_mode_e. if negative, then this attribute not changed.
  \param[in]   slave_addr iic address of slave. if negative, then this attribute not changed.
  \return      0 for success, negative for error code
*/
int32_t _iic_config(iic_handle_t handle,
                       iic_mode_e mode,
                       iic_speed_e speed,
                       iic_address_mode_e addr_mode,
                       int32_t slave_addr);

/**
  \brief       Start transmitting data as IIC Master.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref _iic_get_status can get operating status.
  \param[in]   handle         iic handle to operate.
  \param[in]   devaddr        iic addrress of slave device. |_BIT[7:1]devaddr_|_BIT[0]R/W_|
                              eg: BIT[7:0] = 0xA0, devaddr = 0x50.
  \param[in]   data           data to send to IIC Slave
  \param[in]   num            Number of data items to send

  \return      0 for success, negative for error code
*/
int32_t _iic_master_send(iic_handle_t handle, uint32_t devaddr, const void *data, uint32_t num);

/**
  \brief       Start receiving data as IIC Master.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref _iic_get_status can get operating status.
  \param[in]   handle  iic handle to operate.
  \param[in]   devaddr        iic addrress of slave device.
  \param[out]  data    Pointer to buffer for data to receive from IIC receiver
  \param[in]   num     Number of data items to receive
  \return      0 for success, negative for error code
*/
int32_t _iic_master_receive(iic_handle_t handle, uint32_t devaddr, void *data, uint32_t num);

/**
  \brief       Start transmitting data as IIC Slave.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref _iic_get_status can get operating status.
  \param[in]   handle  iic handle to operate.
  \param[in]   data  Pointer to buffer with data to transmit to IIC Master
  \param[in]   num   Number of data items to send
  \return      0 for success, negative for error code
*/
int32_t _iic_slave_send(iic_handle_t handle, const void *data, uint32_t num);

/**
  \brief       Start receiving data as IIC Slave.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref _iic_get_status can get operating status.
  \param[in]   handle  iic handle to operate.
  \param[out]  data  Pointer to buffer for data to receive from IIC Master
  \param[in]   num   Number of data items to receive
  \return      0 for success, negative for error code
*/
int32_t _iic_slave_receive(iic_handle_t handle, void *data, uint32_t num);

/**
  \brief       abort transfer.
  \param[in]   handle  iic handle to operate.
  \return      0 for success, negative for error code
*/
int32_t _iic_abort_transfer(iic_handle_t handle);

/**
  \brief       Get IIC status.
  \param[in]   handle  iic handle to operate.
  \return      IIC status \ref iic_status_t
*/
iic_status_t _iic_get_status(iic_handle_t handle);


/**
  \brief       config iic mode.
  \param[in]   handle  iic handle to operate.
  \param[in]   mode      \ref iic_mode_e
  \return      error code
*/
int32_t _iic_config_mode(iic_handle_t handle, iic_mode_e mode);

/**
  \brief       config iic speed.
  \param[in]   handle  iic handle to operate.
  \param[in]   speed     \ref iic_speed_e
  \return      error code
*/
int32_t _iic_config_speed(iic_handle_t handle, iic_speed_e speed);

/**
  \brief       config iic address mode.
  \param[in]   handle  iic handle to operate.
  \param[in]   addr_mode \ref iic_address_mode_e
  \return      error code
*/
int32_t _iic_config_addr_mode(iic_handle_t handle, iic_address_mode_e addr_mode);


/**
  \brief       config iic slave address.
  \param[in]   handle  iic handle to operate.
  \param[in]   slave_addr slave address
  \return      error code
*/
int32_t _iic_config_slave_addr(iic_handle_t handle, int32_t slave_addr);

/**
  \brief       Get IIC transferred data count.
  \param[in]   handle  iic handle to operate.
  \return      the number of the currently transferred data items
*/
uint32_t _iic_get_data_count(iic_handle_t handle);

/**
  \brief       Send START command.
  \param[in]   handle  iic handle to operate.
  \return      error code
*/
int32_t _iic_send_start(iic_handle_t handle);

/**
  \brief       Send STOP command.
  \param[in]   handle  iic handle to operate.
  \return      error code
*/
int32_t _iic_send_stop(iic_handle_t handle);


int32_t _iic_reset(iic_handle_t handle);


#ifdef __cplusplus
}
#endif

#endif /* _CSI_IIC_H_ */

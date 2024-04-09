#ifndef INTERFACE_INCLUDE_KS_I2C_H_
#define INTERFACE_INCLUDE_KS_I2C_H_

#include "ks_datatypes.h"
#include "stdbool.h"
#include "ks_driver.h"
#include "ks_os.h"

#define CONFIG_IIC_NUM                  1



/*----- IIC Control Codes: Mode -----*/
typedef enum {
    IIC_MODE_MASTER,             ///< IIC Master
    IIC_MODE_SLAVE               ///< IIC Slave
} iic_mode_e;



/*----- IIC Control Codes: IIC Bus Speed -----*/
typedef enum {
    IIC_BUS_SPEED_STANDARD  = 1, ///< Standard Speed (100kHz)
    IIC_BUS_SPEED_FAST      = 2, ///< Fast Speed     (400kHz)
    IIC_BUS_SPEED_FAST_PLUS = 3, ///< Fast+ Speed    (  1MHz)
    IIC_BUS_SPEED_HIGH      = 4 ///< High Speed     (3.4MHz)  
} iic_speed_e;

/*----- IIC Control Codes: IIC Address Mode -----*/
typedef enum {
    IIC_ADDRESS_7BIT        = 0,  ///< 7-bit address mode
    IIC_ADDRESS_10BIT       = 1   ///< 10-bit address mode
} iic_address_mode_e;


/**
  \enum        iic_mem_addr_size_t
  \brief       iic memory address size
 */
typedef enum {
    IIC_MEM_ADDR_SIZE_8BIT         = 0U,    ///< IIC e2prom  8bit address mode
    IIC_MEM_ADDR_SIZE_16BIT                 ///< IIC e2prom  16bit address mode
} iic_mem_addr_size_t;




typedef struct {
	iic_mode_e mode;
	iic_speed_e speed;
	iic_address_mode_e addr_mode;
	S32 slave_addr;
} i2c_config_t;


/* This struct define i2c main handle */

typedef struct {
	U32		 inited;
    U32       port;   /**< i2c port */
    i2c_config_t  config; /**< i2c config */
    void         *handle;   /**< i2d handle */
	OSHandle sem;
	OSHandle mutex;
	uint8_t tx_dma_enable;
	uint8_t rx_dma_enable;
	S32  tx_dma_ch;
	S32  rx_dma_ch;

} i2c_dev_t;


/**
  \brief       Initialize IIC Interface . \n
               1. Initializes the resources needed for the IIC interface 
  \param[in]   i2c_id    iic port to operate.
  \return      0 for success, negative for error code
*/

S32 ks_driver_i2c_init(U32 i2c_id,U32 sel);


/**
  \brief       config iic attributes.
  \param[in]   i2c_id    iic port to operate.
  \param[in]   config    struct i2c_config_t.
  \return      0 for success, negative for error code
*/

S32 ks_driver_i2c_config(U32 i2c_id,i2c_config_t* config);

/**
  \brief       Start transmitting data as IIC Master.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref ks_driver_i2c_get_status can get operating status.
  \param[in]   i2c_id    iic port to operate.
  \param[in]   devaddr        iic addrress of slave device. |_BIT[7:1]devaddr_|_BIT[0]R/W_|
                              eg: BIT[7:0] = 0xA0, devaddr = 0x50.
  \param[in]   data           data to send to IIC Slave
  \param[in]   num            Number of data items to send
  \return      0 for success, negative for error code
*/
S32 ks_driver_i2c_master_send(U32 i2c_id, U32 devaddr, const void *data, U32 num,U32 timeout);

/**
  \brief       Start receiving data as IIC Master.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref ks_driver_i2c_get_status can get operating status.
  \param[in]   i2c_id    iic port to operate..
  \param[in]   devaddr        iic addrress of slave device.
  \param[out]  data    Pointer to buffer for data to receive from IIC receiver
  \param[in]   num     Number of data items to receive
  \return      0 for success, negative for error code
*/
S32 ks_driver_i2c_master_receive(U32 i2c_id, U32 devaddr, void *data, U32 num,U32 timeout);

/**
  \brief       Start transmitting data as IIC Slave.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref ks_driver_i2c_get_status can get operating status.
  \param[in]   i2c_id    iic port to operate..
  \param[in]   data  Pointer to buffer with data to transmit to IIC Master
  \param[in]   num   Number of data items to send
  \return      0 for success, negative for error code
*/
S32 ks_driver_i2c_slave_send(U32 i2c_id, const void *data, U32 num,U32 timeout);

/**
  \brief       Start receiving data as IIC Slave.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref ks_driver_i2c_get_status can get operating status.
  \param[in]   i2c_id    iic port to operate..
  \param[out]  data  Pointer to buffer for data to receive from IIC Master
  \param[in]   num   Number of data items to receive
  \return      0 for success, negative for error code
*/
S32 ks_driver_i2c_slave_receive(U32 i2c_id, void *data, U32 num,U32 timeout);




#endif /* INTERFACE_INCLUDE_KS_I2C_H_ */

#ifndef INTERFACE_INCLUDE_AP_SPI_H_
#define INTERFACE_INCLUDE_AP_SPI_H_

#include "ks_datatypes.h"
#include "ks_driver.h"
#include "ks_os.h"

#define CONFIG_SPIM_NUM 1
#define CONFIG_SPIS_NUM 1



/*----- SPI Control Codes: Mode -----*/
typedef enum {
    SPI_MODE_MASTER,             ///< SPI Master (Output on MOSI, Input on MISO)
    SPI_MODE_SLAVE,              ///< SPI Slave  (Output on MISO, Input on MOSI)
} spi_mode_e;

/*----- SPI Control Codes: Mode Parameters: Frame Format -----*/
typedef enum {
    SPI_FORMAT_CPOL0_CPHA0 = 0, ///< Clock Polarity 0, Clock Phase 0
    SPI_FORMAT_CPOL0_CPHA1,     ///< Clock Polarity 0, Clock Phase 1
    SPI_FORMAT_CPOL1_CPHA0,     ///< Clock Polarity 1, Clock Phase 0
    SPI_FORMAT_CPOL1_CPHA1,     ///< Clock Polarity 1, Clock Phase 1
} spi_format_e;




typedef struct
{
	int32_t 		 	baud;
	spi_mode_e 	  		mode;
	spi_format_e	  	format;
	int32_t		  		bit_width;
} spi_config_t;


typedef struct {
	U8		 inited;
    U8       port;   /**< spi port */
   	spi_config_t  config; /**< spi config */
    void         *handle;   /**< spi handle */
	OSHandle sem;
	OSHandle mutex;
} spi_dev_t;


#define SPI_CS0  0b0000000000000001 	
#define SPI_CS1  0b0000000000000010
#define SPI_CS2  0b0000000000000100
#define SPI_CS3  0b0000000000001000
#define SPI_CS4  0b0000000000010000 	
#define SPI_CS5  0b0000000000100000
#define SPI_CS6  0b0000000001000000
#define SPI_CS7  0b0000000010000000
#define SPI_CS8  0b0000000100000000
#define SPI_CS9  0b0000001000000000

/**
  \brief       Initialize SPI Master Interface.
  \param[in]   spi_id  spi port
  \return      drv_err_e code
*/
S32 ks_driver_spi_master_init(U32 spi_id);
/**
  \brief       config spi port.
  \param[in]   spi_id  spi port.
  \param[in]   config   ref strcuct  spi_config_t
  \return      drv_err_e code
*/

S32 ks_driver_spi_master_config(U32 spi_id,	spi_config_t   *config);
/**
  \brief       sending data to SPI transmitter,(received data is ignored).
  \param[in]   spi_id  spi port.
  \param[in]   data  Pointer to buffer with data to send to SPI transmitter. 
  \param[in]   num   Number of data items to send.
  \param[in]  timeout   SPI transmitter/receiver expire time 
  \return      drv_err_e code
*/
S32 ks_driver_spi_master_send(U32 spi_id, const uint8_t *data, uint16_t size, uint32_t timeout);

/**
\brief      receiving data from SPI receiver. 
\param[in]	 spi_id  spi port.
\param[out] data  Pointer to buffer for data to receive from SPI receiver
\param[in]  num   Number of data items to receive
\param[in]  timeout   SPI transmitter/receiver expire time 
\return     drv_err_e code
*/
S32 ks_driver_spi_master_receive(U32 spi_id, uint8_t *data, uint16_t size, uint32_t timeout);


/**
  \brief       sending/receiving data to/from SPI transmitter/receiver.
               if non-blocking mode, this function only start the transfer,
  \param[in]   spi_id  spi port.
  \param[in]   tx_data  Pointer to buffer with data to send to SPI transmitter
  \param[out]  rx_data   Pointer to buffer for data to receive from SPI receiver
  \param[in]   tx_size      Number of data items to send
  \param[in]   rx_size       Number of data items to receive
  \param[in]   timeout  SPI transmitter/receiver expire time 
  \return      error code
*/

S32 ks_driver_spi_master_transfer(U32 spi_id, uint8_t *tx_data, uint8_t *rx_data,uint16_t tx_size,uint16_t rx_size, uint32_t timeout);

/**
  \brief       config cs  select.
  \param[in]   spi_id  spi port.
  \param[in]   cs_sel   ref SPI_CSx  
  \return      drv_err_e code
*/

S32 ks_driver_spi_master_cs_select(U32 spi_id, U32 cs_sel);


/**
  \brief       Initialize SPI Slave Interface.
  \param[in]   spi_id  spi port
  \return      drv_err_e code
*/
S32 ks_driver_spi_slave_init(U32 spi_id);
/**
  \brief       config spi port.
  \param[in]   spi_id  spi port.
  \param[in]   config   ref strcuct  spi_config_t
  \return      drv_err_e code
*/
S32 ks_driver_spi_slave_config(U32 spi_id,	spi_config_t   *config);

/**
  \brief       sending data to SPI transmitter,(received data is ignored).
  \param[in]   spi_id  spi port.
  \param[in]   data  Pointer to buffer with data to send to SPI transmitter. 
  \param[in]   num   Number of data items to send.
  \param[in]  timeout   SPI transmitter/receiver expire time 
  \return      drv_err_e code
*/
S32 ks_driver_spi_slave_send(U32 spi_id, const uint8_t *data, uint16_t size, uint32_t timeout);


/**
\brief      receiving data from SPI receiver. 
\param[in]	 spi_id  spi port.
\param[out] data  Pointer to buffer for data to receive from SPI receiver
\param[in]  num   Number of data items to receive
\param[in]  timeout   SPI transmitter/receiver expire time 
\return     drv_err_e code
*/

S32 ks_driver_spi_slave_receive(U32 spi_id, uint8_t *data, uint16_t size, uint32_t timeout);


/**
  \brief       sending/receiving data to/from SPI transmitter/receiver.
               if non-blocking mode, this function only start the transfer,
  \param[in]   spi_id  spi port.
  \param[in]   tx_data  Pointer to buffer with data to send to SPI transmitter
  \param[out]  rx_data   Pointer to buffer for data to receive from SPI receiver
  \param[in]   tx_size      Number of data items to send
  \param[in]   rx_size       Number of data items to receive
  \param[in]   timeout  SPI transmitter/receiver expire time 
  \return      error code
*/

S32 ks_driver_spi_slave_transfer(U32 spi_id, uint8_t *tx_data, uint8_t *rx_data,uint16_t tx_size,uint16_t rx_size, uint32_t timeout);



#endif /* INTERFACE_INCLUDE_AP_SPI_H_ */

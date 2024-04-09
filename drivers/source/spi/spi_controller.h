#ifndef PLATFORM_DRIVER_SPI_SPI_CONTROLLER_H_
#define PLATFORM_DRIVER_SPI_SPI_CONTROLLER_H_

#include <ks_spi.h>
#include "ks_driver.h"

#ifdef __cplusplus
extern "C" {
#endif


int32_t _spi_master_init(spi_dev_t *spi);

int32_t _spi_slave_init(spi_dev_t *spi);

int32_t _spi_config(spi_dev_t *spi,
						spi_config_t   *config);

int32_t _spi_send(spi_dev_t *spi, const uint8_t *data, uint16_t size, uint32_t timeout);

int32_t _spi_recv(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout);

int32_t _spi_transfer(spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data,
                         uint16_t tx_size,uint16_t rx_size, uint32_t timeout);


int32_t _spi_send_and_recv(spi_dev_t *spi, uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data,uint16_t rx_size, uint32_t timeout);

int32_t _spi_send_and_send(spi_dev_t *spi, uint8_t *tx1_data, uint16_t tx1_size, uint8_t *tx2_data,
        uint16_t tx2_size, uint32_t timeout);


int32_t _spi_cs_select(spi_dev_t *spi,U32 cs);

#ifdef __cplusplus
}
#endif


#endif /* PLATFORM_DRIVER_SPI_SPI_CONTROLLER_H_ */

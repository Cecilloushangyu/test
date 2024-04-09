#include "ks_spi.h"
#include "spi_controller.h"



static spi_dev_t spim_dev[CONFIG_SPIM_NUM];
static spi_dev_t spis_dev[CONFIG_SPIS_NUM];

int ks_driver_spi_master_init(U32 spi_id)
{
	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spim_dev[spi_id];

	return _spi_master_init(spi);
}

S32 ks_driver_spi_master_config(U32 spi_id,	spi_config_t   *config){

	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spim_dev[spi_id];
	
	if(spi->inited == 0 ) return DRV_ERROR_STATUS;
	memcpy(&spi->config, config, sizeof(spi_config_t));

	return _spi_config(spi,config);


}

S32 ks_driver_spi_master_send(U32 spi_id, const uint8_t *data, uint16_t size, uint32_t timeout){

	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spim_dev[spi_id];
	
	if(spi->config.mode == SPI_MODE_MASTER ) {

		return _spi_send(spi,data,size,timeout);

	}else{
		return DRV_ERROR_STATUS;
	}

}
S32 ks_driver_spi_master_receive(U32 spi_id, uint8_t *data, uint16_t size, uint32_t timeout){
	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spim_dev[spi_id];
	if(spi->config.mode == SPI_MODE_MASTER ) {
		return _spi_recv(spi,data,size,timeout);

	}else{
		return DRV_ERROR_STATUS;
	}


}

S32 ks_driver_spi_master_transfer(U32 spi_id, uint8_t *tx_data, uint8_t *rx_data,uint16_t tx_size,uint16_t rx_size, uint32_t timeout){

	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spim_dev[spi_id];
	if(spi->config.mode == SPI_MODE_MASTER  ) {
		return _spi_transfer(spi,tx_data,rx_data,tx_size,rx_size,timeout);

	}else{
		return DRV_ERROR_STATUS;
	}


}

S32 ks_driver_spi_master_cs_select(U32 spi_id, U32 cs){

	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spim_dev[spi_id];

	if(spi->config.mode == SPI_MODE_MASTER ) {

		return _spi_cs_select(spi,cs);

	}else{
		return DRV_ERROR_STATUS;
	}


}



int ks_driver_spi_slave_init(U32 spi_id)
{
	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spis_dev[spi_id];

	return _spi_slave_init(spi);
}

S32 ks_driver_spi_slave_config(U32 spi_id,	spi_config_t   *config){

	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spis_dev[spi_id];
	
	if(spi->inited == 0 ) return DRV_ERROR_STATUS;
	memcpy(&spi->config, config, sizeof(spi_config_t));

	return _spi_config(spi,config);


}

S32 ks_driver_spi_slave_send(U32 spi_id, const uint8_t *data, uint16_t size, uint32_t timeout){

	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spis_dev[spi_id];
	
	if(spi->config.mode == SPI_MODE_SLAVE )
	{
		return _spi_send(spi,data,size,timeout);
	}
	else
	{
		return DRV_ERROR_STATUS;
	}


}
S32 ks_driver_spi_slave_receive(U32 spi_id, uint8_t *data, uint16_t size, uint32_t timeout){
	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spis_dev[spi_id];

	if(spi->config.mode == SPI_MODE_SLAVE )
	{
		return _spi_recv(spi,data,size,timeout);
	}
	else
	{
		return DRV_ERROR_STATUS;
	}

}


S32 ks_driver_spi_slave_transfer(U32 spi_id, uint8_t *tx_data, uint8_t *rx_data,uint16_t tx_size,uint16_t rx_size, uint32_t timeout){

	if(spi_id!=0) return DRV_ERROR_PARAMETER;
	spi_dev_t *spi = &spis_dev[spi_id];

	if(spi->config.mode == SPI_MODE_SLAVE )
	{
		return _spi_transfer(spi,tx_data,rx_data,tx_size,rx_size,timeout);
	}
	else
	{
		return DRV_ERROR_STATUS;
	}

}



#include "ks_os.h"
#include "spi_low_level_driver.h"
#include "spi_controller.h"
#include "ks_os.h"
#include "ks_gpio.h"
#include "ks_uart.h"
#include "ks_sysctrl.h"
#include "ks_spi.h"




static void spi_event_cb_fun(int32_t idx, spi_event_e event,void* arg)
{
	spi_dev_t *spi =(spi_dev_t *) arg;

    switch (event) {
    case SPI_EVENT_TRANSFER_COMPLETE:
    case SPI_EVENT_TX_COMPLETE:
    case SPI_EVENT_RX_COMPLETE:
         ks_os_sem_post(spi->sem);
        break;
    default:;
    }
}

int32_t _spi_master_init(spi_dev_t *spi)
{
    int32_t ret;
    spi_handle_t spi_handle;

	if(spi->inited !=0 ) return DRV_ERROR_STATUS ;
	

    if (spi == NULL)
        return DRV_ERROR_PARAMETER;



    if (ks_os_sem_create(&spi->sem, "spim sem",0) != 0) {
        return DRV_ERROR;
    }

    if (ks_os_mutex_create(&spi->mutex,"spim mux") != 0) {
        return DRV_ERROR;
    }

	//0. mux

	ks_driver_sysctrl_set_mux_sel(GPIOA_21,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_22,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_23,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_24,MUX_V_FUN1);
	ks_driver_sysctrl_set_clock_enable(SPIM_CLK_EN,1);
	ks_driver_sysctrl_set_dev_reset(SPIM_RST,1);

    spi_handle = dw_spi_initialize(spi->port, SPI_MODE_MASTER,(spi_event_cb_t)spi_event_cb_fun,spi);
    if (spi_handle == NULL) {
        return DRV_ERROR;
    }
    spi->handle = spi_handle;


    ret = dw_spi_config(spi_handle, 100000, SPI_MODE_MASTER,
                         SPI_FORMAT_CPOL0_CPHA0, SPI_ORDER_MSB2LSB,
                         SPI_SS_MASTER_SW, 8);
    if (ret != 0) {
        return DRV_ERROR;
    }

	spi->inited = 1;
	
    return 0;
}

int32_t _spi_slave_init(spi_dev_t *spi)
{
    int32_t ret;
    spi_handle_t spi_handle;
	
	spi_mode_e	mode = SPI_MODE_SLAVE;

	if(spi->inited !=0 ) return DRV_ERROR_STATUS ;
	

    if (spi == NULL)
        return DRV_ERROR_PARAMETER;



    if (ks_os_sem_create(&spi->sem, "spis sem",0) != 0) {
        return DRV_ERROR;
    }

    if (ks_os_mutex_create(&spi->mutex,"spis mux") != 0) {
        return DRV_ERROR;
    }

	//0. mux
	ks_driver_sysctrl_set_mux_sel(GPIOA_21,MUX_V_FUN2);
	ks_driver_sysctrl_set_mux_sel(GPIOA_22,MUX_V_FUN2);
	ks_driver_sysctrl_set_mux_sel(GPIOA_23,MUX_V_FUN2);
	ks_driver_sysctrl_set_mux_sel(GPIOA_24,MUX_V_FUN2);
	ks_driver_sysctrl_set_clock_enable(SPIS_CLK_EN,1);
	ks_driver_sysctrl_set_dev_reset(SPIS_RST,1);

	
    spi_handle = dw_spi_initialize(spi->port, mode,(spi_event_cb_t)spi_event_cb_fun,spi);
    if (spi_handle == NULL) {
        return DRV_ERROR;
    }
    spi->handle = spi_handle;


    ret = dw_spi_config(spi_handle, 100000, SPI_MODE_SLAVE,
                         SPI_FORMAT_CPOL0_CPHA0, SPI_ORDER_MSB2LSB,
                         SPI_SS_MASTER_SW, 8);
    if (ret != 0) {
        return DRV_ERROR;
    }

	spi->inited = 1;
	
    return 0;
}


int32_t _spi_config(spi_dev_t *spi,
						spi_config_t   *config)
{
    int32_t ret;
    if (spi == NULL)
        return -1;
	
	
    ret = dw_spi_config(spi->handle, config->baud,  config->mode,
                          config->format, SPI_ORDER_MSB2LSB,
                         SPI_SS_MASTER_SW,  config->bit_width);


	if (ret != 0) {
		return DRV_ERROR;
	}
	
	return 0;

}



int32_t _spi_cs_select(spi_dev_t *spi,U32 cs)
{

	int32_t ret;
	if (spi == NULL)
		return -1;

	//
	if(cs&SPI_CS0)
	ks_driver_sysctrl_set_mux_sel(GPIOA_22,MUX_V_FUN1);
	if(cs&SPI_CS1)
	ks_driver_sysctrl_set_mux_sel(GPIOA_25,MUX_V_FUN1);
	if(cs&SPI_CS2)
	ks_driver_sysctrl_set_mux_sel(GPIOA_12,MUX_V_FUN3);
	if(cs&SPI_CS3)
	ks_driver_sysctrl_set_mux_sel(GPIOA_13,MUX_V_FUN3);
	if(cs&SPI_CS4)
	ks_driver_sysctrl_set_mux_sel(GPIOA_10,MUX_V_FUN3);
	if(cs&SPI_CS5)
	ks_driver_sysctrl_set_mux_sel(GPIOA_1,MUX_V_FUN3);
	if(cs&SPI_CS6)
	ks_driver_sysctrl_set_mux_sel(GPIOA_0,MUX_V_FUN3);
	if(cs&SPI_CS7)
	ks_driver_sysctrl_set_mux_sel(GPIOA_16,MUX_V_FUN2);
	if(cs&SPI_CS8)
	ks_driver_sysctrl_set_mux_sel(GPIOA_3,MUX_V_FUN2);
	if(cs&SPI_CS9)
	ks_driver_sysctrl_set_mux_sel(GPIOA_4,MUX_V_FUN2);

	ret = dw_spi_cs_control(spi->handle,cs);

	if (ret != 0) {
		return DRV_ERROR;
	}

	return 0;


}		   

int32_t _spi_send(spi_dev_t *spi, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    int32_t ret;
    if (spi == NULL)
        return -1;
	
    ks_os_mutex_enter(spi->mutex);
	
    ret = dw_spi_send(spi->handle, data, size);
	
	if (ret != 0) {
		goto ERR;
	}

	ret = ks_os_sem_pend(spi->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));

	if (ret != 0) {
		kprintf("spi:%d timeout ret %d \r\n", spi->port,ret);
		dw_spi_abort_transfer(spi->handle);
		ret = DRV_ERROR_TIMEOUT;
	}

ERR:
    ks_os_mutex_leave(spi->mutex);

    return ret;
}

int32_t _spi_recv(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout)
{
    int32_t ret;
    if (spi == NULL)
        return -1;


    ks_os_mutex_enter(spi->mutex);
	


    ret = dw_spi_receive(spi->handle, data, size);

	
	if (ret != 0) {
		goto ERR;
	}
	
	ret = ks_os_sem_pend(spi->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));

	if (ret != 0) {
		kprintf("spi:%d timeout ret %d \r\n", spi->port,ret);
		dw_spi_abort_transfer(spi->handle);
		ret = DRV_ERROR_TIMEOUT;
	}

ERR:
	ks_os_mutex_leave(spi->mutex);

    return ret;
}

int32_t _spi_transfer(spi_dev_t *spi, uint8_t *tx_data, uint8_t *rx_data,
                          uint16_t tx_size,uint16_t rx_size,  uint32_t timeout)
{
    int32_t ret;
    if (spi == NULL)
        return -1;


    ks_os_mutex_enter(spi->mutex);
	

    ret = dw_spi_transfer(spi->handle, tx_data, rx_data, tx_size, rx_size);
		
	if (ret != 0) {
		goto ERR;
	}
	ret = ks_os_sem_pend(spi->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));

	if (ret != 0) {
		kprintf("spi:%d timeout ret %d \r\n", spi->port,ret);
		dw_spi_abort_transfer(spi->handle);
		ret = DRV_ERROR_TIMEOUT;
	}

ERR:
	ks_os_mutex_leave(spi->mutex);

    return ret;
}

int32_t _spi_send_and_recv(spi_dev_t *spi, uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data,
        uint16_t rx_size, uint32_t timeout)
{
    int32_t ret;
    if (spi == NULL)
        return -1;
	
    ks_os_mutex_enter(spi->mutex);

	
    ret = dw_spi_send(spi->handle, tx_data, tx_size);
	if (ret != 0) {
		goto ERR;
	}
	
	ret = ks_os_sem_pend(spi->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));
	if (ret != 0) {
		kprintf("spi:%d timeout ret %d \r\n", spi->port,ret);
	}

    ret = dw_spi_receive(spi->handle, rx_data, rx_size);
	if (ret != 0) {
		goto ERR;
	}
	ret = ks_os_sem_pend(spi->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));
	if (ret != 0) {
		kprintf("spi:%d timeout ret %d \r\n", spi->port,ret);
	}


ERR:
	ks_os_mutex_leave(spi->mutex);

	return ret;

}

int32_t _spi_send_and_send(spi_dev_t *spi, uint8_t *tx1_data, uint16_t tx1_size, uint8_t *tx2_data,
        uint16_t tx2_size, uint32_t timeout)
{
    int32_t ret;
    if (spi == NULL)
        return -1;
	
    ks_os_mutex_enter(spi->mutex);

    ret = dw_spi_send(spi->handle, tx1_data, tx1_size);
	if (ret != 0) {
		goto ERR;
	}
	
	ret = ks_os_sem_pend(spi->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));
	if (ret != 0) {
		kprintf("spi:%d timeout ret %d \r\n", spi->port,ret);
	}

    ret =  dw_spi_send(spi->handle, tx2_data, tx2_size);
	if (ret != 0) {
		goto ERR;
	}
	
	ret = ks_os_sem_pend(spi->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));
	if (ret != 0) {
		kprintf("spi:%d timeout ret %d \r\n", spi->port,ret);
	}

	
ERR:
	ks_os_mutex_leave(spi->mutex);

	return ret;


}

int32_t _spi_finalize(spi_dev_t *spi)
{
    dw_spi_uninitialize(spi->handle);
    return 0;
}



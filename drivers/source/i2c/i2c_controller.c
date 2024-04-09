#include "ks_os.h"
#include "i2c_controller.h"
#include "i2c_controller.h"
#include "ks_sysctrl.h"
#include "ks_i2c.h"
#include "ks_gpio.h"
#include "ks_uart.h"
#include "i2c_low_level_driver.h"
#include "ks_driver.h"
#include "ks_dma.h"
#include "ks_cache.h"


void _i2c_event_cb(int32_t idx, iic_event_e event,void* arg)
{
	i2c_dev_t *i2c =(i2c_dev_t *) arg;

    if (event == IIC_EVENT_TRANSFER_DONE) {
        ks_os_sem_post(i2c->sem);
    } else {
    	if(event == IIC_EVENT_BUS_ERROR){
            kprintf("_i2c_event_cb event IIC_EVENT_BUS_ERROR \r\n");
		}
		else if(event == IIC_EVENT_ARBITRATION_LOST){
			kprintf("_i2c_event_cb event IIC_EVENT_ARBITRATION_LOST \r\n");
		}
        else if(event == IIC_EVENT_ADDRESS_NACK){
            kprintf("_i2c_event_cb event IIC_EVENT_ADDRESS_NACK \r\n");
		}
    }
}

int32_t _i2c_init(i2c_dev_t *i2c,uint32_t sel)
{
	int mode;

	int tx_dma_enable;
	int rx_dma_enable;

	tx_dma_enable = (sel & TX_DMA_ENABLE);
	rx_dma_enable = (sel & RX_DMA_ENABLE);

	
	if(i2c->inited !=0 ) return DRV_ERROR_STATUS ;

    if (i2c == NULL) {
        return DRV_ERROR_PARAMETER;
    }

    if (ks_os_sem_create(&i2c->sem, "i2c sem",0) != 0) {
        return DRV_ERROR;
    }

    if (ks_os_mutex_create(&i2c->mutex,"i2c mux") != 0) {
        return DRV_ERROR;
    }

	mode = (sel&0x0F);

	//0. mux
	if(mode == 0){
		ks_driver_sysctrl_set_mux_sel(GPIOA_15,MUX_V_FUN1);
		ks_driver_sysctrl_set_mux_sel(GPIOA_16,MUX_V_FUN1);
		ks_driver_sysctrl_set_clock_enable(I2C_CLK_EN,1);
	}
	else{
		ks_driver_sysctrl_set_mux_sel(GPIOA_21,MUX_V_FUN3);
		ks_driver_sysctrl_set_mux_sel(GPIOA_22,MUX_V_FUN3);
		ks_driver_sysctrl_set_clock_enable(I2C_CLK_EN,1);
	}


    iic_handle_t handle = _iic_get_handle(i2c->port);

    if (handle == NULL) {
        return DRV_ERROR_PARAMETER;
    }
	
	OSHandle dma_handle = ks_driver_dma_init();

	//dma channel 提前分配
	if(tx_dma_enable||rx_dma_enable){
		
		i2c->tx_dma_enable = tx_dma_enable;
		i2c->rx_dma_enable = rx_dma_enable;

		if(i2c->tx_dma_enable){
			i2c->tx_dma_ch = ks_driver_dma_bind_channel( dma_handle);
			if(i2c->tx_dma_ch == -1) ks_exception_assert(0);
		}
		if(i2c->rx_dma_enable){
			i2c->rx_dma_ch = ks_driver_dma_bind_channel( dma_handle);
			if(i2c->rx_dma_ch == -1) ks_exception_assert(0);
		}
	}
	
    i2c->handle = (void *)handle;

	_iic_initialize(handle,i2c);

	_iic_register_event_cb(handle,(iic_event_cb_t)_i2c_event_cb,i2c);

	i2c->inited = 1;

	return 0;
	
}


int32_t _i2c_config(i2c_dev_t *i2c,i2c_config_t* config){

	return _iic_config(i2c->handle,config->mode,config->speed,config->addr_mode,config->slave_addr);

}

int32_t _i2c_master_send(i2c_dev_t *i2c, uint16_t dev_addr, const void *data,
                            uint16_t size, uint32_t timeout)
{
    ks_os_mutex_enter(i2c->mutex);
    int ret = _iic_master_send((iic_handle_t)i2c->handle, dev_addr, data, size);

    if (ret != 0) {
        goto ERR;
    }

    ret = ks_os_sem_pend(i2c->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));

    if (ret != 0) {
        kprintf("i2c:%d timeout %d \r\n", i2c->port,ret);
		_iic_abort_transfer(i2c->handle);
		ret = DRV_ERROR_TIMEOUT;
    }

ERR:
    ks_os_mutex_leave(i2c->mutex);

    return ret;
}

int32_t _i2c_master_recv(i2c_dev_t *i2c, uint16_t dev_addr, void *data,
                            uint16_t size, uint32_t timeout)
{
    ks_os_mutex_enter(i2c->mutex);
    int ret = _iic_master_receive((iic_handle_t)i2c->handle, dev_addr, data, size);

    if (ret != 0) {
        goto ERR;
    }

    ret = ks_os_sem_pend(i2c->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));

    if (ret != 0) {
        kprintf("i2c:%d timeout ret %d \r\n", i2c->port,ret);
		_iic_abort_transfer(i2c->handle);
		ret = DRV_ERROR_TIMEOUT;
    }

ERR:
    ks_os_mutex_leave(i2c->mutex);

    return ret;
}

int32_t _i2c_slave_send(i2c_dev_t *i2c, const void *data, uint16_t size, uint32_t timeout)
{
    ks_os_mutex_enter(i2c->mutex);
    int ret = _iic_slave_send((iic_handle_t)i2c->handle, data, size);

    if (ret != 0) {
        goto ERR;
    }

    ret = ks_os_sem_pend(i2c->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));

    if (ret != 0) {
        kprintf("i2c:%d timeout ret  %d  \r\n", i2c->port,ret);
		_iic_abort_transfer(i2c->handle);
		ret = DRV_ERROR_TIMEOUT;
    }

ERR:
    ks_os_mutex_leave(i2c->mutex);

    return ret;
}


int32_t _i2c_slave_recv(i2c_dev_t *i2c, void *data, uint16_t size, uint32_t timeout)
{
    ks_os_mutex_enter(i2c->mutex);
    int ret = _iic_slave_receive((iic_handle_t)i2c->handle, data, size);

    if (ret != 0) {
        goto ERR;
    }

    ret = ks_os_sem_pend(i2c->sem, (timeout==KS_OS_WAIT_FOREVER)?KS_OS_WAIT_FOREVER:ks_os_msec_to_tick(timeout));

    if (ret != 0) {
        kprintf("i2c:%d timeout %d  \r\n", i2c->port,ret);
		_iic_abort_transfer(i2c->handle);
		ret = DRV_ERROR_TIMEOUT;
    }

    ks_cpu_dcache_invalidate(data, size);

ERR:
    ks_os_mutex_leave(i2c->mutex);

    return ret;
}


int32_t _i2c_finalize(i2c_dev_t *i2c)
{
     if (i2c == NULL) {
        return DRV_ERROR_PARAMETER;
    }
    ks_os_sem_delete(i2c->sem);
    ks_os_mutex_delete(i2c->mutex);
    return _iic_uninitialize((iic_handle_t)i2c->handle);
}



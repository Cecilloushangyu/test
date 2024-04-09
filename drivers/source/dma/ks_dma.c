
#include "ks_dma.h"
#include "ks_driver.h"
#include "dma_controller.h"



OSHandle ks_driver_dma_init(){

	return _dma_initialize(0);

}


OSHandle ks_driver_dma_get_handle(){

	return _dma_get_handle(0);


}

/**
  \brief       De-initialize DMA Interface. stops operation and releases the software resources used by the interface
  \param[in]   handle damc handle to operate.
  \return      error code
*/
S32 ks_driver_dma_uninit(OSHandle handle){

	return _dma_uninitialize(handle);

}



/**
  \brief     bind one free dma channel
  \param[in] handle damc handle to operate.
  \return    -1 - no channel can be used, other - channel index
 */
S32 ks_driver_dma_bind_channel(OSHandle handle){
	int ret ;
	ks_os_irq_mask_all();
	ret=  _dma_bind_channel(handle);
	ks_os_irq_unmask_all();
	return ret;

}

/**
  \brief     get one free dma channel
  \param[in] handle damc handle to operate.
  \param[in] ch channel num. if -1 then allocate a free channal in this dma
  \return    -1 - no channel can be used, other - channel index
 */
S32 ks_driver_dma_alloc_channel(OSHandle handle, S32 ch){
	int ret ;
	ks_os_irq_mask_all();
	ret=  _dma_alloc_channel(handle,ch);
	ks_os_irq_unmask_all();
	return ret;

}


/**
  \brief        release dma channel and related resources
  \param[in]    handle damc handle to operate.
  \param[in]    ch  channel num.
  \return       error code
 */
S32 ks_driver_dma_release_channel(OSHandle handle, S32 ch){
	int ret ;
	ks_os_irq_mask_all();
	ret=  _dma_release_channel(handle,ch);
	ks_os_irq_unmask_all();
	return ret;


}

/**
  \brief        config dma channel
  \param[in]    handle damc handle to operate.
  \param[in]    ch          channel num.
  \param[in]    config      dma channel transfer configure
  \param[in]    cb_event    Pointer to \ref dma_event_cb_t
  \return       error code if negative, otherwise return the channel num if success.
 */
S32 ks_driver_dma_config_channel(OSHandle handle, S32 ch,
                               dma_config_t *config, dma_event_cb callback,void* arg){
	int ret ;
	ks_os_irq_mask_all();

	ret = _dma_config_channel(handle,ch,config, callback,arg);

	ks_os_irq_unmask_all();
	
 	return ret;

}

/**
  \brief       start generate dma channel signal.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \param[in]   psrcaddr    dma transfer source address
  \param[in]   pdstaddr    dma transfer destination address
  \param[in]   length      dma transfer length (unit: bytes).
  \return      error code
*/
S32 ks_driver_dma_start(OSHandle handle, S32 ch, void *psrcaddr,
                      void *pdstaddr, U32 length){

	int ret ;
	ks_os_irq_mask_all();

	ret=  _dma_start(handle,ch,psrcaddr,pdstaddr,length);

	ks_os_irq_unmask_all();
	
 	return ret;


}

/**
  \brief       Stop generate dma channel signal.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \return      error code
*/
S32 ks_driver_dma_stop(OSHandle handle, S32 ch){
	int ret ;
	ks_os_irq_mask_all();
	ret=  _dma_stop(handle,ch);
	ks_os_irq_unmask_all();
	
 	return ret;

}

/**
  \brief       Get DMA channel status.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \return      DMA status \ref dma_status_e
*/
dma_status_e ks_driver_dma_get_status(OSHandle handle, S32 ch){

	return _dma_get_status(handle,ch);


}



/**
  \brief       Get DMA channel trans offset.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \param[in&out]   psrc_offset src trans offset.
  \param[in&out]   pdst_offset dst trans offset.
  \return        error code
*/

S32 ks_driver_dma_get_trans_offset(OSHandle handle, S32 ch, U32* psrc_offset,U32* pdst_offset)
{
	int ret ;
	ks_os_irq_mask_all();
	ret=  _dma_get_trans_offset(handle,ch,(uint32_t*)psrc_offset,(uint32_t*)pdst_offset);
	ks_os_irq_unmask_all();

	return ret;
}

S32 ks_driver_dma_check_fifo_empty(OSHandle handle, S32 ch){

	return _dma_check_fifo_empty(handle,ch);

}

S32 ks_driver_dma_suspend(OSHandle handle, S32 ch,S32 enable){
	int ret ;
	ks_os_irq_mask_all();
	ret=  _dma_suspend(handle,ch,enable);
	ks_os_irq_unmask_all();
	
 	return ret;

}



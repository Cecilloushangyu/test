
#ifndef _DMA_CONTROL_H_
#define _DMA_CONTROL_H_

#include <stdbool.h>
#include "dw_dmac.h"
#include "ks_dma.h"
#include <string.h>
#include <ks_os.h>
#include <ks_shell.h>
#include <ks_driver.h>


#define ERR_DMA(errno) (1000 | errno)


typedef struct {
    uint32_t inited;
    uint32_t base;
    uint32_t control_base;
    uint32_t irq;
    uint8_t ch_num;
	//高4位标识通道绑定状态，低4位表示通道打开状态
    uint8_t ch_opened[DMA_MAXCHANNEL];
    dma_status_e status[DMA_MAXCHANNEL];
	dma_channel_context ch_ctx[DMA_MAXCHANNEL];
    uint32_t src_tw;
    uint32_t dst_tw;
} dw_dma_priv_t;




#define readl(addr) \
    *((volatile uint32_t *)(addr))

#define writel(b, addr) \
    *((volatile uint32_t *)(addr)) = b


OSHandle _dma_initialize(int32_t idx);
int32_t _dma_uninitialize(OSHandle handle);

OSHandle _dma_get_handle(int32_t idx);



/**
  \brief     bind one free dma channel
  \param[in] handle damc handle to operate.
  \return    -1 - no channel can be used, other - channel index
 */
int32_t _dma_bind_channel(OSHandle handle);

/**
  \brief     get one free dma channel
  \param[in] handle damc handle to operate.
  \param[in] ch channel num. if -1 then allocate a free channal in this dma
  \return    -1 - no channel can be used, other - channel index
 */
int32_t _dma_alloc_channel(OSHandle handle, int32_t ch);


/**
  \brief        release dma channel and related resources
  \param[in]    handle damc handle to operate.
  \param[in]    ch  channel num.
  \return       error code
 */
int32_t _dma_release_channel(OSHandle handle, int32_t ch);


/**
  \brief        config dma channel
  \param[in]    handle damc handle to operate.
  \param[in]    ch          channel num.
  \param[in]    config      dma channel transfer configure
  \param[in]    cb_event    Pointer to \ref dma_event_cb_t
  \param[in]    arg    Pointer to \ref callback arg
  \return       error code
 */
int32_t _dma_config_channel(OSHandle handle, int32_t ch,
                               dma_config_t *config,dma_event_cb callback,void* arg);



/**
  \brief       start generate dma channel signal.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \param[in]   psrcaddr    dma transfer source address
  \param[in]   pdstaddr    dma transfer destination address
  \param[in]   length      dma transfer length (unit: bytes).
  \return      error code
*/
int32_t _dma_start(OSHandle handle, int32_t ch, void *psrcaddr,
                      void *pdstaddr, uint32_t length);


/**
\brief		 Stop generate dma channel signal.
\param[in]	 handle damc handle to operate.
\param[in]	 ch  channel num.
\return 	 error code
*/
int32_t _dma_stop(OSHandle handle, int32_t ch);

/**
  \brief       Get DMA channel status.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \return      DMA status \ref dma_status_t
*/
int32_t _dma_get_status(OSHandle handle, int32_t ch);



/**
  \brief       Get DMA channel trans offset.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \param[in&out]   psrc_offset src trans offset.
  \param[in&out]   pdst_offset dst trans offset.
  \return        error code
*/
int32_t _dma_get_trans_offset(OSHandle handle, int32_t ch, uint32_t* psrc_offset,uint32_t* pdst_offset);


int32_t _dma_check_fifo_empty(OSHandle handle, int32_t ch);

int32_t _dma_suspend(OSHandle handle, int32_t ch, int enable);

#endif


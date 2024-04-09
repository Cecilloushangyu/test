#ifndef _KS_DMA_H_
#define _KS_DMA_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ks_os.h>
#include <ks_driver.h>

#define CONFIG_DMAC_NUM 1 //1个DMA

//通道dma一次最大传输字节
#define DMA_CHx_MAX_BLK_SIZE  4095

typedef enum {
    DMA_STATE_FREE = 0,       ///< DMA channel not yet initialized or disabled
    DMA_STATE_READY,          ///< DMA channel process success and ready for use, but not start yet
    DMA_STATE_BUSY,           ///< DMA channel process is ongoing
    DMA_STATE_DONE,           ///< DMA channel transfer done
    DMA_STATE_ERROR,          ///< DMA channel transfer error
} dma_status_e;



/****** DMA Event *****/
typedef enum {
    DMA_EVENT_TRANSFER_DONE        = 0,  ///< transfer complete
    DMA_EVENT_TRANSFER_HALF_DONE   = 1,  ///< transfer half done
    DMA_EVENT_TRANSFER_MODE_DONE   = 2,  ///< transfer complete in a certain dma trigger mode.
    DMA_EVENT_CAHNNEL_PEND         = 3,  ///< it happens when there is a low priority channel was preempted by a high priority channel
    DMA_EVENT_TRANSFER_ERROR       = 4,  ///< transfer error
} dma_event_e;

typedef enum {
    DMA_DATAWIDTH_SIZE8  = 1,
    DMA_DATAWIDTH_SIZE16 = 2,
    DMA_DATAWIDTH_SIZE32 = 4
} dma_datawidth_e;

typedef enum {
    DMA_ADDR_INC    = 0,
    DMA_ADDR_DEC,
    DMA_ADDR_CONSTANT
} dma_addr_inc_e;

typedef enum {
    DMA_MEM2MEM     = 0,
    DMA_MEM2PERH,
    DMA_PERH2MEM,
} dma_trans_type_e;




typedef enum {
    DMA_ADDR_LITTLE = 0,
    DMA_ADDR_BIG
} dma_addr_endian_e;

typedef enum {
    DMA_HANDSHAKING_HARDWARE = 0,
    DMA_HANDSHAKING_SOFTWARE
} dma_channel_req_mode_e;


typedef enum {
    DMA_BURST_TRANSACTION_SIZE1 = 0,
    DMA_BURST_TRANSACTION_SIZE4 = 1,
    DMA_BURST_TRANSACTION_SIZE8 = 2,
} dma_burst_transaction_len_t;

typedef enum {
	DMA_SPIS_RX,
	DMA_SPIS_TX,
	DMA_SPIM_RX,
	DMA_SPIM_TX,
	DMA_IIC_RX,
	DMA_IIC_TX,
	DMA_UART0_RX,
	DMA_UART0_TX,
	DMA_UART1_RX,
	DMA_UART1_TX,
	DMA_UART2_RX,
	DMA_UART2_TX,
	DMA_UART3_RX,
	DMA_UART3_TX,
	DMA_DEV_MAX,
} dma_handshaking_device_e;

typedef struct {
    dma_addr_inc_e         src_inc;        ///< source address increment
    dma_addr_inc_e         dst_inc;        ///< destination address increment
    dma_addr_endian_e      src_endian;     ///< source read data little-big endian change control.
    dma_addr_endian_e      dst_endian;     ///< destination write data little-big endian change control.
    dma_burst_transaction_len_t    burst_len; 		///< burst transaction length (unit: bytes)
	dma_datawidth_e        src_tw;         ///< source transfer width in byte
    dma_datawidth_e        dst_tw;         ///< destination transfer width in byte
    dma_handshaking_device_e hs_if;          ///< a hardware handshaking interface (optional).
    dma_trans_type_e       type;           ///< transfer type
    dma_channel_req_mode_e ch_mode;        ///< software or hardware to tigger dma channel work.
} dma_config_t;





typedef struct _dma_channel_context dma_channel_context;


typedef void (*dma_event_cb)(dma_channel_context *dma_ch_ctx, int channel_id,dma_event_e event);   ///< Pointer to \ref dma_event_cb_t : dmac event call back.


struct _dma_channel_context {
	U32         ctrl_id; //ctrl id
    U32         ch_id;
	U32         len;
	U32			psrcaddr;
	U32			pdstaddr;
	U32         transtype;
	U32			alloc_success;
	U32			alloc_fails;
	U32			free_success;
    void 		*arg;     
    dma_event_cb callback;
};




/**
  \brief       Initialize DMA Interface. 1. Initializes the resources needed for the DMA interface 2.registers event callback function
  \return      pointer to dma instances
*/
OSHandle ks_driver_dma_init();


OSHandle ks_driver_dma_get_handle();

/**
  \brief       De-initialize DMA Interface. stops operation and releases the software resources used by the interface
  \param[in]   handle damc handle to operate.
  \return      error code
*/
S32 ks_driver_dma_uninit(OSHandle handle);

/**
  \brief     bind one free dma channel
  \param[in] handle damc handle to operate.
  \return    -1 - no channel can be used, other - channel index
 */
S32 ks_driver_dma_bind_channel(OSHandle handle);

/**
  \brief     get one free dma channel
  \param[in] handle damc handle to operate.
  \param[in] ch channel num. if -1 then allocate a free channal in this dma
  \return    -1 - no channel can be used, other - channel index
 */
S32 ks_driver_dma_alloc_channel(OSHandle handle, S32 ch);

/**
  \brief        release dma channel and related resources
  \param[in]    handle damc handle to operate.
  \param[in]    ch  channel num.
  \return       error code
 */
S32 ks_driver_dma_release_channel(OSHandle handle, S32 ch);

/**
  \brief        config dma channel
  \param[in]    handle damc handle to operate.
  \param[in]    ch          channel num.
  \param[in]    config      dma channel transfer configure
  \param[in]    callback    Pointer to \ref dma_channel_context
  \return       error code if negative, otherwise return the channel num if success.
 */
S32 ks_driver_dma_config_channel(OSHandle handle, S32 ch,
                               dma_config_t *config, dma_event_cb callback,void* arg);

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
                      void *pdstaddr, U32 length);

/**
  \brief       Stop generate dma channel signal.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \return      error code
*/
S32 ks_driver_dma_stop(OSHandle handle, S32 ch);

/**
  \brief       Get DMA channel status.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \return      DMA status \ref dma_status_e
*/
dma_status_e ks_driver_dma_get_status(OSHandle handle, S32 ch);


/**
  \brief       Get DMA channel trans offset.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \param[in&out]   psrc_offset src trans offset.
  \param[in&out]   pdst_offset dst trans offset.
  \return        error code
*/

S32 ks_driver_dma_get_trans_offset(OSHandle handle, S32 ch, U32* psrc_offset,U32* pdst_offset);


S32 ks_driver_dma_check_fifo_empty(OSHandle handle, S32 ch);

S32 ks_driver_dma_suspend(OSHandle handle, S32 ch,S32 enable);


#ifdef __cplusplus
}
#endif

#endif /* _CSI_DMA_H_ */


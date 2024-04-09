
#include <string.h>
#include "ks_spi.h"
#include "spi_low_level_driver.h"
#include "spi_controller.h"

#if defined(SPI_DMA_SURPPORT)
#include <ks_cache.h>
#include <ks_dma.h>
#endif

#define DRIVER_MAX(a, b)    (((a) > (b)) ? (a) : (b))
#define DRIVER_MIN(a, b)    (((a) < (b)) ? (a) : (b))

#define HANDLE_PARAM_CHK(para, err)                                                                \
    do {                                                                                           \
        if ((int32_t)para == (int32_t)NULL) {                                                      \
            return (err);                                                                          \
        }                                                                                          \
    } while (0)

#define ERR_SPI(errno) (1000 | errno)
#define SPI_BUSY_TIMEOUT    0x1000000
#define SPI_NULL_PARAM_CHK(para)  HANDLE_PARAM_CHK(para, ERR_SPI(DRV_ERROR_PARAMETER))


#define TRANSFER_STAT_IDLE      0
#define TRANSFER_STAT_SEND      1
#define TRANSFER_STAT_RCV       2
#define TRANSFER_STAT_TRAN      3

typedef struct {

	uint32_t idx;
    uint32_t base;
    uint32_t irq;

    spi_event_cb_t cb_event;
	void* arg;
    volatile uint32_t send_num_left;
    volatile uint32_t recv_num_left;
    uint8_t *send_buf;
    uint8_t *recv_buf;
    uint32_t enable_slave;
    volatile uint32_t transfer_num;
    volatile uint32_t tot_num_left;
    uint8_t state;
    uint32_t mode;
    uint8_t ss_mode;
    spi_status_t status;
	int32_t 		 baud;
	spi_mode_e 	  spi_mode;
	spi_format_e	  format;
	spi_bit_order_e  order;
	int32_t		  bit_width;

#if defined  SPI_DMA_SURPPORT 
    int32_t dma_tx_ch;
    int32_t dma_rx_ch;
	OSHandle dma_handle;
    uint8_t dma_use;
	uint8_t* dma_txbuffer;
	uint8_t* dma_rxbuffer;
#endif

    uint8_t  transfer_stat;     /* TRANSFER_STAT_* : 0 - idle, 1 - send , 2 -receive , 3 - transceive */
    volatile uint32_t tot_num;
} dw_spi_priv_t;

#define SPI_DMA_BLOCK_SIZE  1024

static int32_t dw_spi_set_mode(spi_handle_t handle, DW_SPI_MODE mode);
//static U8 s_spi_dma_txbuffer[SPI_DMA_BLOCK_SIZE] __attribute__ ((aligned (64)));
//static U8 s_spi_dma_rxbuffer[SPI_DMA_BLOCK_SIZE] __attribute__ ((aligned (64)));
static dw_spi_priv_t spim_instance[CONFIG_SPIM_NUM];
static dw_spi_priv_t spis_instance[CONFIG_SPIS_NUM];

static int spim_reg_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;
	
    dw_spi_priv_t *spi_priv = &spim_instance[0];

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

	
	ks_shell_printf(ctx->uart_id,"\r\n");

	ks_shell_printf(ctx->uart_id,"CTRLR0 : %x \r\n",addr->CTRLR0);
	ks_shell_printf(ctx->uart_id,"CTRLR1 : %x \r\n",addr->CTRLR1);
	
	ks_shell_printf(ctx->uart_id,"SR : %x \r\n",addr->SR);
	ks_shell_printf(ctx->uart_id,"ISR : %x \r\n",addr->ISR);

	ks_shell_printf(ctx->uart_id,"RISR : %x \r\n",addr->RISR);
	ks_shell_printf(ctx->uart_id,"IMR : %x \r\n",addr->IMR);

	

    return 0;
}


static int spim_ctx_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;
	
    dw_spi_priv_t *spi_priv = &spim_instance[0];


	ks_shell_printf(ctx->uart_id,"\r\n");

	ks_shell_printf(ctx->uart_id,"mode : %x \r\n",spi_priv->spi_mode);
	ks_shell_printf(ctx->uart_id,"baud : %x \r\n",spi_priv->baud);
	ks_shell_printf(ctx->uart_id,"format : %x \r\n",spi_priv->format);
	ks_shell_printf(ctx->uart_id,"order : %x \r\n",spi_priv->order);
	ks_shell_printf(ctx->uart_id,"bit_width : %x \r\n",spi_priv->bit_width);

	
#if defined  SPI_DMA_SURPPORT 
	ks_shell_printf(ctx->uart_id,"dma_handle : %x \r\n",spi_priv->dma_handle);
	ks_shell_printf(ctx->uart_id,"dma_use : %x \r\n",spi_priv->dma_use);
	ks_shell_printf(ctx->uart_id,"dma_tx_ch : %x \r\n",spi_priv->dma_tx_ch);
	ks_shell_printf(ctx->uart_id,"dma_rx_ch : %x \r\n",spi_priv->dma_rx_ch);
	ks_shell_printf(ctx->uart_id,"send_num : %x \r\n",spi_priv->send_num);
	ks_shell_printf(ctx->uart_id,"recv_num : %x \r\n",spi_priv->recv_num);
#endif

    return 0;
}


static int spis_reg_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;
	
    dw_spi_priv_t *spi_priv = &spis_instance[0];

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

	
	ks_shell_printf(ctx->uart_id,"\r\n");

	ks_shell_printf(ctx->uart_id,"CTRLR0 : %x \r\n",addr->CTRLR0);
	ks_shell_printf(ctx->uart_id,"CTRLR1 : %x \r\n",addr->CTRLR1);
	
	ks_shell_printf(ctx->uart_id,"SR : %x \r\n",addr->SR);
	ks_shell_printf(ctx->uart_id,"ISR : %x \r\n",addr->ISR);

	ks_shell_printf(ctx->uart_id,"RISR : %x \r\n",addr->RISR);
	ks_shell_printf(ctx->uart_id,"IMR : %x \r\n",addr->IMR);

	

    return 0;
}


static int spis_ctx_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;
	
    dw_spi_priv_t *spi_priv = &spis_instance[0];


	ks_shell_printf(ctx->uart_id,"\r\n");

	ks_shell_printf(ctx->uart_id,"mode : %x \r\n",spi_priv->mode);
	ks_shell_printf(ctx->uart_id,"baud : %x \r\n",spi_priv->baud);
	ks_shell_printf(ctx->uart_id,"format : %x \r\n",spi_priv->format);
	ks_shell_printf(ctx->uart_id,"order : %x \r\n",spi_priv->order);
	ks_shell_printf(ctx->uart_id,"bit_width : %x \r\n",spi_priv->bit_width);

	
#if defined  SPI_DMA_SURPPORT 
	ks_shell_printf(ctx->uart_id,"dma_handle : %x \r\n",spi_priv->dma_handle);
	ks_shell_printf(ctx->uart_id,"dma_use : %x \r\n",spi_priv->dma_use);
	ks_shell_printf(ctx->uart_id,"dma_tx_ch : %x \r\n",spi_priv->dma_tx_ch);
	ks_shell_printf(ctx->uart_id,"dma_rx_ch : %x \r\n",spi_priv->dma_rx_ch);
	ks_shell_printf(ctx->uart_id,"send_num : %x \r\n",spi_priv->send_num);
	ks_shell_printf(ctx->uart_id,"recv_num : %x \r\n",spi_priv->recv_num);
#endif

    return 0;
}

static cmd_proc_t spi_driver_cmds[] = {
	{.cmd = "spim_ctx", .fn = spim_ctx_cmd,  .help = "spim_ctx_cmd "},
	{.cmd = "spim_reg", .fn = spim_reg_cmd,  .help = "spim_reg_cmd "},
	{.cmd = "spis_ctx", .fn = spis_ctx_cmd,  .help = "spis_ctx_cmd "},
	{.cmd = "spis_reg", .fn = spis_reg_cmd,  .help = "spis_reg_cmd "},

};

#if defined  SPI_DMA_SURPPORT
static void dw_set_spi_dma_data_level(dw_spi_priv_t *spi_priv)
{
    uint8_t i;
    uint32_t num = 0;

    if (spi_priv->mode == DW_SPI_TXRX) {
        num = spi_priv->transfer_num;
    } else if (spi_priv->mode == DW_SPI_TX) {
        num = spi_priv->send_num;
    } else if (spi_priv->mode == DW_SPI_RX) {
        num = spi_priv->recv_num;
    }

    for (i = 2; i > 0; i--) {
        if (!(num % (2 << i))) {
            break;
        }
    }

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (i == 0) {
        addr->DMARDLR = 0;
        addr->DMATDLR = 1;
    } else {
        addr->DMARDLR = (2 << i) - 1;
        addr->DMATDLR = (2 << i) - 1;
    }
}

void dw_spi_dma_event_cb(dma_channel_context* ch_ctx, int32_t ch, dma_event_e event)
{
    dw_spi_priv_t *spi_priv = (dw_spi_priv_t *)ch_ctx->arg;

    uint8_t i = 0u;
    uint32_t timeout = 0;

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
	kprintf("dw_spi_dma_event_cb  %d \r\n",event);
	
    if (event == DMA_EVENT_TRANSFER_ERROR) {           /* DMA transfer ERROR */
        if (spi_priv->cb_event) {
            spi_priv->cb_event(spi_priv->idx, SPI_EVENT_DATA_LOST,spi_priv->arg);
        }
    } else if (event == DMA_EVENT_TRANSFER_DONE) {  /* DMA transfer complete */
        if (spi_priv->mode == DW_SPI_TXRX) {
            if (ks_driver_dma_get_status(spi_priv->dma_handle,spi_priv->dma_tx_ch) != DMA_STATE_DONE) {
                return;
            }

            if (ks_driver_dma_get_status(spi_priv->dma_handle,spi_priv->dma_rx_ch) != DMA_STATE_DONE) {
                return;
            }

            addr->SPIENR            = DW_SPI_DISABLE;
            ks_driver_dma_stop(spi_priv->dma_handle,spi_priv->dma_tx_ch);
            ks_driver_dma_stop(spi_priv->dma_handle,spi_priv->dma_rx_ch);

            spi_priv->recv_buf      += spi_priv->transfer_num;
            spi_priv->send_buf      += spi_priv->transfer_num;
            spi_priv->recv_num      -= spi_priv->transfer_num;
            spi_priv->send_num      -= spi_priv->transfer_num;
            spi_priv->clk_num       -= spi_priv->transfer_num;

            if (spi_priv->clk_num) {
                if (spi_priv->clk_num > SPI_DMA_BLOCK_SIZE) {
                    spi_priv->transfer_num = SPI_DMA_BLOCK_SIZE;
                } else {
                    spi_priv->transfer_num = spi_priv->clk_num;
                }

                dw_set_spi_dma_data_level(spi_priv);
                addr->DMACR   = DW_SPI_RDMAE | DW_SPI_TDMAE;
                addr->SER = spi_priv->enable_slave;
                addr->SPIENR    = DW_SPI_ENABLE;
                ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_tx_ch, spi_priv->send_buf, (uint8_t *) & (addr->DR), spi_priv->transfer_num);
                ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_rx_ch, (uint8_t *) & (addr->DR), spi_priv->recv_buf, spi_priv->transfer_num);
                return;
            }

            addr->SER = 0;
            ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch);
            ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch);
            spi_priv->dma_rx_ch = -1;
            spi_priv->dma_tx_ch = -1;
            spi_priv->status.busy = 0U;

            if (spi_priv->cb_event) {
                spi_priv->cb_event(spi_priv->idx, SPI_EVENT_TRANSFER_COMPLETE,spi_priv->arg);
            }
        } else if (spi_priv->mode == DW_SPI_TX) {
            ks_driver_dma_stop(spi_priv->dma_handle,spi_priv->dma_tx_ch);
            spi_priv->clk_num -= spi_priv->send_num;
            spi_priv->send_buf += spi_priv->send_num;

            if (spi_priv->clk_num) {
                if (spi_priv->clk_num >= SPI_DMA_BLOCK_SIZE) {
                    spi_priv->send_num = SPI_DMA_BLOCK_SIZE;
                } else {
                    spi_priv->send_num = spi_priv->clk_num;
                }

                dw_set_spi_dma_data_level(spi_priv);

                ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_tx_ch, spi_priv->send_buf, (uint8_t *) & (addr->DR), spi_priv->send_num);
                return;
            }

            while (addr->SR & DW_SPI_BUSY) {
                timeout ++;

                if (timeout > SPI_BUSY_TIMEOUT) {
                    return;
                }
            }

            ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch);
            spi_priv->dma_tx_ch = -1;
            addr->TXFTLR            = DW_SPI_TXFIFO_LV;
            addr->IMR               = DW_SPI_IMR_TXEIM;
        } else {


			spi_priv->clk_num -= spi_priv->recv_num;
            spi_priv->recv_buf += spi_priv->recv_num;

		
            if (spi_priv->clk_num) {
                if (spi_priv->clk_num >= SPI_DMA_BLOCK_SIZE) {
                    spi_priv->recv_num = SPI_DMA_BLOCK_SIZE;
                } else {
                    spi_priv->recv_num = spi_priv->clk_num;
                }

                dw_set_spi_dma_data_level(spi_priv);
                addr->CTRLR1 = spi_priv->recv_num - 1;
                ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_rx_ch, (uint8_t *) & (addr->DR), spi_priv->recv_buf, spi_priv->recv_num);
                addr->DR = DW_SPI_START_RX;
                return;
            }

            addr->SPIENR = DW_SPI_DISABLE;
            addr->SER = 0;
            ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch);
            spi_priv->dma_rx_ch = -1;
            spi_priv->status.busy = 0U;

            if (spi_priv->cb_event) {
                spi_priv->cb_event(spi_priv->idx, SPI_EVENT_RX_COMPLETE,spi_priv->arg);
            }
        }
    }

    spi_priv->status.busy = 0U;
}

/**
  \brief sending data to SPI transmitter with DMA,(received data is ignored).
*/
static int32_t dw_spi_send_dma(dw_spi_priv_t *spi_priv, const void *data, uint32_t num)
{
    dma_config_t config;


    config.src_inc  = DMA_ADDR_INC;
    config.dst_inc  = DMA_ADDR_CONSTANT;
    config.src_tw   = 1;
    config.dst_tw   = 1;

    if (spi_priv->idx == 0) {
		if(spi_priv->spi_mode == SPI_MODE_MASTER){
        	config.hs_if    = DMA_SPIM_TX;
		}
		else{
			config.hs_if    = DMA_SPIS_TX;
		}
    } else {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    config.type     = DMA_MEM2PERH;
    config.ch_mode   = DMA_HANDSHAKING_HARDWARE;

    if (spi_priv->dma_tx_ch == -1) {
        spi_priv->dma_tx_ch = ks_driver_dma_alloc_channel(spi_priv->dma_handle,-1);

        if (spi_priv->dma_tx_ch == -1) {
            return ERR_SPI(DRV_ERROR_BUSY);
        }
    }

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
    uint8_t *ptxbuffer = (uint8_t *)data;


	ks_cpu_dcache_clean(ptxbuffer,num);

    spi_priv->send_buf      = ptxbuffer;
    spi_priv->clk_num       = num;
    spi_priv->send_num      = num;

    if (spi_priv->clk_num > SPI_DMA_BLOCK_SIZE) {
        spi_priv->send_num = SPI_DMA_BLOCK_SIZE;
    } else {
        spi_priv->send_num = spi_priv->clk_num;
    }

    int32_t ret = ks_driver_dma_config_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch, &config, dw_spi_dma_event_cb, (void*)spi_priv);

    if (ret < 0) {
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch);
        return ret;
    }

    addr->SPIENR    = DW_SPI_DISABLE; /* enable SPI */
    dw_spi_set_mode(spi_priv, DW_SPI_TX);
    dw_set_spi_dma_data_level(spi_priv);
    addr->DMACR     = DW_SPI_TDMAE;
    addr->SER       = spi_priv->enable_slave;
    addr->SPIENR  = DW_SPI_ENABLE;

    ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_tx_ch, spi_priv->send_buf, (uint8_t *) & (addr->DR), spi_priv->send_num);
    return 0;
}

/**
  \brief receiving data from SPI receiver with DMA.
*/
static int32_t dw_spi_receive_dma(dw_spi_priv_t *spi_priv, void *data, uint32_t num)
{
    dma_config_t config;

    config.src_inc  = DMA_ADDR_CONSTANT;
    config.dst_inc  = DMA_ADDR_INC;
    config.src_tw   = 1;
    config.dst_tw   = 1;

    if (spi_priv->idx == 0) {
		if(spi_priv->spi_mode == SPI_MODE_MASTER){
        	config.hs_if    = DMA_SPIM_RX;
		}
		else{
			config.hs_if    = DMA_SPIS_RX;
		}
    } else {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    config.type     = DMA_PERH2MEM;
    config.ch_mode   = DMA_HANDSHAKING_HARDWARE;

    if (spi_priv->dma_rx_ch == -1) {
        spi_priv->dma_rx_ch = ks_driver_dma_alloc_channel(spi_priv->dma_handle,-1);

        if (spi_priv->dma_rx_ch == -1) {
            return ERR_SPI(DRV_ERROR_BUSY);
        }
    }

    uint8_t *prx_buffer = (uint8_t *)data;

	ks_cpu_dcache_invalidate(prx_buffer,num);

    spi_priv->recv_buf      = prx_buffer;
    spi_priv->clk_num       = num;

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
    int32_t ret;

    if (spi_priv->clk_num > SPI_DMA_BLOCK_SIZE) {
        spi_priv->recv_num = SPI_DMA_BLOCK_SIZE;
    } else {
        spi_priv->recv_num = spi_priv->clk_num;
    }

    ret = ks_driver_dma_config_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch, &config, dw_spi_dma_event_cb, spi_priv);

    if (ret < 0) {
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch);
        return ret;
    }

    addr->SPIENR    = DW_SPI_DISABLE; /* enable SPI */
    dw_spi_set_mode(spi_priv, DW_SPI_RX);
    dw_set_spi_dma_data_level(spi_priv);

    addr->CTRLR1 = spi_priv->recv_num - 1;
    addr->SER = spi_priv->enable_slave;
    addr->DMACR = DW_SPI_RDMAE;
    addr->SPIENR  = DW_SPI_ENABLE;


    ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_rx_ch, (uint8_t *) & (addr->DR), spi_priv->recv_buf, spi_priv->recv_num);
    addr->DR = DW_SPI_START_RX;
    return 0;
}

/**
  \brief sending/receiving data to/from SPI transmitter/receiver with DMA.
*/
static int32_t dw_spi_transfer_dma(dw_spi_priv_t *spi_priv, const void *data_out, void *data_in, uint32_t num_out, uint32_t num_in)
{
    dma_config_t tx_config, rx_config;

    tx_config.src_inc  = DMA_ADDR_INC;
    tx_config.dst_inc  = DMA_ADDR_CONSTANT;
    tx_config.src_tw   = 1;
    tx_config.dst_tw   = 1;

    if (spi_priv->idx == 0) {
		if(spi_priv->spi_mode == SPI_MODE_MASTER){
        	tx_config.hs_if    = DMA_SPIM_TX;
		}
		else{
			tx_config.hs_if    = DMA_SPIS_TX;
		}
    } else {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    tx_config.type     = DMA_MEM2PERH;

    rx_config.src_inc  = DMA_ADDR_CONSTANT;
    rx_config.dst_inc  = DMA_ADDR_INC;
    rx_config.src_tw   = 1;
    rx_config.dst_tw   = 1;

    if (spi_priv->idx == 0) {
		if(spi_priv->spi_mode == SPI_MODE_MASTER){
        	rx_config.hs_if    = DMA_SPIM_RX;
		}
		else{
			rx_config.hs_if    = DMA_SPIS_RX;
		}
    } else {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    rx_config.type     = DMA_PERH2MEM;

    if (spi_priv->dma_tx_ch == -1) {
        spi_priv->dma_tx_ch = ks_driver_dma_alloc_channel(spi_priv->dma_handle,-1);

        if (spi_priv->dma_tx_ch == -1) {
            return ERR_SPI(DRV_ERROR_BUSY);
        }
    }

    if (spi_priv->dma_rx_ch == -1) {
        spi_priv->dma_rx_ch  = ks_driver_dma_alloc_channel(spi_priv->dma_handle,-1);

        if (spi_priv->dma_rx_ch == -1) {
            ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch);
            return ERR_SPI(DRV_ERROR_BUSY);
        }
    }

    uint8_t *ptx_buffer = (uint8_t *)data_out;
    uint8_t *prx_buffer = (uint8_t *)data_in;
	
	ks_cpu_dcache_clean(ptx_buffer,num_out);
	
	ks_cpu_dcache_invalidate(prx_buffer,num_in);
	
    spi_priv->send_buf      = ptx_buffer;
    spi_priv->recv_buf      = prx_buffer;
    spi_priv->clk_num       = (num_out > num_in) ? num_out : num_in;

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
    int32_t ret;

    if (spi_priv->clk_num >= SPI_DMA_BLOCK_SIZE) {
        spi_priv->transfer_num = SPI_DMA_BLOCK_SIZE;
    } else {
        spi_priv->transfer_num = spi_priv->clk_num;
    }

    ret = ks_driver_dma_config_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch, &tx_config, dw_spi_dma_event_cb, spi_priv);

    if (ret < 0) {
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch);
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch);
        return ret;
    }

    ret = ks_driver_dma_config_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch, &rx_config, dw_spi_dma_event_cb, spi_priv);

    if (ret < 0) {
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch);
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch);
        return ret;
    }

    addr->SPIENR    = DW_SPI_DISABLE;   /* disable SPI */
    addr->DMACR   = DW_SPI_RDMAE | DW_SPI_TDMAE;
    dw_spi_set_mode(spi_priv, DW_SPI_TXRX);
    dw_set_spi_dma_data_level(spi_priv);
    addr->SER = spi_priv->enable_slave;
    addr->SPIENR    = DW_SPI_ENABLE;    /* enable SPI */


    ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_rx_ch, (uint8_t *) & (addr->DR), spi_priv->recv_buf, spi_priv->transfer_num);
    ks_driver_dma_start(spi_priv->dma_handle,spi_priv->dma_tx_ch, spi_priv->send_buf, (uint8_t *) & (addr->DR), spi_priv->transfer_num);

    return 0;
}
#endif


/**
  \brief       Set the SPI datawidth.
  \param[in]   handle     spi handle
  \param[in]   datawidth  date frame size in bits
  \return      error code
*/
int32_t dw_spi_config_datawidth(spi_handle_t handle, uint32_t datawidth)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    addr->SPIENR    = DW_SPI_DISABLE;
    addr->IMR       = DW_SPI_INT_DISABLE;

    if ((datawidth >= (DW_SPI_DATASIZE_4 + 1)) && (datawidth <= (DW_SPI_DATASIZE_16  + 1))) {
        uint16_t temp = addr->CTRLR0;
        temp &= 0xfff0;         /* temp has the value of CTRLR0 with DFS being cleared.*/
        temp |= (datawidth - 1);    /* get the final CTRLR0 after datawidth config. */
        addr->CTRLR0 = temp;    /* write CTRLR0 */

        spi_priv->state |= SPI_CONFIGURED;
        return 0;
    }

    return ERR_SPI(SPI_ERROR_DATA_BITS);
}

/**
  \brief       Set the SPI clock divider.
  \param[in]   handle   spi handle
  \param[in]   baud     spi baud rate
  \return      error code
*/
int32_t dw_spi_config_baudrate(spi_handle_t handle, uint32_t baud)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    addr->SPIENR    = DW_SPI_DISABLE;
    addr->IMR       = DW_SPI_INT_DISABLE;

    int32_t sckdv = ks_os_get_apb_clock() / baud;
    /*BAUDR reg  max 15 bit set */
    if (sckdv < 0x10000) {
        addr->BAUDR =  sckdv;
    } else {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    spi_priv->state |= SPI_CONFIGURED;

    return 0;
}

/**
  \brief       Set the SPI polarity.
  \param[in]   addr  pointer to register address
  \param[in]   polarity spi polarity
  \return      error code
*/
static int32_t dw_spi_set_polarity(dw_spi_reg_t *addr, DW_SPI_CLOCK_POLARITY polarity)
{
    /* To config the polarity, we can set the SCPOL bit(CTRLR0[7]) as below:
     *     0 - inactive state of serial clock is low
     *     1 - inactive state of serial clock is high
     */
    switch (polarity) {
        case DW_SPI_CLOCK_POLARITY_LOW:
            addr->CTRLR0 &= (~DW_SPI_POLARITY);
            break;

        case DW_SPI_CLOCK_POLARITY_HIGH:
            addr->CTRLR0 |= DW_SPI_POLARITY;
            break;

        default:
            return -1;
    }

    return 0;
}

/**
  \brief       Set the SPI Phase.
  \param[in]   addr  pointer to register address
  \param[in]   phase    Serial clock phase
  \return      error code
*/
static int32_t dw_spi_set_phase(dw_spi_reg_t *addr, DW_SPI_CLOCK_PHASE phase)
{
    switch (phase) {
        case DW_SPI_CLOCK_PHASE_MIDDLE:
            addr->CTRLR0 &= (~DW_SPI_PHASE);
            break;

        case DW_SPI_CLOCK_PHASE_START:
            addr->CTRLR0 |= DW_SPI_PHASE;
            break;

        default:
            return -1;
    }

    return 0;
}

/**
  \brief       config the SPI format.
  \param[in]   handle   spi handle
  \param[in]   format   spi format. \ref spi_format_e
  \return      error code
*/
int32_t dw_spi_config_format(spi_handle_t handle, spi_format_e format)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    addr->SPIENR    = DW_SPI_DISABLE;
    addr->IMR       = DW_SPI_INT_DISABLE;

    switch (format) {
        case SPI_FORMAT_CPOL0_CPHA0:
            dw_spi_set_polarity(addr, DW_SPI_CLOCK_POLARITY_LOW);
            dw_spi_set_phase(addr, DW_SPI_CLOCK_PHASE_MIDDLE);
            break;

        case SPI_FORMAT_CPOL0_CPHA1:
            dw_spi_set_polarity(addr, DW_SPI_CLOCK_POLARITY_LOW);
            dw_spi_set_phase(addr, DW_SPI_CLOCK_PHASE_START);
            break;

        case SPI_FORMAT_CPOL1_CPHA0:
            dw_spi_set_polarity(addr, DW_SPI_CLOCK_POLARITY_HIGH);
            dw_spi_set_phase(addr, DW_SPI_CLOCK_PHASE_MIDDLE);
            break;

        case SPI_FORMAT_CPOL1_CPHA1:
            dw_spi_set_polarity(addr, DW_SPI_CLOCK_POLARITY_HIGH);
            dw_spi_set_phase(addr, DW_SPI_CLOCK_PHASE_START);
            break;

        default:
            return ERR_SPI(SPI_ERROR_FRAME_FORMAT);
    }

    spi_priv->state |= SPI_CONFIGURED;

    return 0;
}


/**
  \brief       config the SPI mode.
  \param[in]   handle   spi handle
  \param[in]   mode     spi mode. \ref spi_mode_e
  \return      error code
*/
int32_t dw_spi_config_mode(spi_handle_t handle, spi_mode_e  mode)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    addr->SPIENR    = DW_SPI_DISABLE;
    addr->IMR       = DW_SPI_INT_DISABLE;

	if(spi_priv->spi_mode  == SPI_MODE_MASTER){

		/* if master, config RX_SAMPLE_DLY to reach higher baud rate,
		 SSI_RX_DLY_SR_DEPTH is set to 4 in hardware
		 (refer to Table 6-31 of DW_apb_ssi Databook),
		 so the max value can be 4 */
	    addr->RX_SAMPLE_DLY = 4;
		if (mode != SPI_MODE_MASTER) {
			 return ERR_SPI(DRV_ERROR_PARAMETER);
		}
	}

	if(spi_priv->spi_mode  == SPI_MODE_SLAVE){

		if (mode != SPI_MODE_SLAVE) {
			 return ERR_SPI(DRV_ERROR_PARAMETER);
		}

	}

	spi_priv->state |= SPI_CONFIGURED;

    return 0;
}

/**
  \brief       config the SPI mode.
  \param[in]   handle   spi handle
  \param[in]   order    spi bit order.\ref spi_bit_order_e
  \return      error code
*/
int32_t dw_spi_config_bit_order(spi_handle_t handle, spi_bit_order_e order)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    if (order == SPI_ORDER_MSB2LSB) {
        spi_priv->state |= SPI_CONFIGURED;
        return 0;
    }

    return ERR_SPI(SPI_ERROR_BIT_ORDER);;
}

/**
  \brief       config the SPI slave select mode.
  \param[in]   handle   spi handle
  \param[in]   ss_mode  spi slave select mode. \ref spi_ss_mode_e
  \return      error code
*/
int32_t dw_spi_config_ss_mode(spi_handle_t handle, spi_ss_mode_e ss_mode)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    switch (ss_mode) {
        case SPI_SS_MASTER_SW:
            spi_priv->ss_mode = SPI_SS_MASTER_SW;
            break;

        case SPI_SS_MASTER_HW_OUTPUT:
            spi_priv->ss_mode = SPI_SS_MASTER_HW_OUTPUT;
            break;

        case SPI_SS_MASTER_HW_INPUT:
            spi_priv->ss_mode = SPI_SS_MASTER_HW_INPUT;
            break;

        case SPI_SS_SLAVE_HW:
            spi_priv->ss_mode = SPI_SS_SLAVE_HW;
            break;

        case SPI_SS_SLAVE_SW:
            spi_priv->ss_mode = SPI_SS_SLAVE_SW;
            break;

        default:
            return ERR_SPI(SPI_ERROR_SS_MODE);
    }

    spi_priv->state |= SPI_CONFIGURED;

    return 0;
}

/**
  \brief       Set the SPI mode.
  \param[in]   addr  pointer to register address
  \param[in]   mode     SPI_Mode
  \return      error code
*/
static int32_t dw_spi_set_mode(spi_handle_t handle, DW_SPI_MODE mode)
{
    dw_spi_priv_t *spi_priv = handle;
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    /* It is impossible to write to this register when the SSI is enabled.*/
    /* we can set the TMOD to config transfer mode as below:
     *     TMOD_BIT9  TMOD_BIT8      transfer mode
     *         0          0         transmit & receive
     *         0          1           transmit only
     *         1          0           receive only
     *         1          1             reserved
     */
    switch (mode) {
        case DW_SPI_TXRX:
            addr->CTRLR0 &= (~DW_SPI_TMOD_BIT8);
            addr->CTRLR0 &= (~DW_SPI_TMOD_BIT9);
            break;

        case DW_SPI_TX:
            addr->CTRLR0 |= DW_SPI_TMOD_BIT8;
            addr->CTRLR0 &= (~DW_SPI_TMOD_BIT9);
            break;

        case DW_SPI_RX:
            addr->CTRLR0 &= (~DW_SPI_TMOD_BIT8);
            addr->CTRLR0 |= DW_SPI_TMOD_BIT9;
            break;

        default:
            addr->CTRLR0 |= DW_SPI_TMOD_BIT8;
            addr->CTRLR0 |= DW_SPI_TMOD_BIT9;
            break;
    }

    spi_priv->mode = mode;
    asm volatile ("dsb");
    asm volatile ("isb");
    return 0;
}

/*!
 * \brief push next tx block to SPI FIFO, for TXRX and TX mode.
 *
 * update:
 * - send_buf
 * - send_num
 * - transfer_num (about to delete)
 * - clk_num
 *
 * @warning In TXRX mode, reading data should be taken care of before calling this function.
 *
 * @warning spi_priv->clk_num should be updated after this function, if needed.
 *
 * \param idx       SPI device index
 * \param spi_priv  SPI private type pointer
 */
static void _dw_spi_push_next_tx_block(int32_t idx, dw_spi_priv_t *spi_priv)
{
    ks_os_irq_mask_all();

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    /*judge next count to fill*/
    const uint8_t tx_space = DW_SPI_FIFO_MAX_LV - addr->TXFLR;
    spi_priv->transfer_num = DRIVER_MIN(spi_priv->tot_num_left, tx_space);

    /*fill as much as needed*/
    for (uint32_t i=0; i<spi_priv->transfer_num; i++){
        if (0 == (spi_priv->send_num_left)) {
            addr->DR = 0x0;
        }
        else {
            addr->DR = *spi_priv->send_buf;
            spi_priv->send_buf++;
            spi_priv->send_num_left--;
        }
    }

    /*update tot_num_left*/
    if (spi_priv->tot_num_left > spi_priv->transfer_num){
        spi_priv->tot_num_left -= spi_priv->transfer_num;
    }
    else{
        spi_priv->tot_num_left = 0;
        /*no data left outside, change water marker to 1 Byte for TX*/
        addr->TXFTLR = 0;
    }

    ks_os_irq_unmask_all();
}

static void _dw_spi_pull_all_rx_data(int32_t idx, dw_spi_priv_t *spi_priv)
{
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
    uint8_t dummy_byte;

    /* read data out from rx FIFO */
    while (addr->RXFLR > 0) {
        if (spi_priv->recv_num_left > 0){
            /*fill recv buffer*/
            *spi_priv->recv_buf = addr->DR;
            spi_priv->recv_buf++;
            spi_priv->recv_num_left--;
        }
        else{
            /*if extending recv_buf_len, just read and drop the byte*/
            dummy_byte = addr->DR;
        }
    }
}

static void _dw_spi_stop_tx(int32_t idx, dw_spi_priv_t *spi_priv)
{
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
    uint8_t temp;

    addr->SER = 0;
    addr->SPIENR = DW_SPI_DISABLE;
    spi_priv->status.busy = 0U;

    temp = addr->IMR;
    temp &= ~DW_SPI_IMR_TXEIM;
    addr->IMR = temp;
}

static int _dw_spi_wait_bus_free(dw_spi_reg_t *addr)
{
    volatile uint32_t timecount = 0;

    while (addr->SR & DW_SPI_BUSY) {
        if (++timecount >= SPI_BUSY_TIMEOUT) {
            return 1;
        }
    }

    return 0;
}

/**
  \brief       interrupt service function for receive FIFO full interrupt .
  \param[in]   spi_priv pointer to spi private.
*/
static void dw_spi_intr_rx_full(int32_t idx, dw_spi_priv_t *spi_priv)
{
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
    uint8_t temp = addr->ICR;

    /*read all valid data*/
    _dw_spi_pull_all_rx_data(idx, spi_priv);

    if (0 == spi_priv->recv_num_left){
        /*done*/

        /*all data has been taken*/
        if (_dw_spi_wait_bus_free(addr)) {
            kprintf("SPI_BUSY_TIMEOUT  \r\n");
            _dw_spi_stop_tx(idx, spi_priv);
            return;
        }

        /*read and drop all data left in FIFO, should be ZERO*/
        _dw_spi_pull_all_rx_data(idx, spi_priv);

        temp = addr->IMR;
        temp &= ~DW_SPI_IMR_RXFIM;
        addr->IMR = temp;

        addr->SER = 0;
        addr->SPIENR = DW_SPI_DISABLE;
        spi_priv->status.busy = 0U;

        if (spi_priv->cb_event) {
            spi_priv->cb_event(idx, SPI_EVENT_RX_COMPLETE, spi_priv->arg);
            return;
        }
    }
    else{
        /*if number left is not big enough, re-calc water marker*/
        addr->RXFTLR = ((spi_priv->recv_num_left <= DW_SPI_FIFO_MAX_LV) ? spi_priv->recv_num_left : (DW_SPI_FIFO_MAX_LV >> 1)) - 1;
    }
}

/**
  \brief       interrupt service function for transmit FIFO empty interrupt.
  \param[in]   spi_priv pointer to spi private.
*/
static void dw_spi_intr_tx_empty(int32_t idx, dw_spi_priv_t *spi_priv)
{
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);
    uint8_t temp = addr->ICR;

    /* transfer mode:transmit & receive */
    uint32_t i = 0u;

    if (0 == spi_priv->tot_num_left) {
        /*no pending data outside*/

        /*read received data in FIFO*/
        if (spi_priv->mode == DW_SPI_TXRX) {
            _dw_spi_pull_all_rx_data(idx, spi_priv);
        }

        /*all data has been taken*/
        if (_dw_spi_wait_bus_free(addr)) {
            kprintf("SPI_BUSY_TIMEOUT  \r\n");
            _dw_spi_stop_tx(idx, spi_priv);
            return;
        }

        /*read received data in FIFO again, maybe new data when waiting for bus free*/
        if (spi_priv->mode == DW_SPI_TXRX) {
            _dw_spi_pull_all_rx_data(idx, spi_priv);
        }

        _dw_spi_stop_tx(idx, spi_priv);

        spi_priv->send_num_left = 0;

        if (spi_priv->cb_event) {
            spi_priv->cb_event(idx
                               , ((spi_priv->mode == DW_SPI_TXRX) ? SPI_EVENT_TRANSFER_COMPLETE : SPI_EVENT_TX_COMPLETE)
                               , spi_priv->arg
                               );
        }
    }
    else{
        /*normal round*/

        if (spi_priv->mode == DW_SPI_TXRX) {
            /*read all data valid*/
            _dw_spi_pull_all_rx_data(idx, spi_priv);
        }

        /*write as many as possible*/
        _dw_spi_push_next_tx_block(idx, spi_priv);
    }
}

/**
  \brief       handler the interrupt.
  \param[in]   spi      Pointer to \ref SPI_RESOURCES
*/
void dw_spi_irqhandler(void* arg)
{
    dw_spi_priv_t *spi_priv = ( dw_spi_priv_t *)arg;
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    uint32_t intr = addr->ISR;

    /* deal with receive FIFO full interrupt */
    if (intr & DW_SPI_RXFIFO_FULL) {
        dw_spi_intr_rx_full(spi_priv->idx, spi_priv);
    }
    /* deal with transmit FIFO empty interrupt */
    else if (intr & DW_SPI_TXFIFO_EMPTY) {
        dw_spi_intr_tx_empty(spi_priv->idx, spi_priv);
    }
}


/**
  \brief       Initialize SPI Interface. 1. Initializes the resources needed for the SPI interface 2.registers event callback function
  \param[in]   idx spi index
  \param[in]   cb_event  event call back function \ref spi_event_cb_t
  \return      return spi handle if success
*/
spi_handle_t dw_spi_initialize(int32_t idx, int32_t	mode,spi_event_cb_t cb_event,void* arg)
{
    uint32_t base ;
    uint32_t irq ;

    dw_spi_priv_t *spi_priv ;


	if(mode == SPI_MODE_MASTER){
		base = SPIM_BASE;
		irq = IRQ_VEC_SPIM;
		spi_priv= &spim_instance[idx];
	}else{
		base = SPIS_BASE;
		irq = IRQ_VEC_SPIS;
		spi_priv= &spis_instance[idx];
	}
	spi_priv->idx = idx;
    spi_priv->base = base;
    spi_priv->irq  = irq;

    spi_priv->spi_mode  = mode;


    spi_priv->cb_event          = cb_event;
	spi_priv->arg   = arg;
    spi_priv->status.busy       = 0U;
    spi_priv->status.data_lost  = 0U;
    spi_priv->status.mode_fault = 0U;
    spi_priv->enable_slave      = 1U;
    spi_priv->state             = SPI_INITIALIZED;

#if defined SPI_DMA_SURPPORT 
    spi_priv->dma_tx_ch = -1;
    spi_priv->dma_rx_ch = -1;
	ks_driver_dma_init();
	spi_priv->dma_handle = ks_driver_dma_get_handle();
	//spi_priv->dma_txbuffer = s_spi_dma_txbuffer;
	//spi_priv->dma_rxbuffer = s_spi_dma_rxbuffer;
#endif

	ks_os_irq_create(spi_priv->irq, dw_spi_irqhandler, (void*) spi_priv);
	ks_os_irq_enable(spi_priv->irq);
	ks_os_irq_map_target(spi_priv->irq, 1);

	ks_shell_add_cmds(spi_driver_cmds, sizeof(spi_driver_cmds) / sizeof(cmd_proc_t));

    return (spi_handle_t)spi_priv;
}

/**
  \brief       De-initialize SPI Interface. stops operation and releases the software resources used by the interface
  \param[in]   handle spi handle to operate.
  \return      error code
*/
int32_t dw_spi_uninitialize(spi_handle_t handle)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;
    ks_os_irq_disable(spi_priv->irq);


    spi_priv->cb_event          = NULL;
	spi_priv->arg				= NULL;
    spi_priv->state             = 0U;
    spi_priv->status.busy       = 0U;
    spi_priv->status.data_lost  = 0U;
    spi_priv->status.mode_fault = 0U;

#if defined  SPI_DMA_SURPPORT 

    if (spi_priv->dma_tx_ch != -1) {
        ks_driver_dma_stop(spi_priv->dma_handle,spi_priv->dma_tx_ch);
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_tx_ch);
        spi_priv->dma_tx_ch = -1;
    }

    if (spi_priv->dma_rx_ch != -1) {
        ks_driver_dma_stop(spi_priv->dma_handle,spi_priv->dma_rx_ch);
        ks_driver_dma_release_channel(spi_priv->dma_handle,spi_priv->dma_rx_ch);
        spi_priv->dma_rx_ch = -1;
    }

#endif



    return 0;
}




/**
  \brief       config spi mode.
  \param[in]   handle spi handle to operate.
  \param[in]   baud      spi baud rate. if negative, then this attribute not changed
  \param[in]   mode      \ref spi_mode_e . if negative, then this attribute not changed
  \param[in]   format    \ref spi_format_e . if negative, then this attribute not changed
  \param[in]   order     \ref spi_bit_order_e . if negative, then this attribute not changed
  \param[in]   ss_mode   \ref spi_ss_mode_e . if negative, then this attribute not changed
  \param[in]   bit_width spi data bitwidth: (1 ~ SPI_DATAWIDTH_MAX) . if negative, then this attribute not changed
  \return      error code
*/
int32_t dw_spi_config(spi_handle_t handle,
                       int32_t          baud,
                       spi_mode_e       mode,
                       spi_format_e     format,
                       spi_bit_order_e  order,
                       spi_ss_mode_e    ss_mode,
                       int32_t          bit_width)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;

    if ((spi_priv->state & SPI_INITIALIZED) == 0U) {
        return ERR_SPI(DRV_ERROR_UNSUPPORTED);
    }

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    spi_priv->status.busy       = 0U;
    spi_priv->status.data_lost  = 0U;
    spi_priv->status.mode_fault = 0U;

    int32_t ret = 0;

    if (baud >= 0) {
        ret = dw_spi_config_baudrate(handle, baud);

        if (ret < 0) {
            return ret;
        }
    }


	ret = dw_spi_config_mode(handle, mode);

	if (ret < 0) {
		return ret;
	}



	ret = dw_spi_config_format(handle, format);

	if (ret < 0) {
		return ret;
	}



	ret = dw_spi_config_bit_order(handle, order);

	if (ret < 0) {
		return ret;
	}



	ret = dw_spi_config_ss_mode(handle, ss_mode);

	if (ret < 0) {
		return ret;
	}



	ret = dw_spi_config_datawidth(handle, bit_width);

	if (ret < 0) {
		return ret;
	}



    spi_priv->state |= SPI_CONFIGURED;

    return 0;
}


/**
  \brief       sending data to SPI transmitter,(received data is ignored).
               if non-blocking mode, this function only start the sending,
               \ref spi_event_e is signaled when operation completes or error happens.
               \ref dw_spi_get_status can indicates operation status.
               if blocking mode, this function return after operation completes or error happens.
  \param[in]   handle spi handle to operate.
  \param[in]   data  Pointer to buffer with data to send to SPI transmitter. data_type is : uint8_t for 1..8 data bits, uint16_t for 9..16 data bits,uint32_t for 17..32 data bits,
  \param[in]   num   Number of data items to send.
  \return      error code
*/
int32_t dw_spi_send(spi_handle_t handle, const void *data, uint32_t num)
{
    if (handle == NULL || data == NULL || num == 0) {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    dw_spi_priv_t *spi_priv = handle;

    if ((spi_priv->state & SPI_CONFIGURED) == 0U) {
        return ERR_SPI(DRV_ERROR_UNSUPPORTED);
    }

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    spi_priv->status.busy       = 1U;
    spi_priv->status.data_lost  = 0U;
    spi_priv->status.mode_fault = 0U;
    spi_priv->tot_num           = num;
    spi_priv->send_num_left     = num;
    spi_priv->tot_num_left      = num;
    spi_priv->send_buf          = (uint8_t *)data;

    spi_priv->transfer_stat     = TRANSFER_STAT_SEND;

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (_dw_spi_wait_bus_free(addr)){
        kprintf("SPI_BUSY_TIMEOUT  \r\n");
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    addr->SPIENR    = DW_SPI_DISABLE;

    dw_spi_set_mode(spi_priv, DW_SPI_TX);

#if defined  SPI_DMA_SURPPORT 
    /* using dma when send count greater than DW_SPI_FIFO_MAX_LV */
    if (spi_priv->clk_num >= DW_SPI_FIFO_MAX_LV) {
        if (dw_spi_send_dma(spi_priv, data, num) == 0) {
            return 0;
        }
    }
#endif

    /* set TX water mark to:
     * - 1/2 FIFO depth (if need 2 or more round to receive)
     * - 1 Byte (if only 1 round is needed)
     */
    addr->TXFTLR = (spi_priv->tot_num > DW_SPI_FIFO_MAX_LV) ? (DW_SPI_FIFO_MAX_LV >> 1) : 0;

    /* enable SPI device */
    addr->SPIENR            = DW_SPI_ENABLE; /* enable SPI */

    /* fill FIFO before start */
    _dw_spi_push_next_tx_block(spi_priv->idx, spi_priv);

    /* enable transmit FIFO empty interrupt */
    addr->IMR               = DW_SPI_IMR_TXEIM;

    /* start transfering when SER is not zero */
    addr->SER       = spi_priv->enable_slave;

    return 0;
}

/**
\brief      receiving data from SPI receiver. if non-blocking mode, this function only start the receiving,
            \ref spi_event_e is signaled when operation completes or error happens.
            \ref dw_spi_get_status can indicates operation status.
            if blocking mode, this function return after operation completes or error happens.
\param[in]  handle spi handle to operate.
\param[out] data  Pointer to buffer for data to receive from SPI receiver
\param[in]  num   Number of data items to receive
\return     error code
*/
int32_t dw_spi_receive(spi_handle_t handle, void *data, uint32_t num)
{
    /* Add: [from v2.1.3h1] @param num should NOT be greater than @sa DW_SPI_NDF_MAX */
    if (handle == NULL || data == NULL || num == 0 || num > DW_SPI_NDF_MAX) {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    dw_spi_priv_t *spi_priv = handle;

    if ((spi_priv->state & SPI_CONFIGURED) == 0U) {
        return ERR_SPI(DRV_ERROR_UNSUPPORTED);
    }

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    spi_priv->status.busy       = 1U;
    spi_priv->status.data_lost  = 0U;
    spi_priv->status.mode_fault = 0U;
    spi_priv->tot_num           = num;
    spi_priv->recv_num_left     = num;
    spi_priv->recv_buf          = (uint8_t *)data;

    spi_priv->transfer_stat     = TRANSFER_STAT_RCV;

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (_dw_spi_wait_bus_free(addr)){
        kprintf("SPI_BUSY_TIMEOUT  \r\n");
        return ERR_SPI(DRV_ERROR_BUSY);
    }

#if defined  SPI_DMA_SURPPORT 
//#if 0 

    /* using dma when receive count greater than DW_SPI_FIFO_MAX_LV */
    if (spi_priv->tot_num >= DW_SPI_FIFO_MAX_LV) {
        if (dw_spi_receive_dma(spi_priv, data, num) == 0) {
            return 0;
        }
    }

#endif

    addr->SPIENR    = DW_SPI_DISABLE;

    dw_spi_set_mode(spi_priv, DW_SPI_RX);
    addr->SER       = spi_priv->enable_slave;

    /* set RX water mark to 1/2 FIFO depth */
    addr->RXFTLR    = ((num <= DW_SPI_FIFO_MAX_LV) ? num : (DW_SPI_FIFO_MAX_LV>>1)) - 1;
//    addr->RXFTLR    = num-1;

    addr->IMR       = DW_SPI_IMR_RXFIM;

    if (spi_priv->spi_mode == SPI_MODE_MASTER){
        /* set CTRLR1.NDF to target number - 1 */
        addr->CTRLR1    = num - 1;
        /* trigger RX start (Figure 2-24) */
        addr->DR        = DW_SPI_START_RX;
    }

    addr->SPIENR    = DW_SPI_ENABLE; /* enable SPI */
    addr->DR        = DW_SPI_START_RX;

    return 0;
}

/**
  \brief       sending/receiving data to/from SPI transmitter/receiver.
               if non-blocking mode, this function only start the transfer,
               \ref spi_event_e is signaled when operation completes or error happens.
               \ref dw_spi_get_status can indicates operation status.
               if blocking mode, this function return after operation completes or error happens.
  \param[in]   handle spi handle to operate.
  \param[in]   data_out  Pointer to buffer with data to send to SPI transmitter
  \param[out]  data_in   Pointer to buffer for data to receive from SPI receiver
  \param[in]   num_out      Number of data items to send
  \param[in]   num_in       Number of data items to receive
  \param[in]   block_mode   blocking and non_blocking to selcect
  \return      error code
*/
int32_t dw_spi_transfer(spi_handle_t handle, const void *data_out, void *data_in, uint32_t num_out, uint32_t num_in)
{
    if (handle == NULL || data_in == NULL || num_out == 0 || num_in == 0 || data_out == NULL) {
        return ERR_SPI(DRV_ERROR_PARAMETER);
    }

    dw_spi_priv_t *spi_priv = handle;

    if ((spi_priv->state & SPI_CONFIGURED) == 0U) {
        return ERR_SPI(DRV_ERROR_UNSUPPORTED);
    }

    if (spi_priv->status.busy) {
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    spi_priv->status.busy = 1U;
    spi_priv->status.data_lost = 0U;
    spi_priv->status.mode_fault = 0U;

    spi_priv->tot_num = DRIVER_MAX(num_out, num_in);
    spi_priv->transfer_stat     = TRANSFER_STAT_TRAN;

    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    if (_dw_spi_wait_bus_free(addr)){
        kprintf("SPI_BUSY_TIMEOUT  \r\n");
        return ERR_SPI(DRV_ERROR_BUSY);
    }

    addr->SPIENR    = DW_SPI_DISABLE;   /* disable SPI */

    dw_spi_set_mode(spi_priv, DW_SPI_TXRX);

#if defined  SPI_DMA_SURPPORT

    if (spi_priv->tot_num >= DW_SPI_FIFO_MAX_LV) {
        if (dw_spi_transfer_dma(spi_priv, data_out, data_in, num_out, num_in) == 0) {
            return 0;
        }
    }

#endif
    spi_priv->send_buf      = (uint8_t *)data_out;
    spi_priv->recv_buf      = (uint8_t *)data_in ;
    spi_priv->send_num_left = num_out;
    spi_priv->recv_num_left = num_in ;
    spi_priv->tot_num_left  = spi_priv->tot_num;

    /* set TX water mark to 1/2 FIFO depth as default, may be updated in @sa _dw_spi_push_next_tx_block */
    addr->TXFTLR = DW_SPI_FIFO_MAX_LV >> 1;

    /* enable SPI device */
    addr->SPIENR            = DW_SPI_ENABLE;

    /* fill FIFO, and update TXFTLR as needed */
    _dw_spi_push_next_tx_block(spi_priv->idx, spi_priv);

    /* enable transmit FIFO empty interrupt */
    addr->IMR               |= DW_SPI_IMR_TXEIM;

    /* start transfering when SER is not zero */
    addr->SER               = spi_priv->enable_slave;   /* enable select cs */

    return 0;
}

/**
  \brief       abort spi transfer.
  \param[in]   handle spi handle to operate.
  \return      error code
*/
int32_t dw_spi_abort_transfer(spi_handle_t handle)
{
    SPI_NULL_PARAM_CHK(handle);

    dw_spi_priv_t *spi_priv = handle;
    dw_spi_reg_t *addr = (dw_spi_reg_t *)(spi_priv->base);

    addr->SPIENR = DW_SPI_DISABLE;
    spi_priv->status.busy = 0U;
    spi_priv->recv_buf = NULL;
    spi_priv->recv_num_left = 0;

    return 0;
}

/**
  \brief       Get SPI status.
  \param[in]   handle spi handle to operate.
  \return      SPI status \ref ARM_SPI_STATUS
*/
spi_status_t dw_spi_get_status(spi_handle_t handle)
{
    spi_status_t spi_status = {0};

    if (handle == NULL) {
        return spi_status;
    }

    dw_spi_priv_t *spi_priv = handle;

    return spi_priv->status;
}

/**
  \brief       Get spi transferred data count.
  \param[in]   handle  spi handle to operate.
  \return      number of data bytes transferred
*/
uint32_t dw_spi_get_data_count(spi_handle_t handle)
{
    uint32_t cnt = 0;

    if (handle == NULL) {
        return 0;
    }

    dw_spi_priv_t *spi_priv = handle;

    if (spi_priv->transfer_stat == TRANSFER_STAT_SEND) {
        cnt = spi_priv->tot_num - spi_priv->send_num_left;
    } else if (spi_priv->transfer_stat == TRANSFER_STAT_RCV) {
        cnt = spi_priv->tot_num - spi_priv->recv_num_left;
    } else if (spi_priv->transfer_stat == TRANSFER_STAT_TRAN) {
        cnt = spi_priv->tot_num - (spi_priv->recv_num_left > spi_priv->send_num_left ? spi_priv->recv_num_left : spi_priv->send_num_left);
    }

    return cnt;
}

/**
  \brief       Control the Slave Select signal (SS).
  \param[in]   handle  spi handle to operate.
  \param[in]   stat    SS state. \ref spi_ss_stat_e.
  \return      error code
*/
int32_t dw_spi_cs_control(spi_handle_t handle, uint32_t select)
{
    SPI_NULL_PARAM_CHK(handle);
    dw_spi_priv_t *spi_priv = handle;
	
	spi_priv->enable_slave = select;
    return 0;
}

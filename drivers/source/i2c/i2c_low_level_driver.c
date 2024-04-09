


#include <string.h>
#include "i2c_low_level_driver.h"
#include "ks_driver.h"
#include <ks_cache.h>
#include <ks_dma.h>

#include "ks_i2c.h"
#include "ks_os.h"

#define IIC_DMA_BLOCK_SIZE      2048
#define IIC_TX_ADDR_RETRY_MAX   5

#define ERR_IIC(errno) (errno)

#define HANDLE_PARAM_CHK(para, err)                                                                \
    do {                                                                                           \
        if ((int32_t)para == (int32_t)NULL) {                                                      \
            return (err);                                                                          \
        }                                                                                          \
    } while (0)


#define IIC_NULL_PARAM_CHK(para) HANDLE_PARAM_CHK(para, ERR_IIC(DRV_ERROR_PARAMETER))

#ifndef DRIVER_MAX
#define DRIVER_MAX(a, b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef DRIVER_MIN
#define DRIVER_MIN(a, b)    (((a) < (b)) ? (a) : (b))
#endif

#define IIC_GET_FAIR_TX_TL(tot_num) (((tot_num) > DW_IIC_FIFO_MAX_LV) ? (DW_IIC_FIFO_MAX_LV/2) : (tot_num))
#define IIC_GET_FAIR_RX_TL(tot_num) (((tot_num) > (DW_IIC_FIFO_MAX_LV-4)) ? (DW_IIC_FIFO_MAX_LV/2-1) : (tot_num-1))

typedef struct {
    uint8_t  *buf_orig;             /*original buf ptr, fixed after set*/
    uint32_t total_num_orig;        /*original total_num, fixed after set*/
    uint8_t  *trans_head_addr;      /*inner trans head addr*/
    uint8_t  *align_start_addr;     /*aligned start addr, >= buf_orig*/
    uint8_t  *align_end_addr;       /*aligned end addr, <= &buf_orig[total_num_orig]*/
    uint8_t  *trans_tail_addr;      /*inner trans tail addr*/
    uint32_t align_offset_head;     /*tmp-align head offset(split pos) if needed, 1 means 1 byte after start*/
    uint32_t align_offset_tail;     /*tmp-align tail offset(split pos) if needed, 1 means 1 byte before end */
}_iic_priv_dma_align_control_t;

typedef struct  {
    int32_t idx;
	uint32_t inited;
    uint32_t base;
    uint32_t irq;
    iic_event_cb_t cb_event;
	void* arg;
    volatile uint32_t rx_total_num;
    volatile uint32_t tx_total_num;
    uint8_t *rx_buf;
    uint8_t *tx_buf;
    uint32_t cmd_read_num;
    volatile uint32_t rx_cnt;
    volatile uint32_t tx_cnt;
    uint32_t status;             ///< status of iic transfer
    iic_mode_e mode;
	iic_speed_e speed;
	iic_address_mode_e addr_mode;
	S32 slave_addr;

	uint32_t fifo_tx_count;
	uint32_t fifo_rx_count;

	OSHandle dma_handle;
	i2c_dev_t* dev;
	
    uint8_t dma_use;
    uint8_t tx_addr_retry_cnt;

	//中断里存在dma_buffer拷贝和dcache 操作导致耗时过长 因此调整到任务中操作
	//uint8_t* dma_buffer;
	uint32_t read_total_num;
	uint32_t read_num;
	iic_trans_e tans_mode;
    int32_t dma_tx_ch;
    int32_t dma_rx_ch;
    volatile uint32_t send_num;
    volatile uint32_t recv_num;

	uint32_t dma_tx_count;
	uint32_t dma_rx_count;
	uint32_t dma_failure;

    _iic_priv_dma_align_control_t rx_dma_align_ctrl;
    _iic_priv_dma_align_control_t tx_dma_align_ctrl;
} _iic_priv_t;

static U8 s_i2c_dma_tx_head_buffer[CACHE_LINE_ALIGN_LEN] __attribute__ ((aligned (64)));
static U8 s_i2c_dma_tx_tail_buffer[CACHE_LINE_ALIGN_LEN] __attribute__ ((aligned (64)));
static U8 s_i2c_dma_rx_head_buffer[CACHE_LINE_ALIGN_LEN] __attribute__ ((aligned (64)));
static U8 s_i2c_dma_rx_tail_buffer[CACHE_LINE_ALIGN_LEN] __attribute__ ((aligned (64)));

static int32_t _iic_slave_send_dma(_iic_priv_t *iic_priv);
static int32_t _iic_slave_receive_dma(_iic_priv_t *iic_priv);
static _iic_priv_t iic_instance[CONFIG_IIC_NUM];

static int i2c_reg_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;
    _iic_priv_t *iic_priv = &iic_instance[0];
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
	
	ks_shell_printf(ctx->uart_id,"\r\n");
	ks_shell_printf(ctx->uart_id,"IC_CON : %x \r\n",addr->IC_CON);
	ks_shell_printf(ctx->uart_id,"IC_TAR : %x \r\n",addr->IC_TAR);
	ks_shell_printf(ctx->uart_id,"IC_SAR : %x \r\n",addr->IC_SAR);
	
	ks_shell_printf(ctx->uart_id,"IC_TX_TL : %x \r\n",addr->IC_TX_TL);
	ks_shell_printf(ctx->uart_id,"IC_RX_TL : %x \r\n",addr->IC_RX_TL);

	ks_shell_printf(ctx->uart_id,"IC_TXFLR : %x \r\n",addr->IC_TXFLR);
	ks_shell_printf(ctx->uart_id,"IC_RXFLR : %x \r\n",addr->IC_RXFLR);

	ks_shell_printf(ctx->uart_id,"IC_SS_SCL_HCNT : %x \r\n",addr->IC_SS_SCL_HCNT);
	ks_shell_printf(ctx->uart_id,"IC_SS_SCL_LCNT : %x \r\n",addr->IC_SS_SCL_LCNT);

	ks_shell_printf(ctx->uart_id,"IC_FS_SCL_HCNT : %x \r\n",addr->IC_FS_SCL_HCNT);
	ks_shell_printf(ctx->uart_id,"IC_FS_SCL_LCNT : %x \r\n",addr->IC_FS_SCL_LCNT);

	ks_shell_printf(ctx->uart_id,"IC_HS_SCL_HCNT : %x \r\n",addr->IC_HS_SCL_HCNT);
	ks_shell_printf(ctx->uart_id,"IC_HS_SCL_LCNT : %x \r\n",addr->IC_HS_SCL_HCNT);

    return 0;
}

static int i2c_ctx_cmd(cmd_proc_t* ctx, int argc, char **argv)
{
    int iret;
    uint32_t uart_id = 0;
    _iic_priv_t *iic_priv = &iic_instance[0];

    ks_shell_printf(ctx->uart_id,"\r\n");
	ks_shell_printf(ctx->uart_id,"mode : %x \r\n",iic_priv->mode);
	ks_shell_printf(ctx->uart_id,"speed : %x \r\n",iic_priv->speed);
	ks_shell_printf(ctx->uart_id,"addr_mode : %x \r\n",iic_priv->addr_mode);
	ks_shell_printf(ctx->uart_id,"slave_addr : %x \r\n",iic_priv->slave_addr);
	ks_shell_printf(ctx->uart_id,"\r\n");
	ks_shell_printf(ctx->uart_id,"fifo_tx_count : %d \r\n",iic_priv->fifo_tx_count);
	ks_shell_printf(ctx->uart_id,"fifo_rx_count : %d \r\n",iic_priv->fifo_rx_count);
	ks_shell_printf(ctx->uart_id,"dma_tx_count : %d\r\n",iic_priv->dma_tx_count);
	ks_shell_printf(ctx->uart_id,"dma_rx_count : %d\r\n",iic_priv->dma_rx_count);
	ks_shell_printf(ctx->uart_id,"dma_failure : %d\r\n",iic_priv->dma_failure);

    return 0;
}

static cmd_proc_t i2c_driver_cmds[] = {
	{.cmd = "i2c_ctx", .fn = i2c_ctx_cmd,  .help = "i2c_ctx_cmd "},
	{.cmd = "i2c_reg", .fn = i2c_reg_cmd,  .help = "i2c_reg_cmd "},
};

static inline void _iic_disable(_iic_reg_t *addr)
{
    /* First clear ACTIVITY, then Disable IIC */
    addr->IC_CLR_ACTIVITY;
    addr->IC_ENABLE = DW_IIC_DISABLE;
}

static inline void _iic_enable(_iic_reg_t *addr)
{
    addr->IC_ENABLE = DW_IIC_ENABLE;
}

static void _iic_do_align_buffer(_iic_priv_dma_align_control_t* p_align_ctrl, uint8_t* ptr, uint32_t len)
{
    const uint32_t start_rxbuf = (uint32_t)ptr;
    const uint32_t end_rxbuf = (uint32_t)&ptr[len];

    p_align_ctrl->buf_orig         = ptr;
    p_align_ctrl->total_num_orig   = len;
    p_align_ctrl->align_start_addr = (uint8_t*)(start_rxbuf & (~(CACHE_LINE_ALIGN_LEN - 1)));   /*for now*/
    p_align_ctrl->align_end_addr   = (uint8_t*)(end_rxbuf   & (~(CACHE_LINE_ALIGN_LEN - 1)));

    if (p_align_ctrl->align_start_addr != p_align_ctrl->align_end_addr){
        //check head
        if(start_rxbuf % CACHE_LINE_ALIGN_LEN != 0){
            //buffer head not align, and the data cross one or more block
            p_align_ctrl->align_start_addr += CACHE_LINE_ALIGN_LEN;
            p_align_ctrl->align_offset_head = (uint32_t)(p_align_ctrl->align_start_addr)  - start_rxbuf;
        }else{
            //buffer head aligns, and the data cross one or more block
            p_align_ctrl->align_offset_head = 0;
        }

        //check tail
        if(end_rxbuf % CACHE_LINE_ALIGN_LEN != 0){
            p_align_ctrl->align_offset_tail = end_rxbuf - (uint32_t)(p_align_ctrl->align_end_addr);
        }else{
            p_align_ctrl->align_offset_tail = 0;
        }
    }
    else{
        //check head
        if(start_rxbuf % CACHE_LINE_ALIGN_LEN != 0){
            //buffer head not align, and all data lie in the same block
            //length MUST be less than `CACHE_LINE_ALIGN_LEN, just copy all data from head tmp-align buffer
            p_align_ctrl->align_offset_head = len;
        }else{
            //buffer head aligns, and all data lie in the same block
            p_align_ctrl->align_offset_head = 0;
        }

        //there is no need to process tail
        p_align_ctrl->align_offset_tail = 0;
    }
}

static void _iic_set_hcnt(_iic_reg_t *addr, DW_IIC_SPEED speed)
{
    switch (speed) {
    case DW_IIC_STANDARDSPEED: addr->IC_SS_SCL_HCNT = ( ks_os_get_apb_clock() / 1000000) * 4       ; break;
    case DW_IIC_FASTSPEED    : addr->IC_FS_SCL_HCNT = ( ks_os_get_apb_clock() / 1000000) * 1       ; break;
    case DW_IIC_FASTPLUSSPEED: addr->IC_FS_SCL_HCNT = ((ks_os_get_apb_clock() / 1000000) / 2.5) * 1; break;
    case DW_IIC_HIGHSPEED    : addr->IC_HS_SCL_HCNT = ((ks_os_get_apb_clock() / 1000000) / 10 ) * 1; break;
    default: break;
    }
}

static void _iic_set_lcnt(_iic_reg_t *addr, DW_IIC_SPEED speed)
{
    switch (speed) {
    case DW_IIC_STANDARDSPEED: addr->IC_SS_SCL_LCNT = ( ks_os_get_apb_clock() / 1000000) * 5       ; break;
    case DW_IIC_FASTSPEED    : addr->IC_FS_SCL_LCNT = ( ks_os_get_apb_clock() / 1000000) * 1       ; break;
    case DW_IIC_FASTPLUSSPEED: addr->IC_FS_SCL_LCNT = ((ks_os_get_apb_clock() / 1000000) / 2.5) * 1; break;
    case DW_IIC_HIGHSPEED    : addr->IC_HS_SCL_LCNT = ((ks_os_get_apb_clock() / 1000000) / 10 ) * 1; break;
    default: break;
    }
}

static inline void _iic_set_transfer_speed(_iic_reg_t *addr, DW_IIC_SPEED speed)
{
    uint16_t temp = addr->IC_CON;

    temp &= ~((1 << 1) + (1 << 2));
	if(speed == DW_IIC_STANDARDSPEED){
		temp |= 1 << 1;
	}else if(speed == DW_IIC_FASTSPEED || speed == DW_IIC_FASTPLUSSPEED){
		temp |= 2 << 1;
	}else {
		temp |= 3 << 1;
	}

    addr->IC_CON = temp;
}

static inline void _iic_set_target_address(_iic_reg_t *addr, uint32_t address)
{
    /* disable i2c */
    _iic_disable(addr);

    uint32_t temp = addr->IC_TAR;

    temp &= 0xfc00;
    temp |= address;

    addr->IC_TAR = temp;
}

static inline void _iic_set_slave_address(_iic_reg_t *addr, uint32_t address)
{
    /* write to IC_SAR register to set the slave address */
    addr->IC_SAR = address & 0x3ff;
}

static inline void _iic_set_addr_mode(_iic_priv_t *iic_priv, iic_address_mode_e addr_mode)
{
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    if (iic_priv->mode == IIC_MODE_MASTER) {
        uint16_t temp = addr->IC_TAR;
        temp &= 0xefff;
        temp |= addr_mode << 12;
        addr->IC_TAR = temp;
    } else {
        if (addr_mode == IIC_ADDRESS_10BIT) {
            addr->IC_CON |= DW_IIC_CON_10BITADDR_SLAVE;
        } else {
            addr->IC_CON &= ~DW_IIC_CON_10BITADDR_SLAVE;
        }
    }
}

static uint32_t _iic_read_clear_intrbits(_iic_reg_t *addr)
{
    uint32_t  stat = 0;

    stat = addr->IC_INTR_STAT;

    if (stat & DW_IIC_INTR_RX_UNDER) {
        addr->IC_CLR_RX_UNDER;
    }

    if (stat & DW_IIC_INTR_RX_OVER) {
        addr->IC_CLR_RX_OVER;
    }

    if (stat & DW_IIC_INTR_TX_OVER) {
        addr->IC_CLR_TX_OVER;
    }

    if (stat & DW_IIC_INTR_RD_REQ) {
        addr->IC_CLR_RD_REQ;
    }

    if (stat & DW_IIC_INTR_TX_ABRT) {
        addr->IC_TX_ABRT_SOURCE;
    }

    if (stat & DW_IIC_INTR_RX_DONE) {
        addr->IC_CLR_RX_DONE;
    }

    if (stat & DW_IIC_INTR_ACTIVITY) {
        addr->IC_CLR_ACTIVITY;
    }

    if (stat & DW_IIC_INTR_STOP_DET) {
        addr->IC_CLR_STOP_DET;
    }

    if (stat & DW_IIC_INTR_START_DET) {
        addr->IC_CLR_START_DET;
    }

    if (stat & DW_IIC_INTR_GEN_CALL) {
        addr->IC_CLR_GEN_CALL;
    }

    return stat;
}

/**
  \brief       interrupt service function for transmit FIFO empty ,start ,stop interrupt.
  \param[in]   iic_priv pointer to iic private.
*/
static void _iic_master_intr_tx(int32_t idx, _iic_priv_t *iic_priv, uint32_t intr_stat)
{
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    uint32_t remain_count;
    uint8_t fifo_free_size;
    uint32_t i;
	
	/*master send data to slave */
	if (intr_stat & DW_IIC_INTR_START_DET) {
		//detect start, nothing to do with data, using `TX_EMPTY to require data
	}
	
    if (intr_stat & DW_IIC_INTR_TX_EMPTY) {
        if (iic_priv->dev->tx_dma_enable){
            //finish notify for DMA trans
            addr->IC_INTR_MASK = 0;
            _iic_disable(addr);

            iic_priv->status  = IIC_STATE_SEND_DONE;
            iic_priv->fifo_tx_count ++;

            if (iic_priv->cb_event) {
                iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
            }
            return;
        }

        remain_count = iic_priv->tx_total_num - iic_priv->tx_cnt;

        if (remain_count > 0) {
            //has more data need to push to FIFO
            fifo_free_size = DW_IIC_FIFO_MAX_LV - addr->IC_TXFLR;
            iic_priv->send_num = DRIVER_MIN(fifo_free_size, remain_count);

            //push data to FIFO, as many as possible
            for (i = 0; i<iic_priv->send_num; i++) {
                addr->IC_DATA_CMD = *iic_priv->tx_buf++;
                iic_priv->tx_cnt++;
            }

            //update next watermarker
            if (iic_priv->tx_cnt >= iic_priv->tx_total_num) {
                //no pending data to send, just wait for all FIFO data sent
                addr->IC_TX_TL = 0;
            }
        }
        else{
            //all data has been sent, stop monitoring empty event, using `STOP_DET to mark finishing tx
            addr->IC_INTR_MASK &= ~DW_IIC_INTR_TX_EMPTY;
            while (addr->IC_STATUS & DW_IIC_STATUS_ACTIVITY);

            _iic_disable(addr);

            iic_priv->status  = IIC_STATE_SEND_DONE;
            iic_priv->fifo_tx_count ++;

            if (iic_priv->cb_event) {
                iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
            }
        }
    }

    if (intr_stat & DW_IIC_INTR_TX_OVER) {
        _iic_disable(addr);
        iic_priv->status = IIC_STATE_ERROR;

        if (iic_priv->cb_event) {
            iic_priv->cb_event(idx, IIC_EVENT_BUS_ERROR,iic_priv->arg);
        }
    }

    if (intr_stat & DW_IIC_INTR_STOP_DET) {
        //all data has been sent, stop condition detected
		if(addr->IC_TXFLR != 0){
			return ;
        }

        //for safe, clear again (in case of bus error etc.)
        addr->IC_INTR_MASK &= ~DW_IIC_INTR_TX_EMPTY;
        _iic_disable(addr);

        iic_priv->status  = IIC_STATE_SEND_DONE;
        iic_priv->fifo_tx_count ++;

        if (iic_priv->cb_event) {
            iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
        }
    }
}

/**
  \brief       interrupt service function for transmit FIFO empty ,start ,stop interrupt.
  \param[in]   iic_priv pointer to iic private.
*/
static void _iic_slave_intr_tx(int32_t idx, _iic_priv_t *iic_priv, uint32_t intr_stat)
{
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    uint32_t remain_count;
    uint8_t fifo_free_size;
    uint32_t i;

    if (intr_stat & DW_IIC_INTR_RD_REQ) {
        //wait for read request
        if (iic_priv->dev->tx_dma_enable) {
            ks_os_irq_disable(iic_priv->irq);
            ks_driver_dma_start(iic_priv->dma_handle, iic_priv->dma_tx_ch
                                , ((iic_priv->tx_dma_align_ctrl.align_offset_head) ? iic_priv->tx_dma_align_ctrl.trans_head_addr : iic_priv->tx_dma_align_ctrl.buf_orig)
                                , (uint8_t *) & (addr->IC_DATA_CMD)
                                , iic_priv->send_num
                                );
            iic_priv->dma_tx_count ++;
        }
        else{
            addr->IC_TX_TL = IIC_GET_FAIR_TX_TL(iic_priv->tx_total_num);
            addr->IC_INTR_MASK |= DW_IIC_INTR_TX_EMPTY;
        }
    }
	
    if (intr_stat & DW_IIC_INTR_TX_EMPTY) {
        remain_count = iic_priv->tx_total_num - iic_priv->tx_cnt;

        if (remain_count){
            //get free size in FIFO
            fifo_free_size = DW_IIC_FIFO_MAX_LV - addr->IC_TXFLR;
            iic_priv->send_num = DRIVER_MIN(remain_count, fifo_free_size);

            //push data to FIFO, as many as possible
            for (i=0; i<iic_priv->send_num; i++){
                addr->IC_DATA_CMD = (uint16_t)*iic_priv->tx_buf++;
                iic_priv->tx_cnt++;
            }

            //update next watermarker, default is half full
            //  (after first block, `remain_count > 0, so there must be pending data
            //   at the beginning of this block)
            if (iic_priv->tx_cnt >= iic_priv->tx_total_num){
                //no pending data to send, wait for all FIFO data sent
                addr->IC_TX_TL = 0;
            }
        }
        else{
            //all data has been sent, stop monitoring TX_EMPTY event,
            //  use STOP_DET to identify finishing tx
            addr->IC_INTR_MASK &= ~DW_IIC_INTR_TX_EMPTY;
            volatile uint32_t timeout = 0x1000;
            while ((addr->IC_STATUS & (1<<6)) != 0 && (--timeout));

            //clear again, for safe
            _iic_disable(addr);
            iic_priv->status  = IIC_STATE_SEND_DONE;

            if (timeout){
                iic_priv->fifo_tx_count ++;

                if (iic_priv->cb_event) {
                    iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
                }
            }
            else{
                if (iic_priv->cb_event) {
                    iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_INCOMPLETE,iic_priv->arg);
                }
            }
        }
    }

    if (intr_stat & DW_IIC_INTR_TX_OVER) {
        iic_priv->status = IIC_STATE_ERROR;
        _iic_disable(addr);

        if (iic_priv->cb_event) {
            iic_priv->cb_event(idx, IIC_EVENT_BUS_ERROR,iic_priv->arg);
        }
    }

    if (intr_stat & DW_IIC_INTR_STOP_DET) {
		if( iic_priv->mode  == IIC_MODE_SLAVE){
			if (iic_priv->tx_cnt != iic_priv->tx_total_num) {
				return ;
			}
		}

        return;

        //clear again, for safe
        addr->IC_INTR_MASK &= ~DW_IIC_INTR_TX_EMPTY;
        _iic_disable(addr);

        iic_priv->status  = IIC_STATE_SEND_DONE;
		iic_priv->fifo_tx_count ++;

        if (iic_priv->cb_event) {
            iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
        }
    }
}

/**
  \brief       interrupt service function for receive FIFO full , stop interrupt .
  \param[in]   iic_priv pointer to iic private.
*/
static void _iic_master_intr_rx(int32_t idx, _iic_priv_t *iic_priv, uint32_t intr_stat)
{
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    uint32_t count;
    uint32_t iic_rxflr  = addr->IC_RXFLR;
    uint32_t remain_count;

    if (intr_stat & DW_IIC_INTR_RX_FULL) {
        remain_count = iic_priv->rx_total_num - iic_priv->rx_cnt;

        if (remain_count){
            //push pending dummy byte, as many as possible, if any
            if (iic_priv->cmd_read_num > 0){
                uint16_t dummy_byte = 1 << 8;
                count = DRIVER_MIN(iic_priv->cmd_read_num, DW_IIC_FIFO_MAX_LV - addr->IC_TXFLR);
                while (count--){
                    addr->IC_DATA_CMD = dummy_byte;
                    iic_priv->cmd_read_num--;
                }
            }

            //read all valid data in FIFO
            iic_priv->recv_num = DRIVER_MIN(remain_count, iic_rxflr);

            for (count=0; count<iic_priv->recv_num; count++) {
                *iic_priv->rx_buf++ = ((addr->IC_DATA_CMD) & 0xff);
                iic_priv->rx_cnt++;
            }

            //refresh next watermarker
            if(iic_priv->rx_total_num > iic_priv->rx_cnt){
                //has pending data to recv, update next watermarker
                addr->IC_RX_TL = IIC_GET_FAIR_RX_TL(iic_priv->rx_total_num - iic_priv->rx_cnt);
            }else{
                //all data has been recv, stop monitoring RX_FULL, use STOP_DET to identify finishing rx

                //wait for data finished
                while (addr->IC_STATUS & DW_IIC_STATUS_ACTIVITY);

                addr->IC_INTR_MASK = 0;
                _iic_disable(addr);

                iic_priv->fifo_tx_count ++;
                iic_priv->status  = IIC_STATE_RECV_DONE;

                if (iic_priv->cb_event) {
                    iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
                }
            }
        }
    }

    if (intr_stat & DW_IIC_INTR_RX_OVER) {
        addr->IC_DATA_CMD = 0;
        _iic_disable(addr);
        iic_priv->status = IIC_STATE_ERROR;
		
        if (iic_priv->cb_event) {
            iic_priv->cb_event(idx, IIC_EVENT_BUS_ERROR,iic_priv->arg);
        }
    }

    if (intr_stat & DW_IIC_INTR_STOP_DET) {
        if (iic_priv->rx_total_num == 0) {
            addr->IC_INTR_MASK = 0;
            _iic_disable(addr);

			iic_priv->fifo_tx_count ++;
            iic_priv->status  = IIC_STATE_RECV_DONE;

            if (iic_priv->cb_event) {
                iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
            }
        }
    }

}


/**
  \brief       interrupt service function for receive FIFO full interrupt .
  \param[in]   iic_priv pointer to iic private.
*/
static void _iic_slave_intr_rx(int32_t idx, _iic_priv_t *iic_priv, uint32_t intr_stat)
{
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    uint32_t count;
    uint32_t iic_rxflr  = addr->IC_RXFLR;
    uint32_t remain_count;

    if (intr_stat & DW_IIC_INTR_START_DET) {
        //nothing to do, just wait for RX FIFO watermarker
	}

    if (intr_stat & DW_IIC_INTR_RX_FULL) {
        remain_count = iic_priv->rx_total_num - iic_priv->rx_cnt;

        if (remain_count){
            iic_priv->recv_num = DRIVER_MIN(remain_count, iic_rxflr);

            //read all valid data in FIFO
            for (count = 0; count<iic_priv->recv_num; count++) {
                *iic_priv->rx_buf++ = ((addr->IC_DATA_CMD) & 0xff);
                iic_priv->rx_cnt++;
            }

            //refresh next watermarker
            if(iic_priv->rx_cnt < iic_priv->rx_total_num){
                //has pending data to recv, update next watermarker
                addr->IC_RX_TL = IIC_GET_FAIR_RX_TL(iic_priv->rx_total_num - iic_priv->rx_cnt);
            }else{
                //all data has been recv, stop monitoring RX_FULL, use STOP_DET to identify finishing rx
                while (addr->IC_STATUS & DW_IIC_STATUS_SLV_ACTIVITY);

                addr->IC_INTR_MASK = 0;
                _iic_disable(addr);

                iic_priv->fifo_rx_count ++;
                iic_priv->status  = IIC_STATE_RECV_DONE;

                if (iic_priv->cb_event) {
                    iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
                }
            }
        }
    }

    if (intr_stat & DW_IIC_INTR_RX_OVER) {
        addr->IC_DATA_CMD = 0;
        _iic_disable(addr);

        iic_priv->status = IIC_STATE_ERROR;

        if (iic_priv->cb_event) {
            iic_priv->cb_event(idx, IIC_EVENT_BUS_ERROR,iic_priv->arg);
        }
    }

    if (intr_stat & DW_IIC_INTR_STOP_DET) {
        if (iic_priv->rx_cnt >= iic_priv->rx_total_num) {
            addr->IC_INTR_MASK = 0;
            _iic_disable(addr);

            iic_priv->fifo_rx_count ++;
            iic_priv->status  = IIC_STATE_RECV_DONE;

            if (iic_priv->cb_event) {
                iic_priv->cb_event(idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
            }
        }
    }
}

void _iic_irqhandler(void* arg)
{
	int idx = (int) arg;
    _iic_priv_t *iic_priv = &iic_instance[idx];
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    uint32_t  intr_stat = _iic_read_clear_intrbits(addr);

    if (intr_stat & DW_IIC_INTR_TX_ABRT) {
        /* If arbitration fault, it indicates either a slave device not
        * responding as expected, or other master which is not supported
        * by this SW.
        */
        uint32_t tx_abrt_source = addr->IC_TX_ABRT_SOURCE;
        addr->IC_CLR_TX_ABRT;

        if (DW_IIC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_MASK == (tx_abrt_source & DW_IIC_TX_ABRT_SOURCE_FLGA_MAS)){
            if (iic_priv->tx_addr_retry_cnt < IIC_TX_ADDR_RETRY_MAX){
                //retry send address
                iic_priv->tx_addr_retry_cnt++;
                kprintf(">>_i2c_event_cb retry TX_ADDR_ABRT: %d \r\n", iic_priv->tx_addr_retry_cnt);

                if (iic_priv->dev->tx_dma_enable){
                    //reset to origin state, from DMA process
                    _iic_master_send((iic_handle_t)iic_priv, (addr->IC_TAR & 0x3FF), iic_priv->tx_dma_align_ctrl.buf_orig, iic_priv->tx_dma_align_ctrl.total_num_orig);
                }
                else{
                    //reset to origin state, from FIFO process
                    _iic_master_send((iic_handle_t)iic_priv, (addr->IC_TAR & 0x3FF), iic_priv->tx_buf-iic_priv->tx_cnt, iic_priv->tx_total_num);
                }
                return;
            }
        }

        iic_priv->status    = IIC_STATE_DONE;
        _iic_disable(addr);
        addr->IC_INTR_MASK = 0;

        kprintf(">>_i2c_event_cb event IIC_EVENT_BUS_ERROR: %04X \r\n", tx_abrt_source);

        if (iic_priv->cb_event) {
            iic_priv->cb_event(idx, IIC_EVENT_BUS_ERROR,iic_priv->arg);
            return;
        }
    }

    switch (iic_priv->status) {
        /* master send data to slave  or  slave send to master*/
        case IIC_STATE_DATASEND: {
			if(iic_priv->mode == IIC_MODE_MASTER){
				_iic_master_intr_tx(idx, iic_priv, intr_stat);
			}else{

				_iic_slave_intr_tx(idx, iic_priv, intr_stat);
			}
            break;
        }

        /* master wait for data from slave or  slave  wait for data from master */
        case IIC_STATE_WFDATA: {
			if(iic_priv->mode == IIC_MODE_MASTER){
				_iic_master_intr_rx(idx, iic_priv, intr_stat);
			}else{
				_iic_slave_intr_rx(idx, iic_priv, intr_stat);
			}
            break;
        }

        /* unexpected state,SW fault */
        default: {
            addr->IC_INTR_MASK = 0;
            _iic_disable(addr);

            if (iic_priv->cb_event) {
                iic_priv->cb_event(idx, IIC_EVENT_ARBITRATION_LOST,iic_priv->arg);
            }
        }
    }
}

int _iic_get_next_dma_len(int residue){
	int dma_len;
	
	if (residue > IIC_DMA_BLOCK_SIZE) {
		dma_len = IIC_DMA_BLOCK_SIZE;
	}else if(residue > CACHE_LINE_ALIGN_LEN){
		dma_len = residue &(~(CACHE_LINE_ALIGN_LEN - 1));
	}else {
		dma_len = residue;
	}

	return dma_len;
}

void _iic_dma_event_cb(dma_channel_context* ch_ctx, int32_t ch, dma_event_e event)
{
    _iic_priv_t *iic_priv = ( _iic_priv_t *)ch_ctx->arg;
    int i = 0;
    uint32_t timeout = 0;
	int next_dmalen;

	if (((iic_priv->dma_tx_ch == ch) && (iic_priv->dma_use == 1)) || ((iic_priv->dma_rx_ch == ch) && (iic_priv->dma_use == 1))) {
		
	}else{
        if (iic_priv->cb_event) {
            iic_priv->cb_event(iic_priv->idx, IIC_EVENT_BUS_ERROR,iic_priv->arg);
        }
	}

    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    if (event == DMA_EVENT_TRANSFER_DONE) {
		/* transfer date done with DMA  */
        if (iic_priv->tans_mode == IIC_TXRX) {
            if (ch == iic_priv->dma_tx_ch){
                //pushing dummy data, using dma
                iic_priv->read_total_num -= iic_priv->read_num;
                iic_priv->read_num = DRIVER_MIN(iic_priv->read_total_num, IIC_DMA_BLOCK_SIZE);
                ks_driver_dma_start(iic_priv->dma_handle, iic_priv->dma_tx_ch, iic_priv->tx_dma_align_ctrl.trans_head_addr, ((uint8_t *)&(addr->IC_DATA_CMD)), iic_priv->read_num);
            }
            if (ch == iic_priv->dma_rx_ch){
                //read channel dma cb
                 iic_priv->rx_total_num -= iic_priv->recv_num;

                 if (iic_priv->rx_total_num) {
                     iic_priv->rx_buf += iic_priv->recv_num;
                     iic_priv->recv_num  = _iic_get_next_dma_len(iic_priv->rx_total_num);

                     if(iic_priv->recv_num < CACHE_LINE_ALIGN_LEN){
                         ks_driver_dma_start(iic_priv->dma_handle,iic_priv->dma_rx_ch, (uint8_t *) & (addr->IC_DATA_CMD), iic_priv->rx_dma_align_ctrl.trans_tail_addr, iic_priv->recv_num);
                     }else{
                         ks_driver_dma_start(iic_priv->dma_handle,iic_priv->dma_rx_ch, (uint8_t *) & (addr->IC_DATA_CMD), iic_priv->rx_buf, iic_priv->recv_num);
                     }

                     return;
                 }

                 ks_os_irq_enable(iic_priv->irq);

                 while (!(addr->IC_RAW_INTR_STAT & DW_IIC_INTR_STOP_DET)) {
                    timeout ++;

                    if (timeout > DW_IIC_TIMEOUT_DEF_VAL) {
                        if (iic_priv->cb_event) {
                            iic_priv->cb_event(iic_priv->idx, IIC_EVENT_TRANSFER_INCOMPLETE,iic_priv->arg);
                        }
                        break;
                    }
                 }

                 ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_tx_ch);
                 ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_rx_ch);
                 ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
                 ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);

                 iic_priv->dma_use = 0;
                 iic_priv->tx_total_num = 0;
                 iic_priv->tx_buf = NULL;
                 iic_priv->dma_tx_ch = -1;
                 addr->IC_DMA_CR &= ~DW_IIC_DMACR_TDMAE_Msk;

                 iic_priv->dma_use = 0;
                 iic_priv->rx_total_num = 0;
                 iic_priv->dma_rx_ch = -1;
                 iic_priv->rx_buf = NULL;
                 addr->IC_DMA_CR &= ~DW_IIC_DMACR_RDMAE_Msk;

                _iic_disable(addr);

                //if using tmp-align head buffer, invalidate dcache and copy
                if (iic_priv->rx_dma_align_ctrl.align_offset_head){
                    memcpy(iic_priv->rx_dma_align_ctrl.buf_orig, iic_priv->rx_dma_align_ctrl.trans_head_addr, iic_priv->rx_dma_align_ctrl.align_offset_head);
                }

                //if using tmp-align tail buffer, invalidate dcache and copy
                if (iic_priv->rx_dma_align_ctrl.align_offset_tail){
                    memcpy(iic_priv->rx_dma_align_ctrl.align_end_addr, iic_priv->rx_dma_align_ctrl.trans_tail_addr, iic_priv->rx_dma_align_ctrl.align_offset_tail);
                }

                if (iic_priv->cb_event) {
                    iic_priv->cb_event(iic_priv->idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
                }
            }
        }else if(iic_priv->tans_mode == IIC_RX){
			/* recieve date done with DMA  */
			if (iic_priv->dma_rx_ch == ch) {
                iic_priv->rx_total_num -= iic_priv->recv_num;

                if (iic_priv->rx_total_num) {
                    iic_priv->rx_buf += iic_priv->recv_num;
                    iic_priv->recv_num  = _iic_get_next_dma_len(iic_priv->rx_total_num);

                    if(iic_priv->recv_num < CACHE_LINE_ALIGN_LEN){
                        ks_driver_dma_start(iic_priv->dma_handle, iic_priv->dma_rx_ch, (uint8_t *) & (addr->IC_DATA_CMD), iic_priv->rx_dma_align_ctrl.trans_tail_addr, iic_priv->recv_num);
                    }else{
                        ks_driver_dma_start(iic_priv->dma_handle, iic_priv->dma_rx_ch, (uint8_t *) & (addr->IC_DATA_CMD), iic_priv->rx_buf, iic_priv->recv_num);
                    }
                    return;
                }

                ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_rx_ch);
                ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
                iic_priv->dma_use = 0;
                iic_priv->rx_total_num = 0;
                iic_priv->dma_rx_ch = -1;
                iic_priv->rx_buf = NULL;
                addr->IC_DMA_CR &= ~DW_IIC_DMACR_RDMAE_Msk;

                ks_os_irq_enable(iic_priv->irq);
                _iic_disable(addr);

                //if using tmp-align head buffer, invalidate dcache and copy
                if (iic_priv->rx_dma_align_ctrl.align_offset_head){
                    ks_cpu_dcache_invalidate(iic_priv->rx_dma_align_ctrl.trans_head_addr, CACHE_LINE_ALIGN_LEN);
                    memcpy(iic_priv->rx_dma_align_ctrl.buf_orig, iic_priv->rx_dma_align_ctrl.trans_head_addr, iic_priv->rx_dma_align_ctrl.align_offset_head);
                }

                //if using tmp-align tail buffer, invalidate dcache and copy
                if (iic_priv->rx_dma_align_ctrl.align_offset_tail){
                    ks_cpu_dcache_invalidate(iic_priv->rx_dma_align_ctrl.trans_tail_addr, CACHE_LINE_ALIGN_LEN);
                    memcpy(iic_priv->rx_dma_align_ctrl.align_end_addr, iic_priv->rx_dma_align_ctrl.trans_tail_addr, iic_priv->rx_dma_align_ctrl.align_offset_tail);
                }

                if (iic_priv->cb_event) {
                    iic_priv->cb_event(iic_priv->idx, IIC_EVENT_TRANSFER_DONE,iic_priv->arg);
                }
			}
		}else if(iic_priv->tans_mode == IIC_TX){
			/* sending date done with DMA  */
			if (iic_priv->dma_tx_ch == ch) {
				iic_priv->tx_total_num -= iic_priv->send_num;

                if (iic_priv->tx_total_num) {
                    iic_priv->tx_buf += iic_priv->send_num;

                    if(iic_priv->tx_buf == iic_priv->tx_dma_align_ctrl.align_end_addr){
                        iic_priv->send_num  = iic_priv->tx_dma_align_ctrl.align_offset_tail;
                        ks_driver_dma_start(iic_priv->dma_handle,iic_priv->dma_tx_ch, iic_priv->tx_dma_align_ctrl.trans_tail_addr, (uint8_t *) & (addr->IC_DATA_CMD), iic_priv->send_num);
                    }else{
                        iic_priv->send_num  = _iic_get_next_dma_len(iic_priv->tx_total_num);
                        ks_driver_dma_start(iic_priv->dma_handle,iic_priv->dma_tx_ch, iic_priv->tx_buf, (uint8_t *) & (addr->IC_DATA_CMD), iic_priv->send_num);
                    }
					return;
				}
			
				ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_tx_ch);
				ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
				iic_priv->dma_use = 0;
				iic_priv->tx_total_num = 0;
				iic_priv->tx_buf = NULL;
				iic_priv->dma_tx_ch = -1;

                addr->IC_TX_TL = 0;
                addr->IC_INTR_MASK = DW_IIC_INTR_TX_EMPTY | DW_IIC_INTR_TX_ABRT;
			
				ks_os_irq_enable(iic_priv->irq);
			}
		}
    } else if (event == DMA_EVENT_TRANSFER_ERROR) {				
		if (iic_priv->dma_tx_ch != -1) {
	        ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_tx_ch);
	        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
		}

		if (iic_priv->dma_rx_ch != -1) {
	        ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_rx_ch);
	        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
		}
		
        if (iic_priv->cb_event) {
            iic_priv->cb_event(iic_priv->idx, IIC_EVENT_BUS_ERROR,iic_priv->arg);
        }
    }

    return;
}

/**
  \brief mater or slave  sending data to IIC transmitter with DMA,(received data is ignored).
*/
static int32_t _iic_master_send_dma(_iic_priv_t *iic_priv)
{
    int32_t ret = 0;
    uint8_t i;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

	iic_priv->tans_mode = IIC_TX;

	iic_priv->dma_tx_ch = ks_driver_dma_alloc_channel(iic_priv->dma_handle,iic_priv->dev->tx_dma_ch);
	if(iic_priv->dma_tx_ch < 0){
		iic_priv->dma_failure ++;
		return ERR_IIC(DRV_ERROR_BUSY);
	}

    dma_config_t config;
    memset(&config, 0, sizeof(dma_config_t));
    config.src_inc  = DMA_ADDR_INC;
    config.dst_inc  = DMA_ADDR_CONSTANT;
    config.src_tw   = DMA_DATAWIDTH_SIZE8;
    config.dst_tw   = DMA_DATAWIDTH_SIZE8;
	config.burst_len =  DMA_BURST_TRANSACTION_SIZE1;
    config.type     = DMA_MEM2PERH;
    config.ch_mode   = DMA_HANDSHAKING_HARDWARE;

    if (iic_priv->idx == 0) {
        config.hs_if    = DMA_IIC_TX;
    } else {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_tx_ch = -1;
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    ret = ks_driver_dma_config_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch, &config, (dma_event_cb) _iic_dma_event_cb, (void*)iic_priv);
    if (ret < 0) {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_tx_ch = -1;
		iic_priv->dma_failure ++;
        return ERR_IIC(DRV_ERROR_BUSY);
    }

	iic_priv->dma_use = 1;
	ks_os_irq_disable(iic_priv->irq);

    _iic_disable(addr);

    addr->IC_DMA_TDLR = DW_IIC_FIFO_MAX_LV/2;
    addr->IC_DMA_CR = DW_IIC_DMACR_TDMAE_Msk;

    //calc align params for non-align address input
    _iic_do_align_buffer(&iic_priv->tx_dma_align_ctrl, iic_priv->tx_buf, iic_priv->tx_total_num);

    //calc the first DMA trans block
    ubase_t dma_buff;
    if (iic_priv->tx_dma_align_ctrl.align_offset_head){
        //need trans in head
        dma_buff = (uint32_t)iic_priv->tx_dma_align_ctrl.trans_head_addr;
        iic_priv->send_num = iic_priv->tx_dma_align_ctrl.align_offset_head;
    }
    else{
        //already align in the first place
        dma_buff = (uint32_t)iic_priv->tx_dma_align_ctrl.buf_orig;
        if (iic_priv->tx_total_num > IIC_DMA_BLOCK_SIZE){
            iic_priv->send_num = IIC_DMA_BLOCK_SIZE;
        }
        else{
            iic_priv->send_num = iic_priv->tx_total_num - iic_priv->tx_dma_align_ctrl.align_offset_tail;
        }
    }

    //clean and invalidate cache
    if (iic_priv->tx_dma_align_ctrl.align_offset_head){
        //clean inner cache, if needed
        memcpy(iic_priv->tx_dma_align_ctrl.trans_head_addr, iic_priv->tx_dma_align_ctrl.buf_orig, iic_priv->tx_dma_align_ctrl.align_offset_head);
        ks_cpu_dcache_clean(iic_priv->tx_dma_align_ctrl.trans_head_addr, CACHE_LINE_ALIGN_LEN);
    }
    //
    if (iic_priv->tx_dma_align_ctrl.align_offset_tail){
        memcpy(iic_priv->tx_dma_align_ctrl.trans_tail_addr, iic_priv->tx_dma_align_ctrl.align_end_addr, iic_priv->tx_dma_align_ctrl.align_offset_tail);
        ks_cpu_dcache_clean(iic_priv->tx_dma_align_ctrl.trans_tail_addr, CACHE_LINE_ALIGN_LEN);
    }
    //clean target cache
    ks_cpu_dcache_clean(iic_priv->tx_dma_align_ctrl.align_start_addr, iic_priv->tx_dma_align_ctrl.align_end_addr - iic_priv->tx_dma_align_ctrl.align_start_addr);

    _iic_enable(addr);

    //start DMA trans
	ks_driver_dma_start(iic_priv->dma_handle, (uint32_t)iic_priv->dma_tx_ch, (void*)dma_buff, (uint8_t *) & (addr->IC_DATA_CMD), iic_priv->send_num);
	iic_priv->dma_tx_count ++;

    return 0;
}


static int32_t _iic_slave_send_dma(_iic_priv_t *iic_priv)
{
    int32_t ret = 0;
    uint8_t i;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

	iic_priv->tans_mode = IIC_TX;

	iic_priv->dma_tx_ch = ks_driver_dma_alloc_channel(iic_priv->dma_handle,iic_priv->dev->tx_dma_ch);
	if(iic_priv->dma_tx_ch < 0){
		iic_priv->dma_failure ++;
		return ERR_IIC(DRV_ERROR_BUSY);
	}

    dma_config_t config;
    memset(&config, 0, sizeof(dma_config_t));
    config.src_inc  = DMA_ADDR_INC;
    config.dst_inc  = DMA_ADDR_CONSTANT;
    config.src_tw   = DMA_DATAWIDTH_SIZE8;
    config.dst_tw   = DMA_DATAWIDTH_SIZE8;
    config.burst_len = DMA_BURST_TRANSACTION_SIZE1;
    config.type     = DMA_MEM2PERH;
    config.ch_mode   = DMA_HANDSHAKING_HARDWARE;

    if (iic_priv->idx == 0) {
        config.hs_if    = DMA_IIC_TX;
    } else {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_tx_ch = -1;
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    ret = ks_driver_dma_config_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch, &config, (dma_event_cb) _iic_dma_event_cb, (void*)iic_priv);
    if (ret < 0) {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_tx_ch = -1;
		iic_priv->dma_failure ++;
        return ERR_IIC(DRV_ERROR_BUSY);
    }

    iic_priv->dma_use = 1;

    _iic_disable(addr);

	addr->IC_DMA_TDLR = DW_IIC_FIFO_MAX_LV/2;
    addr->IC_DMA_CR = DW_IIC_DMACR_TDMAE_Msk;

    _iic_enable(addr);

    //make align
    _iic_do_align_buffer(&iic_priv->tx_dma_align_ctrl, iic_priv->tx_buf, iic_priv->tx_total_num);

    //calc DMA params for first block, only size, cause start address is determined in slave_intr_tx
    if (iic_priv->tx_dma_align_ctrl.align_offset_head){
        //need trans in head
        iic_priv->send_num = iic_priv->tx_dma_align_ctrl.align_offset_head;
    }
    else{
        //already align in the first place
        if (iic_priv->tx_total_num > IIC_DMA_BLOCK_SIZE){
            iic_priv->send_num = IIC_DMA_BLOCK_SIZE;
        }
        else{
            iic_priv->send_num = iic_priv->tx_total_num - iic_priv->tx_dma_align_ctrl.align_offset_tail;
        }
    }

    //clean inner cache, if needed
    if (iic_priv->tx_dma_align_ctrl.align_offset_head){
        memcpy(iic_priv->tx_dma_align_ctrl.trans_head_addr, iic_priv->tx_dma_align_ctrl.buf_orig, iic_priv->tx_dma_align_ctrl.align_offset_head);
        ks_cpu_dcache_clean(iic_priv->tx_dma_align_ctrl.trans_head_addr, CACHE_LINE_ALIGN_LEN);
    }
    if (iic_priv->tx_dma_align_ctrl.align_offset_tail){
        memcpy(iic_priv->tx_dma_align_ctrl.trans_tail_addr, iic_priv->tx_dma_align_ctrl.align_end_addr, iic_priv->tx_dma_align_ctrl.align_offset_tail);
        ks_cpu_dcache_clean(iic_priv->tx_dma_align_ctrl.trans_tail_addr, CACHE_LINE_ALIGN_LEN);
    }
    //invalidate target cache
    ks_cpu_dcache_clean(iic_priv->tx_dma_align_ctrl.align_start_addr, iic_priv->tx_dma_align_ctrl.align_end_addr - iic_priv->tx_dma_align_ctrl.align_start_addr);

    //start DMA first block, after receiving IRQ of `DW_IIC_INTR_RD_REQ

    return 0;
}

/**
  \brief receiving data to IIC receiver with DMA.
*/
static int32_t _iic_slave_receive_dma(_iic_priv_t *iic_priv)
{
    int32_t ret = 0;
    uint8_t i;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

	iic_priv->tans_mode = IIC_RX;
	iic_priv->dma_rx_ch = ks_driver_dma_alloc_channel(iic_priv->dma_handle,iic_priv->dev->rx_dma_ch);
	if(iic_priv->dma_rx_ch < 0){
		iic_priv->dma_failure ++;
		return ERR_IIC(DRV_ERROR_BUSY);
	}

	iic_priv->dma_use = 1;
	ks_os_irq_disable(iic_priv->irq);

    dma_config_t config;
    memset(&config, 0, sizeof(dma_config_t));
    config.src_inc  = DMA_ADDR_CONSTANT;
    config.dst_inc  = DMA_ADDR_INC;
    config.src_tw   = DMA_DATAWIDTH_SIZE8;
    config.dst_tw   = DMA_DATAWIDTH_SIZE8;
	config.burst_len = DMA_BURST_TRANSACTION_SIZE1;
    config.type     = DMA_PERH2MEM;
    config.ch_mode   = DMA_HANDSHAKING_HARDWARE;

    if (iic_priv->idx == 0) {
        config.hs_if    = DMA_IIC_RX;
    } else {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_rx_ch = -1;
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    ret = ks_driver_dma_config_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch, &config, (dma_event_cb)_iic_dma_event_cb, (void*)iic_priv);
    if (ret < 0) {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_rx_ch = -1;
		iic_priv->dma_failure ++;
        return ERR_IIC(DRV_ERROR_BUSY);
    }

    _iic_disable(addr);

	addr->IC_DMA_RDLR = 0 ;
    addr->IC_DMA_CR |= DW_IIC_DMACR_RDMAE_Msk;

    //make align params for non-align address input
    _iic_do_align_buffer(&iic_priv->rx_dma_align_ctrl, iic_priv->rx_buf, iic_priv->rx_total_num);

    //calc the first DMA trans block
    ubase_t dma_buff;
    if (iic_priv->rx_dma_align_ctrl.align_offset_head){
        //need trans in head
        dma_buff = (uint32_t)iic_priv->rx_dma_align_ctrl.trans_head_addr;
        iic_priv->recv_num = iic_priv->rx_dma_align_ctrl.align_offset_head;
    }
    else{
        //already align in the first place
        dma_buff =(uint32_t) iic_priv->rx_dma_align_ctrl.buf_orig;
        iic_priv->recv_num = (iic_priv->rx_total_num > IIC_DMA_BLOCK_SIZE) ? IIC_DMA_BLOCK_SIZE : iic_priv->rx_total_num;
    }

    //inner cache clean and target cache invalidate
    if (iic_priv->rx_dma_align_ctrl.align_offset_head){
        ks_cpu_dcache_clean_and_invalidate(iic_priv->rx_dma_align_ctrl.trans_head_addr, CACHE_LINE_ALIGN_LEN);
    }
    if (iic_priv->rx_dma_align_ctrl.align_offset_tail){
        ks_cpu_dcache_clean_and_invalidate(iic_priv->rx_dma_align_ctrl.trans_tail_addr, CACHE_LINE_ALIGN_LEN);
    }
    //target cache invalidate
    ks_cpu_dcache_invalidate(iic_priv->rx_buf, iic_priv->rx_total_num);

    _iic_enable(addr);

    //start DMA trans
	ks_driver_dma_start(iic_priv->dma_handle, (uint32_t)iic_priv->dma_rx_ch, (uint8_t *) & (addr->IC_DATA_CMD), (void*)dma_buff, iic_priv->recv_num);
	iic_priv->dma_rx_count ++;

	return 0;
}




/**
  \brief reading data to IIC transmitter with DMA.
*/
static int32_t _iic_master_receive_dma(_iic_priv_t *iic_priv)
{
    int32_t ret = 0;
    uint8_t i;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
	
    iic_priv->tans_mode = IIC_TXRX;

	iic_priv->dma_tx_ch = ks_driver_dma_alloc_channel(iic_priv->dma_handle,iic_priv->dev->tx_dma_ch);
	if(iic_priv->dma_tx_ch < 0){
		iic_priv->dma_failure ++;
		return ERR_IIC(DRV_ERROR_BUSY);
	}
	
	iic_priv->dma_rx_ch = ks_driver_dma_alloc_channel(iic_priv->dma_handle,iic_priv->dev->rx_dma_ch);
	if(iic_priv->dma_rx_ch < 0){
		iic_priv->dma_failure ++;
		return ERR_IIC(DRV_ERROR_BUSY);
	}
	
	iic_priv->dma_use = 1;
	ks_os_irq_disable(iic_priv->irq);

    dma_config_t config;
    memset(&config, 0, sizeof(dma_config_t));
    config.src_inc  = DMA_ADDR_CONSTANT;
    config.dst_inc  = DMA_ADDR_CONSTANT;
    config.src_tw   = DMA_DATAWIDTH_SIZE16;
    config.dst_tw   = DMA_DATAWIDTH_SIZE16;
	config.burst_len = DMA_BURST_TRANSACTION_SIZE1;
    config.type     = DMA_MEM2PERH;
    config.ch_mode   = DMA_HANDSHAKING_HARDWARE;

    if (iic_priv->idx == 0) {
        config.hs_if    = DMA_IIC_TX;
    } else {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_tx_ch = -1;
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    ret = ks_driver_dma_config_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch, &config, (dma_event_cb) _iic_dma_event_cb, (void*)iic_priv);
    if (ret < 0) {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_tx_ch = -1;
		iic_priv->dma_failure ++;
        return ERR_IIC(DRV_ERROR_BUSY);
    }
	
    config.src_inc  = DMA_ADDR_CONSTANT ;
    config.dst_inc  = DMA_ADDR_INC;
    config.src_tw   = DMA_DATAWIDTH_SIZE8;
    config.dst_tw   = DMA_DATAWIDTH_SIZE8;
	config.burst_len = DMA_BURST_TRANSACTION_SIZE1;
    config.type     = DMA_PERH2MEM;
    config.ch_mode   = DMA_HANDSHAKING_HARDWARE;

    if (iic_priv->idx == 0) {
        config.hs_if    = DMA_IIC_RX;
    } else {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_rx_ch = -1;
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    ret = ks_driver_dma_config_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch, &config, (dma_event_cb)_iic_dma_event_cb, (void*)iic_priv);
    if (ret < 0) {
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
        ks_os_irq_enable(iic_priv->irq);
        iic_priv->dma_use = 0;
        iic_priv->dma_rx_ch = -1;
		iic_priv->dma_failure ++;
        return ERR_IIC(DRV_ERROR_BUSY);
    }

    _iic_disable(addr);
 
	iic_priv->read_total_num  = iic_priv->rx_total_num;
	
	addr->IC_DMA_RDLR = 0;
	addr->IC_DMA_TDLR = DW_IIC_FIFO_MAX_LV/2;
    addr->IC_DMA_CR = DW_IIC_DMACR_TDMAE_Msk|DW_IIC_DMACR_RDMAE_Msk;

    _iic_enable(addr);

    //make align params for non-align address input
    _iic_do_align_buffer(&iic_priv->rx_dma_align_ctrl, iic_priv->rx_buf, iic_priv->rx_total_num);

    //calc dma param for first block
    ubase_t dma_buff;
    if (iic_priv->rx_dma_align_ctrl.align_offset_head){
        //need tran(uint32_t)s in head
        dma_buff = (uint32_t)iic_priv->rx_dma_align_ctrl.trans_head_addr;
        iic_priv->recv_num = iic_priv->rx_dma_align_ctrl.align_offset_head;
    }
    else{
        //already align in the first place
        dma_buff = (uint32_t)iic_priv->rx_dma_align_ctrl.buf_orig;
        if (iic_priv->rx_total_num > IIC_DMA_BLOCK_SIZE){
            iic_priv->recv_num = IIC_DMA_BLOCK_SIZE;
        }
        else{
            iic_priv->recv_num = iic_priv->rx_total_num - iic_priv->rx_dma_align_ctrl.align_offset_tail;
        }
    }

    //clean inner tx head buffer cache (as dummy data to push reading)
    ((uint16_t*)iic_priv->tx_dma_align_ctrl.trans_head_addr)[0] = 1 << 8;
    ks_cpu_dcache_clean(iic_priv->tx_dma_align_ctrl.trans_head_addr, sizeof(uint16_t));

    //inner cache clean
    if (iic_priv->rx_dma_align_ctrl.align_offset_head){
        ks_cpu_dcache_clean_and_invalidate(iic_priv->rx_dma_align_ctrl.trans_head_addr, CACHE_LINE_ALIGN_LEN);
    }
    if (iic_priv->rx_dma_align_ctrl.align_offset_tail){
        ks_cpu_dcache_clean_and_invalidate(iic_priv->rx_dma_align_ctrl.trans_tail_addr, CACHE_LINE_ALIGN_LEN);
    }
    //invalidate target buffer cache
    ks_cpu_dcache_invalidate(iic_priv->rx_dma_align_ctrl.align_start_addr, iic_priv->rx_dma_align_ctrl.align_end_addr - iic_priv->rx_dma_align_ctrl.align_start_addr);

    //start recv DMA first
    ks_driver_dma_start(iic_priv->dma_handle,  (uint32_t)iic_priv->dma_rx_ch, (uint8_t *) & (addr->IC_DATA_CMD), (void*)dma_buff, iic_priv->recv_num);

    //start send DMA second, dummy data, to push RX, const src address and const dst address,
    //  so just set to the entire recv length, and don't care if finish or not
    iic_priv->read_num = DRIVER_MIN(iic_priv->read_total_num, IIC_DMA_BLOCK_SIZE);
    iic_priv->read_total_num = iic_priv->rx_total_num;

    ks_driver_dma_start(iic_priv->dma_handle, iic_priv->dma_tx_ch, iic_priv->tx_dma_align_ctrl.trans_head_addr, ((uint8_t *)&(addr->IC_DATA_CMD)), iic_priv->read_num);
	
	iic_priv->dma_tx_count ++;
	iic_priv->dma_rx_count ++;

    return 0;
}




/**
  \brief       Initialize IIC Interface specified by pins. \n
               1. Initializes the resources needed for the IIC interface 2.registers event callback function
  \param[in]   idx iic index
  \param[in]   cb_event  Pointer to \ref iic_event_cb_t
  \return      0 for success, negative for error code
*/
void _iic_initialize(iic_handle_t handle,i2c_dev_t *i2c)
{
    uint32_t base = I2C_BASE;
    uint32_t irq = IRQ_VEC_I2C;
    _iic_priv_t *iic_priv = handle;

    if(iic_priv->inited !=0 ) return ;

    iic_priv->base = base;
    iic_priv->irq  = irq;

    iic_priv->cb_event = NULL;
    iic_priv->rx_total_num = 0;
    iic_priv->tx_total_num = 0;
    iic_priv->rx_buf = NULL;
    iic_priv->tx_buf = NULL;
    iic_priv->rx_cnt = 0;
    iic_priv->tx_cnt = 0;
    iic_priv->status = 0;

	iic_priv->dev = i2c;
    iic_priv->dma_use = 0;
    iic_priv->dma_tx_ch = -1;
    iic_priv->dma_rx_ch = -1;

	iic_priv->dma_handle = ks_driver_dma_get_handle();

    //fixed assign tmp-align inner buffer for tx/rx
    iic_priv->tx_dma_align_ctrl.trans_head_addr  = s_i2c_dma_tx_head_buffer;
    iic_priv->tx_dma_align_ctrl.trans_tail_addr  = s_i2c_dma_tx_tail_buffer;
    iic_priv->rx_dma_align_ctrl.trans_head_addr  = s_i2c_dma_rx_head_buffer;
    iic_priv->rx_dma_align_ctrl.trans_tail_addr  = s_i2c_dma_rx_tail_buffer;

    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    /* mask all interrupts */
    addr->IC_INTR_MASK  = 0x00;
    addr->IC_CON        = DW_IIC_CON_DEFAUL;

	ks_os_irq_create(iic_priv->irq, _iic_irqhandler, (void*) iic_priv->idx);
	ks_os_irq_enable(iic_priv->irq);
	ks_os_irq_map_target(iic_priv->irq, 1);
	ks_shell_add_cmds(i2c_driver_cmds, sizeof(i2c_driver_cmds) / sizeof(cmd_proc_t));
}


iic_handle_t _iic_get_handle(int32_t idx)
{
	  _iic_priv_t *iic_priv = &iic_instance[idx];

	  if(iic_priv->idx!= idx)
	  iic_priv->idx = idx ;

	  return iic_priv;
}

/**
  \brief       De-initialize IIC Interface. stops operation and releases the software resources used by the interface
  \param[in]   handle  iic handle to operate.
  \return      error code
*/
int32_t _iic_uninitialize(iic_handle_t handle)
{
    IIC_NULL_PARAM_CHK(handle);

    /* First clear ACTIVITY, then Disable IIC */
    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    addr->IC_CLR_ACTIVITY;
    addr->IC_INTR_MASK  = 0x00;
    addr->IC_ENABLE = DW_IIC_DISABLE;

    iic_priv->cb_event = NULL;
    iic_priv->rx_total_num = 0;
    iic_priv->tx_total_num = 0;
    iic_priv->rx_buf = NULL;
    iic_priv->tx_buf = NULL;
    iic_priv->rx_cnt = 0;
    iic_priv->tx_cnt = 0;
    iic_priv->status = 0;
	


    if (iic_priv->dma_tx_ch != -1) {
        ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
        iic_priv->dma_tx_ch = -1;
    }

    if (iic_priv->dma_rx_ch != -1) {
        ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_rx_ch);
        ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
        iic_priv->dma_rx_ch = -1;
    }



    ks_os_irq_disable(iic_priv->irq);

	iic_priv->inited = 0 ;

    return 0;
}


/**
  \brief       config iic.
  \param[in]   handle  iic handle to operate.
  \param[in]   mode      \ref iic_mode_e.if negative, then this attribute not changed
  \param[in]   speed     \ref iic_speed_e.if negative, then this attribute not changed
  \param[in]   addr_mode \ref iic_address_mode_e.if negative, then this attribute not changed
  \param[in]   slave_addr slave address.if negative, then this attribute not changed
  \return      error code
*/
int32_t _iic_config(iic_handle_t handle,
                       iic_mode_e mode,
                       iic_speed_e speed,
                       iic_address_mode_e addr_mode,
                       int32_t slave_addr)
{
    int32_t ret;

    ret = _iic_config_mode(handle, mode);

    if (ret < 0) {
        return ret;
    }

    ret = _iic_config_speed(handle, speed);

    if (ret < 0) {
        return ret;
    }

    ret = _iic_config_addr_mode(handle, addr_mode);

    if (ret < 0) {
        return ret;
    }

    ret = _iic_config_slave_addr(handle, slave_addr);

    if (ret < 0) {
        return ret;
    }

    return 0;
}


/**
  \brief       config iic mode.
  \param[in]   handle  iic handle to operate.
  \param[in]   mode      \ref iic_mode_e.
  \return      error code
*/
int32_t _iic_config_mode(iic_handle_t handle, iic_mode_e mode)
{
    IIC_NULL_PARAM_CHK(handle);


    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    _iic_disable(addr);

    switch (mode) {
        case IIC_MODE_MASTER:
            iic_priv->mode = IIC_MODE_MASTER;
            addr->IC_CON |= DW_IIC_CON_MASTER_ENABLE;  /* master enbaled */
            break;

        case IIC_MODE_SLAVE:
            iic_priv->mode = IIC_MODE_SLAVE;
            addr->IC_CON &= ~DW_IIC_CON_SLAVE_ENABLE;  /* slave enabled */
            addr->IC_CON &= ~DW_IIC_CON_MASTER_ENABLE;  /* master enbaled */

            break;

        default:
            return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    return 0;
}

int32_t _iic_register_event_cb(iic_handle_t handle,iic_event_cb_t cb_event,void* arg){

	_iic_priv_t *iic_priv  = handle;


	iic_priv->cb_event = cb_event;
	iic_priv->arg = arg;
	return 0;

}

/**
  \brief       config iic speed.
  \param[in]   handle  iic handle to operate.
  \param[in]   speed     \ref iic_speed_e.
  \return      error code
*/
int32_t _iic_config_speed(iic_handle_t handle, iic_speed_e speed)
{
    IIC_NULL_PARAM_CHK(handle);
    _iic_priv_t *iic_priv = handle;

    _iic_reg_t *addr = (_iic_reg_t *)(((_iic_priv_t *)handle)->base);
    _iic_disable(addr);
	
	iic_priv->speed = speed;

    switch (speed) {
        case IIC_BUS_SPEED_STANDARD:
            _iic_set_transfer_speed(addr, DW_IIC_STANDARDSPEED);
	        _iic_set_hcnt(addr, DW_IIC_STANDARDSPEED);
            _iic_set_lcnt(addr, DW_IIC_STANDARDSPEED);
            break;

        case IIC_BUS_SPEED_FAST:
            _iic_set_transfer_speed(addr, DW_IIC_FASTSPEED);
			_iic_set_hcnt(addr, DW_IIC_FASTSPEED);
            _iic_set_lcnt(addr, DW_IIC_FASTSPEED);
            break;
			
        case IIC_BUS_SPEED_FAST_PLUS:
            _iic_set_transfer_speed(addr, DW_IIC_FASTPLUSSPEED);
			_iic_set_hcnt(addr, DW_IIC_FASTPLUSSPEED);
            _iic_set_lcnt(addr, DW_IIC_FASTPLUSSPEED);
		break;

        case IIC_BUS_SPEED_HIGH:
            _iic_set_transfer_speed(addr, DW_IIC_HIGHSPEED);
			_iic_set_hcnt(addr, DW_IIC_HIGHSPEED);
            _iic_set_lcnt(addr, DW_IIC_HIGHSPEED);
            break;

        default:
            return ERR_IIC(DRV_ERROR_PARAMETER);
    }


    return 0;
}

/**
  \brief       config iic address mode.
  \param[in]   handle  iic handle to operate.
  \param[in]   addr_mode \ref iic_address_mode_e.
  \return      error code
*/
int32_t _iic_config_addr_mode(iic_handle_t handle, iic_address_mode_e addr_mode)
{
    IIC_NULL_PARAM_CHK(handle);



    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    _iic_disable(addr);
	
	iic_priv->addr_mode = addr_mode;

    switch (addr_mode) {
        case IIC_ADDRESS_10BIT:
        case IIC_ADDRESS_7BIT:
            _iic_set_addr_mode(iic_priv, addr_mode);
            break;

        default:
            return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    return 0;
}

/**
  \brief       config iic slave address.
  \param[in]   handle  iic handle to operate.
  \param[in]   slave_addr slave address.
  \return      error code
*/
int32_t _iic_config_slave_addr(iic_handle_t handle, int32_t slave_addr)
{
    IIC_NULL_PARAM_CHK(handle);
    _iic_priv_t *iic_priv = handle;

    if (slave_addr < 0 || iic_priv->mode == IIC_MODE_MASTER) {
        return 0;
    }
	iic_priv->slave_addr = slave_addr;

    _iic_reg_t *addr = (_iic_reg_t *)(((_iic_priv_t *)handle)->base);
    _iic_disable(addr);

    _iic_set_slave_address(addr, slave_addr);
    return 0;
}

/**
  \brief       Start transmitting data as IIC Master.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref _iic_get_status can indicates transmission status.
  \param[in]   handle         iic handle to operate.
  \param[in]   devaddr        iic addrress of slave device.
  \param[in]   data           data to send to IIC Slave
  \param[in]   num            Number of data items to send
  \return      0 for success, negative for error code
*/
int32_t _iic_master_send(iic_handle_t handle, uint32_t devaddr, const void *data, uint32_t num)
{
    IIC_NULL_PARAM_CHK(handle);

    if (data == NULL || num == 0) {
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    _iic_set_target_address(addr, devaddr);

    iic_priv->tx_buf          = (uint8_t *)data;
    iic_priv->tx_total_num    = num;
    iic_priv->tx_cnt          = 0;
    iic_priv->status          = IIC_STATE_DATASEND;
    iic_priv->tx_addr_retry_cnt = 0;

    _iic_disable(addr);

    if (iic_priv->dev->tx_dma_enable) {
        return _iic_master_send_dma(iic_priv);
    }
    else{

        /* open corresponding interrupts */
        addr->IC_INTR_MASK = DW_IIC_INTR_DEFAULT_MASK;
        addr->IC_CLR_INTR;

        //if num is small enough for one FIFO loop, just set watermark to 0, or half empty
        addr->IC_TX_TL = (num > DW_IIC_FIFO_MAX_LV) ? (DW_IIC_FIFO_MAX_LV >> 1) : 0;

        _iic_enable(addr);

    }
    return 0;
}

/**
  \brief       Start receiving data as IIC Master.
               This function is non-blocking,\ref iic_event_e is signaled when transfer completes or error happens.
               \ref _iic_get_status can indicates transmission status.
  \param[in]   handle  iic handle to operate.
  \param[in]   devaddr        iic addrress of slave device.
  \param[out]  data    Pointer to buffer for data to receive from IIC receiver
  \param[in]   num     Number of data items to receive
  \return      0 for success, negative for error code
*/
int32_t _iic_master_receive(iic_handle_t handle, uint32_t devaddr, void *data, uint32_t num)
{
    IIC_NULL_PARAM_CHK(handle);

    if (data == NULL || num == 0) {
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    iic_priv->rx_buf          = (uint8_t *)data;
    iic_priv->rx_total_num    = num;
    iic_priv->rx_cnt          = 0;
    iic_priv->status          = IIC_STATE_WFDATA;
    iic_priv->cmd_read_num    = num;
    iic_priv->tx_addr_retry_cnt = 0;

    _iic_set_target_address(addr, devaddr);
    _iic_disable(addr);

	if (iic_priv->dev->rx_dma_enable && iic_priv->dev->tx_dma_enable) {
		return _iic_master_receive_dma(iic_priv);
	}

    addr->IC_INTR_MASK = DW_IIC_INTR_STOP_DET | DW_IIC_INTR_RX_FULL;
    addr->IC_CLR_INTR;

    //if num is small enough for one FIFO loop, just set watermark to 0, or half empty
    addr->IC_RX_TL = IIC_GET_FAIR_RX_TL(iic_priv->rx_total_num);
    addr->IC_TX_TL = 0; //always using max TL, that is 0

    _iic_enable(addr);

    //push dummy data as needed
    uint16_t dummy_byte = 1 << 8;
    for (unsigned int i=0; i<num && i<DW_IIC_FIFO_MAX_LV; i++, iic_priv->cmd_read_num--){
        addr->IC_DATA_CMD = dummy_byte;
    }

    return 0;
}






/**
  \brief       Start transmitting data as IIC Slave.
  \param[in]   handle  iic handle to operate.
  \param[in]   data  Pointer to buffer with data to transmit to IIC Master
  \param[in]   num   Number of data items to send
  \return      error code
*/
int32_t _iic_slave_send(iic_handle_t handle, const void *data, uint32_t num)
{
    IIC_NULL_PARAM_CHK(handle);

    if (data == NULL || num == 0) {
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    iic_priv->tx_buf          = (uint8_t *)data;
    iic_priv->tx_total_num    = num;
    iic_priv->tx_cnt          = 0;
    iic_priv->status          = IIC_STATE_DATASEND;

    //@note whether using DMA or not, both should wait for read request to start DMA, not auto start like other modes
    if (iic_priv->dev->tx_dma_enable) {
        //only preparing env to send
        _iic_slave_send_dma(iic_priv);
    }

    _iic_disable(addr);
	
    /* open corresponding read interrupts */
    addr->IC_INTR_MASK =  DW_IIC_INTR_RD_REQ | DW_IIC_INTR_STOP_DET;
    addr->IC_CLR_INTR;

    //update next watermarker (half empty + 1 or all data)
    addr->IC_TX_TL = (iic_priv->tx_total_num > DW_IIC_FIFO_MAX_LV) ? (DW_IIC_FIFO_MAX_LV >> 1) : 0;

    _iic_enable(addr);

    return 0;
}

/**
  \fn          int32_t  _iic_slave_receive (iic_handle_t handle, const void *data, uint32_t num)
  \brief       Start receiving data as IIC Slave.
  \param[in]   handle  iic handle to operate.
  \param[out]  data  Pointer to buffer for data to receive from IIC Master
  \param[in]   num   Number of data items to receive
  \return      error code
*/
int32_t _iic_slave_receive(iic_handle_t handle, void *data, uint32_t num)
{
    IIC_NULL_PARAM_CHK(handle);

    if (data == NULL || num == 0) {
        return ERR_IIC(DRV_ERROR_PARAMETER);
    }

    _iic_priv_t *iic_priv = handle;

    iic_priv->rx_buf            = (uint8_t *)data;
    iic_priv->rx_total_num      = num;
    iic_priv->rx_cnt            = 0;
    iic_priv->status            = IIC_STATE_WFDATA;

    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);
    _iic_disable(addr);

	if (iic_priv->dev->rx_dma_enable) {
        return _iic_slave_receive_dma(iic_priv);
    }
    else{
        addr->IC_INTR_MASK = DW_IIC_INTR_STOP_DET | DW_IIC_INTR_RX_FULL | DW_IIC_INTR_START_DET;
        addr->IC_CLR_INTR;

        /* Sets receive FIFO threshold */
        addr->IC_RX_TL = ((iic_priv->rx_total_num > DW_IIC_FIFO_MAX_LV) ? (DW_IIC_FIFO_MAX_LV >> 1) : iic_priv->rx_total_num) - 1;

        _iic_enable(addr);
    }

    return 0;
}

/**
  \brief       abort transfer.
  \param[in]   handle  iic handle to operate.
  \return      error code
*/
int32_t _iic_abort_transfer(iic_handle_t handle)
{
    IIC_NULL_PARAM_CHK(handle);

    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);

    _iic_disable(addr);



	if(iic_priv->dma_use != 0){
		
		if (iic_priv->dma_tx_ch != -1) {
			ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_tx_ch);
			ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_tx_ch);
			iic_priv->dma_tx_ch = -1;
			addr->IC_DMA_CR &= ~DW_IIC_DMACR_TDMAE_Msk;
		}
		
		if (iic_priv->dma_rx_ch != -1) {
			//ks_driver_dma_suspend(iic_priv->dma_handle,iic_priv->dma_rx_ch);
			ks_driver_dma_stop(iic_priv->dma_handle,iic_priv->dma_rx_ch);
			ks_driver_dma_release_channel(iic_priv->dma_handle,iic_priv->dma_rx_ch);
			iic_priv->dma_rx_ch = -1;
			addr->IC_DMA_CR &= ~DW_IIC_DMACR_RDMAE_Msk;
		}

		
		ks_os_irq_enable(iic_priv->irq);
		
		iic_priv->dma_tx_ch = -1;
		iic_priv->dma_rx_ch = -1;
		iic_priv->dma_use = 0;

	}

    iic_priv->rx_cnt          = 0;
    iic_priv->tx_cnt          = 0;
    iic_priv->rx_buf          = NULL;
    iic_priv->tx_buf          = NULL;
    return 0;
}


/**
  \brief       Get IIC status.
  \param[in]   handle  iic handle to operate.
  \return      IIC status \ref iic_status_t
*/
iic_status_t _iic_get_status(iic_handle_t handle)
{
    iic_status_t iic_status = {0};

    if (handle == NULL) {
        return iic_status;
    }

    _iic_priv_t *iic_priv = handle;
    _iic_reg_t *addr = (_iic_reg_t *)(iic_priv->base);


    if ((iic_priv->status == IIC_STATE_DATASEND) || (iic_priv->status == IIC_STATE_WFDATA)) {
        iic_status.busy = 1;
    }

    if (iic_priv->status == IIC_STATE_WFDATA) {
        iic_status.direction = 1;
    }

    if (addr->IC_RAW_INTR_STAT & 0x800) {
        iic_status.general_call = 1;
    }

    if (iic_priv->status == IIC_STATE_ERROR) {
        iic_status.bus_error = 1;
    }

    return iic_status;
}

/**
  \brief       Get IIC transferred data count.
  \param[in]   handle  iic handle to operate.
  \return      number of data bytes transferred
*/
uint32_t _iic_get_data_count(iic_handle_t handle)
{
    uint32_t cnt = 0;

    if (handle == NULL) {
        return 0;
    }

    _iic_priv_t *iic_priv = handle;

    if ((iic_priv->status == IIC_STATE_WFDATA) || (iic_priv->status == IIC_STATE_RECV_DONE)) {
        cnt = iic_priv->rx_cnt;
    } else if ((iic_priv->status == IIC_STATE_DATASEND) || (iic_priv->status == IIC_STATE_SEND_DONE)) {
        cnt = iic_priv->tx_cnt;
    }

    return cnt;
}


/**
  \brief       Send START command.
  \param[in]   handle  iic handle to operate.
  \return      error code
*/
int32_t _iic_send_start(iic_handle_t handle)
{
    return ERR_IIC(DRV_ERROR_UNSUPPORTED);
}

/**
  \brief       Send STOP command.
  \param[in]   handle  iic handle to operate.
  \return      error code
*/
int32_t _iic_send_stop(iic_handle_t handle)
{
    return ERR_IIC(DRV_ERROR_UNSUPPORTED);
}



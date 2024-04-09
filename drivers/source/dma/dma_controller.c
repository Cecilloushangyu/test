
#include <stdbool.h>
#include "dw_dmac.h"
#include "ks_dma.h"
#include <string.h>
#include <ks_os.h>
#include <ks_shell.h>
#include "dma_controller.h"
#include "ks_cache.h"



static dw_dma_priv_t dma_instance;

static int dma_ctx_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
	uint32_t channel_id = -1;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &channel_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
    dw_dma_priv_t *dma_priv = &dma_instance;
	
	ks_shell_printf(ctx->uart_id,"\r\n");

	if(channel_id  > dma_priv->ch_num){

		for(int i = 0 ;i<dma_priv->ch_num;i++){
			ks_shell_printf(ctx->uart_id,"ch_id : %d ch_opened : %d status : %d alloc_success : %d alloc_fails : %d free_success %d\r\n",dma_priv->ch_ctx[i].ch_id,dma_priv->ch_opened[i],dma_priv->status[i],dma_priv->ch_ctx[i].alloc_success,dma_priv->ch_ctx[i].alloc_fails,dma_priv->ch_ctx[i].free_success);
		}

	}else{
		ks_shell_printf(ctx->uart_id,"ch_id : %d ch_opened : %d status : %d alloc_success : %d alloc_fails : %d free_success %d \r\n",dma_priv->ch_ctx[channel_id].ch_id,dma_priv->ch_opened[channel_id],dma_priv->status[channel_id],dma_priv->ch_ctx[channel_id].alloc_success,dma_priv->ch_ctx[channel_id].alloc_fails,dma_priv->ch_ctx[channel_id].free_success);
	}

    return 0;
}

static int dma_clear_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t channel_id = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &channel_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}

	
    dw_dma_priv_t *dma_priv = &dma_instance;

	
	dma_channel_context* channel = &dma_priv->ch_ctx[channel_id];
	

	channel->alloc_success = 0;
	channel->alloc_fails = 0;
	ks_shell_printf(ctx->uart_id,"\r\n");

	ks_shell_printf(ctx->uart_id,"ch_id : %d \r\n",channel->ch_id);
	ks_shell_printf(ctx->uart_id,"alloc_success : %d \r\n",channel->alloc_success);
	ks_shell_printf(ctx->uart_id,"alloc_fails : %d \r\n",channel->alloc_fails);

    return 0;
}

static int dma_reg_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;
    uint32_t channel_id = 0;
	uint32_t src_offset = 0;
	uint32_t dst_offset = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &channel_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}

    dw_dma_priv_t *dma_priv = &dma_instance;
    uint32_t addr = dma_priv->base;
    uint32_t sarx = readl(addr +channel_id * 0x58+ DMA_REG_SARx);
    uint32_t darx = readl(addr +channel_id * 0x58+ DMA_REG_DARx);

	ks_shell_printf(ctx->uart_id,"\r\n");
	
	ks_shell_printf(ctx->uart_id,"DMA_REG_SAR%d : %x \r\n",channel_id,sarx);
	ks_shell_printf(ctx->uart_id,"DMA_REG_DAR%d : %x \r\n",channel_id,darx);
	ks_shell_printf(ctx->uart_id,"psrcaddr%d : %x \r\n",channel_id,dma_priv->ch_ctx[channel_id].psrcaddr);
	ks_shell_printf(ctx->uart_id,"pdstaddr%d : %x \r\n",channel_id,dma_priv->ch_ctx[channel_id].pdstaddr);
	ks_shell_printf(ctx->uart_id,"DMA_REG_CTRLax%d : %x \r\n",channel_id, readl(addr +channel_id * 0x58+ DMA_REG_CTRLax));
	ks_shell_printf(ctx->uart_id,"DMA_REG_CTRLbx%d : %x \r\n",channel_id, readl(addr +channel_id * 0x58+ DMA_REG_CTRLbx));
	ks_shell_printf(ctx->uart_id,"DMA_REG_CFGax%d : %x \r\n",channel_id, readl(addr +channel_id * 0x58+ DMA_REG_CFGax));
	ks_shell_printf(ctx->uart_id,"DMA_REG_CFGbx%d : %x \r\n",channel_id, readl(addr +channel_id * 0x58+ DMA_REG_CFGbx));

	_dma_get_trans_offset((OSHandle)dma_priv,(int)channel_id,&src_offset,&dst_offset);
	ks_shell_printf(ctx->uart_id,"ch_id : %d src_offset : %d dst_offset : %d\r\n",channel_id,src_offset,dst_offset);

    return 0;
}

static cmd_proc_t dma_ctx_cmds[] = {
	{.cmd = "dma_ctx", .fn = dma_ctx_cmd,  .help = "dma_ctx <channel>"},
	{.cmd = "dma_clear", .fn = dma_clear_cmd,  .help = "dma_clear <channel>"},
	{.cmd = "dma_reg", .fn = dma_reg_cmd,  .help = "dma_reg "},

};

static int32_t _dma_set_channel(uint32_t addr, uint8_t ch, uint32_t source, uint32_t dest, uint32_t size)
{
    writel(size, addr + ch * 0x58 + DMA_REG_CTRLbx);
    writel(source, addr + ch * 0x58 + DMA_REG_SARx);
    writel(dest, addr + ch * 0x58 + DMA_REG_DARx);

    return 0;
}

static int32_t _dma_set_transfertype(uint32_t addr, uint8_t ch, dma_trans_type_e transtype)
{
    if (transtype > DMA_PERH2MEM) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CTRLax);
    value &= ~(0x300000);
    value |= transtype << 20;
    writel(value, addr + ch * 0x58 + DMA_REG_CTRLax);

    return 0;
}

static int32_t _dma_set_addrinc(uint32_t addr, uint8_t ch, dma_addr_inc_e src_addrinc, dma_addr_inc_e dst_addrinc)
{
    if ((src_addrinc != DMA_ADDR_INC && src_addrinc != DMA_ADDR_DEC && src_addrinc != DMA_ADDR_CONSTANT) ||
        (dst_addrinc != DMA_ADDR_INC && dst_addrinc != DMA_ADDR_DEC && dst_addrinc != DMA_ADDR_CONSTANT)) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CTRLax);
    value &= ~(0x780);
    value |= (src_addrinc << 9);
    value |= (dst_addrinc << 7);
    writel(value, addr + ch * 0x58 + DMA_REG_CTRLax);

    return 0;
}

static int32_t _dma_set_transferwidth(uint32_t addr, uint8_t ch, dma_datawidth_e src_width, dma_datawidth_e dst_width)
{
    if ((src_width != DMA_DATAWIDTH_SIZE8 && src_width != DMA_DATAWIDTH_SIZE16 && src_width != DMA_DATAWIDTH_SIZE32) ||
        (dst_width != DMA_DATAWIDTH_SIZE8 && dst_width != DMA_DATAWIDTH_SIZE16 && dst_width != DMA_DATAWIDTH_SIZE32)) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CTRLax);
    value &= ~(0x7e);

    if (src_width == DMA_DATAWIDTH_SIZE32) {
        value |= (src_width - 2) << 4;
    } else {
        value |= (src_width - 1) << 4;
    }

    if (dst_width == DMA_DATAWIDTH_SIZE32) {
        value |= (dst_width - 2) << 1;
    } else {
        value |= (dst_width - 1) << 1;
    }

    writel(value, addr + ch * 0x58 + DMA_REG_CTRLax);

    return 0;
}

static int32_t _dma_set_burstlength(uint32_t addr, uint8_t ch, uint8_t burstlength)
{
    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CTRLax);
    value &= ~(0x1f800);
    value |= burstlength << 11 | burstlength << 14;
    writel(value, addr + ch * 0x58 + DMA_REG_CTRLax);

    return 0;

}

/**
  \brief       Set software or hardware handshaking.
  \param[in]   addr pointer to dma register.
  \return      error code
*/
static int32_t _dma_set_handshaking(uint32_t addr, uint8_t ch, dma_channel_req_mode_e src_handshaking, dma_channel_req_mode_e dst_handshaking)
{
    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CFGax);
    value &= ~(0xc00);
    value |= (src_handshaking << 11  | dst_handshaking << 10);
    writel(value, addr + ch * 0x58 + DMA_REG_CFGax);

    return 0;
}

static int32_t _dma_set_max_abrst(uint32_t addr, uint8_t ch)
{
    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CFGax);
    value |= (1 << 20 );
    writel(value, addr + ch * 0x58 + DMA_REG_CFGax);

    return 0;
}

static int _dma_assign_hdhs_interface(uint32_t addr, uint8_t ch, dma_handshaking_device_e src_device, dma_handshaking_device_e dst_device)
{
    if ( src_device >= DMA_DEV_MAX || dst_device >= DMA_DEV_MAX) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CFGbx);
    value &= ~(0x7f80);
    value |= (src_device << 7 | dst_device << 11);
    writel(value, addr + ch * 0x58 + DMA_REG_CFGbx);

    return 0;
}


void _dma_irq_handler(void* arg)
{
    dw_dma_priv_t *dma_priv = (dw_dma_priv_t *)arg;
    uint32_t addr = dma_priv->base;

    /*
     * StatusInt_temp contain the information that which types of interrupr are
     * requested.
     */
    uint32_t count = 0;

    uint32_t status_tfr = readl(addr + DMA_REG_StatusTfr);
    uint32_t status_block = readl(addr + DMA_REG_StatusBlock);
    uint32_t status_err = readl(addr + DMA_REG_StatusErr);

    writel(status_tfr, addr + DMA_REG_ClearTfr);
    writel(status_block, addr + DMA_REG_ClearBlock);
    writel(status_err, addr + DMA_REG_ClearErr);

    if (status_tfr) {
        for (count = 0; count < dma_priv->ch_num; count++) {
            if (status_tfr &(uint32_t)(1 << count)) {
				dma_priv->status[count] = DMA_STATE_DONE;
				if (dma_priv->ch_ctx[count].callback!=NULL) {
					dma_priv->ch_ctx[count].callback(&dma_priv->ch_ctx[count],count, DMA_EVENT_TRANSFER_DONE);
					//dma_priv->ch_ctx[count].callback = NULL;
					//dma_priv->ch_ctx[count].arg = NULL;
				}
            }
        }
		
    }



    if (status_err) {

        for (count = 0; count < dma_priv->ch_num; count++) {
			if (status_err & (uint32_t) (1 << count)) {
				//writel(1U << count, addr + DMA_REG_ClearTfr);
			    dma_priv->status[count] = DMA_STATE_ERROR;
				if (dma_priv->ch_ctx[count].callback!=NULL) {
					dma_priv->ch_ctx[count].callback(&dma_priv->ch_ctx[count],count, DMA_EVENT_TRANSFER_ERROR);
					//dma_priv->ch_ctx[count].callback = NULL;
					//dma_priv->ch_ctx[count].arg = NULL;
				}
            }
        }
    }
}

OSHandle _dma_get_handle(int32_t idx)
{
	dw_dma_priv_t *dma_priv = &dma_instance;
	return (OSHandle)dma_priv;
}

/**
  \brief       Initialize DMA Interface. 1. Initializes the resources needed for the DMA interface 2.registers event callbadw function
  \param[in]   dmac idx
  \return      pointer to dma instances
*/
OSHandle _dma_initialize(int32_t idx)
{

    if (idx < 0 || idx >= CONFIG_DMAC_NUM) {
        return 0;
    }

    uint32_t base = DMAC_BASE;
    uint32_t irq = IRQ_VEC_DMA;


    dw_dma_priv_t *dma_priv = &dma_instance;

	
	if(dma_priv->inited !=0 ) return (OSHandle)dma_priv;

	memset(dma_priv,0,sizeof(dw_dma_priv_t));
    dma_priv->base = base;
    dma_priv->irq  = irq;
    dma_priv->ch_num = DMA_MAXCHANNEL;
	
	for (int ch_num = 0; ch_num < dma_priv->ch_num; ch_num++) {
		dma_priv->ch_ctx[ch_num].ch_id = ch_num;
	}

	ks_os_irq_create(dma_priv->irq, _dma_irq_handler, (void*)dma_priv);
	ks_os_irq_enable(dma_priv->irq);
	ks_os_irq_map_target(dma_priv->irq, 1);

    writel(DMA_MASK, base + DMA_REG_MaskTfr);
    writel(DMA_MASK, base + DMA_REG_MaskErr);
    writel(DMA_MASK, base + DMA_REG_MaskBlock);
    writel(DMA_MASK, base + DMA_REG_MaskSrcTran);
    writel(DMA_MASK, base + DMA_REG_MaskDstTran);
    writel(DMA_INTC, base + DMA_REG_ClearTfr);
    writel(DMA_INTC, base + DMA_REG_ClearBlock);
    writel(DMA_INTC, base + DMA_REG_ClearErr);
    writel(DMA_INTC, base + DMA_REG_ClearSrcTran);
    writel(DMA_INTC, base + DMA_REG_ClearDstTran);

	dma_priv->inited = 1;
					 
	ks_shell_add_cmds(dma_ctx_cmds, sizeof(dma_ctx_cmds) / sizeof(cmd_proc_t));

    return (OSHandle)dma_priv;
}

/**
  \brief       De-initialize DMA Interface. stops operation and releases the software resources used by the interface
  \param[in]   handle damc handle to operate.
  \return      error code
*/
int32_t _dma_uninitialize(OSHandle handle)
{
    if (handle == 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dw_dma_priv_t *dma_priv = (dw_dma_priv_t *)handle;
    uint32_t addr = dma_priv->base;

    writel(DMA_MASK, addr + DMA_REG_MaskTfr);
    writel(DMA_MASK, addr + DMA_REG_MaskErr);
    writel(DMA_MASK, addr + DMA_REG_MaskBlock);
    writel(DMA_MASK, addr + DMA_REG_MaskSrcTran);
    writel(DMA_MASK, addr + DMA_REG_MaskDstTran);
    writel(DMA_INTC, addr + DMA_REG_ClearTfr);
    writel(DMA_INTC, addr + DMA_REG_ClearBlock);
    writel(DMA_INTC, addr + DMA_REG_ClearErr);
    writel(DMA_INTC, addr + DMA_REG_ClearSrcTran);
    writel(DMA_INTC, addr + DMA_REG_ClearDstTran);
	
	dma_priv->inited = 0;

    ks_os_irq_disable(dma_priv->irq);

    return 0;
}

/**
  \brief     get one free dma channel
  \param[in] handle damc handle to operate.
  \return    -1 - no channel can be used, other - channel index
 */
int32_t _dma_bind_channel(OSHandle handle)
{
    dw_dma_priv_t *dma_priv =(dw_dma_priv_t *) handle;

    if (handle == 0 ) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint8_t ch_num = 0;

    for (ch_num = 0; ch_num < dma_priv->ch_num; ch_num++) {
        if (dma_priv->ch_opened[ch_num] == 0) {
            dma_priv->ch_opened[ch_num] = (0x01<<(4+ch_num));
            break;
        }
    }

    if (ch_num >= dma_priv->ch_num) {
        return -1;
    }

    return ch_num;
}

/**
  \brief     get one free dma channel
  \param[in] handle damc handle to operate.
  \param[in] ch channel num. if -1 then allocate a free channal in this dma
  \return    -1 - no channel can be used, other - channel index
 */
int32_t _dma_alloc_channel(OSHandle handle, int32_t ch)
{
    dw_dma_priv_t *dma_priv =(dw_dma_priv_t *) handle;

    if (handle == 0 || ch > dma_priv->ch_num) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint8_t ch_num = 0;

    if (ch == -1) {     // alloc a free channal
        for (ch_num = 0; ch_num < dma_priv->ch_num; ch_num++) {
            if (dma_priv->ch_opened[ch_num] == 0) {
                dma_priv->ch_opened[ch_num] = 1;
                break;
            }
        }

        if (ch_num >= dma_priv->ch_num) {
			dma_priv->ch_ctx[ch_num].alloc_fails ++;
            return -1;
        }
    } else {    //alloc a fixed channel
        if ((uint8_t)(dma_priv->ch_opened[ch] & 0x0F) != 0 ){
			dma_priv->ch_ctx[ch_num].alloc_fails ++;
            return ERR_DMA(DRV_ERROR_BUSY);
        }


		//通道初始化时候，已经提前分配好,判断状态是否异常
		if((uint8_t)(dma_priv->ch_opened[ch] & 0xF0) != 0){
			if ((uint8_t)(dma_priv->ch_opened[ch]&0xF0) != (uint8_t)(0x01<<(4+ch))){
				dma_priv->ch_ctx[ch_num].alloc_fails ++;
	            return ERR_DMA(DRV_ERROR_STATUS);
			}
		}

        dma_priv->ch_opened[ch] |= 0x1;
        ch_num = ch;
    }

    uint32_t addr = dma_priv->base;

    writel((1 << ch_num), addr + DMA_REG_ClearTfr);
    writel((1 << ch_num), addr + DMA_REG_ClearBlock);
    writel((1 << ch_num), addr + DMA_REG_ClearErr);
    writel((1 << ch_num), addr + DMA_REG_ClearSrcTran);
    writel((1 << ch_num), addr + DMA_REG_ClearDstTran);

    uint32_t value = 1 << ch_num | (1 << (ch_num + 8));
    writel(value, addr + DMA_REG_MaskTfr);
    writel(value, addr + DMA_REG_MaskErr);

    dma_priv->status[ch_num] = DMA_STATE_READY;
	dma_priv->ch_ctx[ch_num].alloc_success ++;

    return ch_num;
}

/**
  \brief        release dma channel and related resources
  \param[in]    handle damc handle to operate.
  \param[in]    ch  channel num.
  \return       error code
 */
int32_t _dma_release_channel(OSHandle handle, int32_t ch)
{
    dw_dma_priv_t *dma_priv = (dw_dma_priv_t *) handle;

    if (handle == 0 || ch >= dma_priv->ch_num || ch < 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint32_t addr = dma_priv->base;
    dma_priv->status[ch] = DMA_STATE_FREE;

	dma_priv->ch_opened[ch] &= ~(0x01);

    writel((1 << ch), addr + DMA_REG_ClearTfr);
    writel((1 << ch), addr + DMA_REG_ClearBlock);
    writel((1 << ch), addr + DMA_REG_ClearErr);
    writel((1 << ch), addr + DMA_REG_ClearSrcTran);
    writel((1 << ch), addr + DMA_REG_ClearDstTran);

    uint32_t value = (1 << (ch + 8));
    writel(value, addr + DMA_REG_MaskTfr);
    writel(value, addr + DMA_REG_MaskErr);
	dma_priv->ch_ctx[ch].free_success ++;
    return 0;
}



/**
  \brief        config dma channel
  \param[in]    handle damc handle to operate.
  \param[in]    ch          channel num.
  \param[in]    config      dma channel transfer configure
  \param[in]    cb_event    Pointer to \ref dma_event_cb_t
  \return       error code
 */
int32_t _dma_config_channel(OSHandle handle, int32_t ch,
                               dma_config_t *config,dma_event_cb callback,void* arg)
{
    dw_dma_priv_t *dma_priv = (dw_dma_priv_t *)handle;

    if (handle == 0 || ch >= dma_priv->ch_num || config == NULL) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }


    if (ch == -1) { //alloc a free channel
        ch = _dma_alloc_channel(handle, -1);

        if (ch < 0) {
            return ERR_DMA(DRV_ERROR_BUSY);
        }
    }

    if (dma_priv->ch_opened[ch] == 0) {
        return ERR_DMA(DRV_ERROR_BUSY);
    }

    dma_priv->ch_ctx[ch].ch_id = ch;
    dma_priv->ch_ctx[ch].arg = arg;
    dma_priv->ch_ctx[ch].callback = callback;

    uint32_t addr = dma_priv->base;

    /* Initializes corresponding channel registers */
	_dma_set_max_abrst(addr, ch);


    int32_t ret = _dma_set_transferwidth(addr, ch, config->src_tw, config->dst_tw);

    if (ret < 0) {
        return ret;
    }

    dma_priv->dst_tw = config->dst_tw;
    dma_priv->src_tw = config->src_tw;

//    int32_t grouplen = ((length * config->src_tw / config->dst_tw) - 1) % 16;

    ret = _dma_set_transfertype(addr, ch, config->type);

    if (ret < 0) {
        return ret;
    }

    if (config->type == DMA_MEM2MEM) {
        _dma_set_handshaking(addr, ch, DMA_HANDSHAKING_SOFTWARE, DMA_HANDSHAKING_SOFTWARE);
        ret = _dma_set_addrinc(addr, ch, config->src_inc, config->dst_inc);

        if (ret < 0) {
            return ret;
        }

    } else if (config->type == DMA_MEM2PERH) {
        _dma_set_handshaking(addr, ch, DMA_HANDSHAKING_SOFTWARE, DMA_HANDSHAKING_HARDWARE);
        ret = _dma_set_addrinc(addr, ch, config->src_inc, config->dst_inc);

        if (ret < 0) {
            return ret;
        }

        ret = _dma_assign_hdhs_interface(addr, ch, config->hs_if, config->hs_if);

        if (ret < 0) {
            return ret;
        }

    } else if (config->type == DMA_PERH2MEM) {
        _dma_set_handshaking(addr, ch, DMA_HANDSHAKING_HARDWARE, DMA_HANDSHAKING_SOFTWARE);
        ret = _dma_set_addrinc(addr, ch, config->src_inc, config->dst_inc);

        if (ret < 0) {
            return ret;
        }

        ret = _dma_assign_hdhs_interface(addr, ch, config->hs_if, config->hs_if);

        if (ret < 0) {
            return ret;
        }

    }

	//kprintf("_dma_set_burstlength %d \r\n",grouplen );

    _dma_set_burstlength(addr, ch, config->burst_len);

	dma_priv->ch_ctx[ch].transtype = config->type ;

    writel(0x1, addr + DMA_REG_Cfg);
    dma_priv->status[ch] = DMA_STATE_READY;

    return ch;
}


uint8_t _dma_find_max_prime_num(uint32_t num)
{
	uint8_t ret;

	if (!(num % 8U)) {
	   ret = 8U;
	} else if (!(num % 4U)) {
	   ret = 4U;
	} else {
	   ret = 1U;
	}

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
int32_t _dma_start(OSHandle handle, int32_t ch, void *psrcaddr,
                      void *pdstaddr, uint32_t length)
{
    dw_dma_priv_t *dma_priv = (dw_dma_priv_t *)handle;

    if (handle == 0 || ch >= dma_priv->ch_num || ch < 0 || psrcaddr == NULL || pdstaddr == NULL || length > DMA_CHx_MAX_BLK_SIZE) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dma_priv->status[ch] = DMA_STATE_BUSY;
    uint32_t addr = dma_priv->base;
	dma_priv->ch_ctx[ch].len = length;

    if ((length * dma_priv->src_tw) % dma_priv->dst_tw != 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

#if 0
    uint8_t i;

    for (i = 7; i > 0; i--) {
        if (!((length * dma_priv->src_tw / dma_priv->dst_tw) % (2 << (i + 1)))) {
            break;
        }
    }

	    int32_t grouplen = 0;
    //int32_t grouplen = i;

    //_dma_set_burstlength(addr, ch, grouplen);
#endif


    _dma_set_channel(addr, ch, (uint32_t)psrcaddr, (uint32_t)pdstaddr, length);

	dma_priv->ch_ctx[ch].pdstaddr = (uint32_t)psrcaddr;
	dma_priv->ch_ctx[ch].pdstaddr = (uint32_t)pdstaddr;
	

    // interrupt enable
    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CTRLax);
    value |= DMA_INT_EN;
    writel(value, addr + ch * 0x58 + DMA_REG_CTRLax);

    value = readl(addr + DMA_REG_ChEn);
    value |= (DMA_CH_EN << (8 + ch)) | (DMA_CH_EN << ch);
    writel(value, addr + DMA_REG_ChEn);

    return 0;
}

/**
  \brief       Stop generate dma channel signal.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \return      error code
*/
int32_t _dma_stop(OSHandle handle, int32_t ch)
{
    if (handle == 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dw_dma_priv_t *dma_priv =(dw_dma_priv_t *)handle;

    if (ch >= dma_priv->ch_num || ch < 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dma_priv->status[ch] = DMA_STATE_DONE;

    uint32_t addr = dma_priv->base;
    // interrupt disable
    uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CTRLax);
    value &= ~DMA_INT_EN;
    writel(value, addr + ch * 0x58 + DMA_REG_CTRLax);

    value = readl(addr + DMA_REG_ChEn);
    value |= (DMA_CH_EN << (8 + ch));
    value &= ~(DMA_CH_EN << ch);
    writel(value, addr + DMA_REG_ChEn);
    return 0;
}

/**
  \brief       Get DMA channel status.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \return      DMA status \ref dma_status_t
*/
int32_t _dma_get_status(OSHandle handle, int32_t ch)
{
    if (handle == 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dw_dma_priv_t *dma_priv = ( dw_dma_priv_t *)handle;

    if (ch >= dma_priv->ch_num || ch < 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    return dma_priv->status[ch];
}


/**
  \brief       Get DMA channel trans offset.
  \param[in]   handle damc handle to operate.
  \param[in]   ch  channel num.
  \param[in&out]   psrc_offset src trans offset.
  \param[in&out]   pdst_offset dst trans offset.
  \return        error code
*/
int32_t _dma_get_trans_offset(OSHandle handle, int32_t ch, uint32_t* psrc_offset,uint32_t* pdst_offset)
{
    if (handle == 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dw_dma_priv_t *dma_priv = ( dw_dma_priv_t *)handle;

    if (ch >= dma_priv->ch_num || ch < 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint32_t addr = dma_priv->base;
    uint32_t sarx = readl(addr +ch * 0x58+ DMA_REG_SARx);
    uint32_t darx = readl(addr +ch * 0x58+ DMA_REG_DARx);

	if(dma_priv->ch_ctx[ch].pdstaddr >= darx ){
		*pdst_offset = (dma_priv->ch_ctx[ch].pdstaddr - darx);
	}else{
		*pdst_offset = (darx - dma_priv->ch_ctx[ch].pdstaddr );

	}

	if(dma_priv->ch_ctx[ch].psrcaddr >= sarx ){
		*psrc_offset = (dma_priv->ch_ctx[ch].psrcaddr - sarx);
	}else{
		*psrc_offset = (sarx - dma_priv->ch_ctx[ch].psrcaddr);

	}
    return 0;
}

int32_t _dma_check_fifo_empty(OSHandle handle, int32_t ch){
    if (handle == 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dw_dma_priv_t *dma_priv =(dw_dma_priv_t *)handle;

    if (ch >= dma_priv->ch_num || ch < 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    uint32_t addr = dma_priv->base;
	uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CFGax);
	if(value & (1 << 9) ){
		return 1;
	}else{
		return 0;
	}
	
}

int32_t _dma_suspend(OSHandle handle, int32_t ch, int enable)
{
    if (handle == 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }

    dw_dma_priv_t *dma_priv =(dw_dma_priv_t *)handle;

    if (ch >= dma_priv->ch_num || ch < 0) {
        return ERR_DMA(DRV_ERROR_PARAMETER);
    }


    uint32_t addr = dma_priv->base;
	uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CFGax);
	
	if(enable !=0){
		value |= (1 << 8);
	}else{
		value &= ~(1 << 8);
	}
	writel(value, addr + ch * 0x58 + DMA_REG_CFGax);
	
	if(enable){
		while(1){
			uint32_t value = readl(addr + ch * 0x58 + DMA_REG_CFGax);
			if(value & (1 << 9) ){
				break;
			}
		}
	}
	
    return 0;
}


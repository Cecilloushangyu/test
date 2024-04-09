
#include <ks_datatypes.h>
#include "mmcsd_controller.h"
#include "mmcsd_low_level_driver.h"
#include "ks_shell.h"
#include "ks_gpio.h"
#include "ks_sysctrl.h"
#include "ks_mmcsd.h"
#include "ks_cache.h"

//根据硬件设计　自定义 GPIO 
#define  SDCARD_DETECT_PIN  GPIOA_25  


#define MSHC_ALIGN_LEN   1024  // 要求DMA起始地址为1K对齐
#define MSHC_BUFF_SIZE   (MSHC_MAX_MUTI_BLOCK_COUNT*MSHC_BLOCK_SIZE)  

// 为EMMC分配了两个长1KB的cache buffer，可以考虑用ping-pong的方式来提高性能
#define CACHE_BUF_ADDR (0x901E1000)  // Buffer的虚拟地址
#define CACHE_BUF_ADDR_PHY (0x001E1000)  // 对应的物理地址
#define CACHE_BUF_ADDR_2 (0x901E1400)  // Buffer的虚拟地址
#define CACHE_BUF_ADDR_2_PHY (0x001E1400)  // 对应的物理地址

#define DelayMicrosecond ks_os_poll_delay_usec

#define ERR_RECOVER_MAX 2

static int s_sd_detect_gpio;

typedef struct mshc
{
    struct mmcsd_host  *host;
    char* name;
    MSHC_REG_T* base;
    int        irqn;
    uint8_t         *cachebuf_phy;
    uint8_t         *cachebuf_virtual;

}mshc_ctx;

//static uint8_t g_nocache_buffer[MSHC_BUFF_SIZE] __attribute__((aligned(64)));

mshc_ctx g_mshc_ctx;
sd_recover_t g_sd_recover;
	
void mshc_reset(MSHC_REG_T *mshc, uint8_t u8Mask)
{
    /* Wait max 100 ms */
    unsigned int timeout = 100;

    mshc->SW_RST |= u8Mask;
    while (mshc->SW_RST & u8Mask)
    {
        if (timeout == 0)
        {
            kprintf("SD Reset fail \r\n");
            return;
        }
        timeout--;
        DelayMicrosecond(1000);
    }
}

void mshc_set_power(MSHC_REG_T *mshc, uint32_t u32OnOff)
{
    if (u32OnOff)
    {
        /* Power on VDD1 */
        mshc->S_PWR_CTRL.SD_BUS_PWR_VDD1 = 1;

        /* Set 3.3v for EMMC, SD and */
        mshc->S_PWR_CTRL.SD_BUS_VOL_VDD1 = 7;
    }
    else
    {
        /* Power off VDD1 */
        mshc->S_PWR_CTRL.SD_BUS_PWR_VDD1 = 0;

        /* Set 0v for EMMC, SD and */
        mshc->S_PWR_CTRL.SD_BUS_VOL_VDD1 = 0;
    }
}

uint32_t mshc_set_clock(MSHC_REG_T *mshc, uint32_t u32SrcFreqInHz, uint32_t u32ExceptedFreqInHz)
{
    uint32_t timeout;
    uint32_t div;

    if (u32ExceptedFreqInHz == 0)
        goto exit_MSHC_SetClock;

    /* Wait max 20 ms */
    timeout = 200;
    while (mshc->PSTATE & 0x3)   //(MSHC_CMD_INHIBIT | MSHC_DATA_INHIBIT))
    {
        if (timeout == 0)
        {
            kprintf("Timeout to wait cmd & data inhibit \r\n");
            goto exit_MSHC_SetClock;
        }
        timeout--;
        DelayMicrosecond(100);
    }

//    /* Shutdown clocks. */
//    mshc->CLK_CTRL = 0;
//    DelayMicrosecond(1000);

    div = (u32SrcFreqInHz / 2) / u32ExceptedFreqInHz;
    if (div > 0)
    {
        while ((u32SrcFreqInHz / (2 * div)) > u32ExceptedFreqInHz)
        {
            div++;
        }
    }

    mshc->S_CLK_CTRL.FREQ_SEL = div & 0xff;
	
    //kprintf("FREQ_SEL  %d \r\n",mshc->S_CLK_CTRL.FREQ_SEL );

	
    mshc->S_CLK_CTRL.UPPER_FREQ_SEL = (div >> 8) & 0x3;

    mshc->S_CLK_CTRL.INTERNAL_CLK_EN = 1;

    /* Wait stable */
    /* Wait max 20 ms */
    timeout = 200;
    while (!mshc->S_CLK_CTRL.INTERNAL_CLK_STABLE)
    {
        if (timeout == 0)
        {
            kprintf("Timeout to wait CLK stable.\n");
            goto exit_MSHC_SetClock;
        }
        timeout--;
        DelayMicrosecond(100);
    }


    mshc->S_CLK_CTRL.PLL_ENABLE = 1;

    timeout = 200;
    while (!mshc->S_CLK_CTRL.INTERNAL_CLK_STABLE)
    {
        if (timeout == 0)
        {
            kprintf("Timeout to wait CLK stable.\n");
            goto exit_MSHC_SetClock;
        }
        timeout--;
        DelayMicrosecond(100);
    }


    /* Enable SD CLK */
    mshc->S_CLK_CTRL.SD_CLK_EN = 1;

    return (div == 0) ? u32SrcFreqInHz : u32SrcFreqInHz / (2 * div);

exit_MSHC_SetClock:

    mshc->CLK_CTRL = 0;

    return 0;
}








static uint32_t mshc_get_cmd_resptype(uint32_t resp_type)
{
    uint32_t resptype = 0;
    switch (resp_type)
    {
    case RESP_NONE:
        resptype = MMC_RSP_NONE;
        break;
    case RESP_R1:
        resptype = MMC_RSP_R1;
        break;
    case RESP_R1B:
        resptype = MMC_RSP_R1b;
        break;
    case RESP_R2:
        resptype = MMC_RSP_R2;
        break;
    case RESP_R3:
        resptype = MMC_RSP_R3;
        break;
    case RESP_R4:
        resptype = MMC_RSP_R4;
        break;
    case RESP_R6:
        resptype = MMC_RSP_R6;
        break;
    case RESP_R7:
        resptype = MMC_RSP_R7;
        break;
    case RESP_R5:
        resptype = MMC_RSP_R5;
        break;
    default:
        resptype = 0xffffffff;
    }
    return resptype ;
}

static void mshc_send_commanddone(MSHC_REG_T *mshc, struct mmc_cmd *cmd)
{
    if (cmd->resp_type & MMC_RSP_136)
    {
        /* CRC is stripped so we need to do some shifting. */
        cmd->response[0] = (mshc->RESP67 << 8) | mshc->S_RESP45.B3;
        cmd->response[1] = (mshc->RESP45 << 8) | mshc->S_RESP23.B3;
        cmd->response[2] = (mshc->RESP23 << 8) | mshc->S_RESP01.B3;
        cmd->response[3] = (mshc->RESP01 << 8);
    }
    else
    {
        cmd->response[0] = mshc->RESP01;
        cmd->response[1] = cmd->response[2] = cmd->response[3] = 0;

    }
#if 0
	kprintf("mshc->RESP01 %x \r\n", mshc->RESP01);
	kprintf("mshc->RESP23 %x \r\n", mshc->RESP23);
	kprintf("mshc->RESP45 %x \r\n", mshc->RESP45);
	kprintf("mshc->RESP67 %x \r\n", mshc->RESP67);
#endif
}

static int mshc_xfer_data(MSHC_REG_T *mshc, struct mmc_data *data)
{
    uint32_t staaddr, timeout;

    if (data->flags & DATA_DIR_READ)
    {
        staaddr = (uint32_t)data->dest;
    }
    else
    {
        staaddr = (uint32_t)data->src;
    }

    timeout = 50000; // 0.5s

    while (!mshc->S_NORMAL_INT_STAT.XFER_COMPLETE) /* SDHCI_INT_DATA_END? */
    {
        if (mshc->S_NORMAL_INT_STAT.ERR_INTERRUPT == 1)
            return -1;

        if (mshc->S_NORMAL_INT_STAT.DMA_INTERRUPT)        /* SDHCI_INT_DMA_END */
        {
            mshc->S_NORMAL_INT_STAT.DMA_INTERRUPT = 1;    /* Clear SDHCI_INT_DMA_END */

            staaddr &= ~(MSHC_BLOCK_SIZE * 1024 - 1);
            staaddr += MSHC_BLOCK_SIZE * 1024;

            mshc->SDMASA = staaddr;
        }
        if (timeout-- > 0)
            DelayMicrosecond(10);
        else
            return -2;
    }

    return 0;
}

static void mshc_list_errors(MSHC_REG_T *mshc)
{
    if (mshc->S_NORMAL_INT_STAT.ERR_INTERRUPT)
    {
        kprintf("Error List:");
        if (mshc->S_ERROR_INT_STAT.CMD_TOUT_ERR)
            kprintf("\tCMD_TOUT_ERR.");
        if (mshc->S_ERROR_INT_STAT.CMD_CRC_ERR)
            kprintf("\tCMD_CRC_ERR.");
        if (mshc->S_ERROR_INT_STAT.CMD_END_BIT_ERR)
            kprintf("\tCMD_END_BIT_ERR.");
        if (mshc->S_ERROR_INT_STAT.CMD_IDX_ERR)
            kprintf("\tCMD_IDX_ERR.");
        if (mshc->S_ERROR_INT_STAT.DATA_TOUT_ERR)
            kprintf("\tDATA_TOUT_ERR.");
        if (mshc->S_ERROR_INT_STAT.DATA_CRC_ERR)
            kprintf("\tDATA_CRC_ERR.");
        if (mshc->S_ERROR_INT_STAT.DATA_END_BIT_ERR)
            kprintf("\tDATA_END_BIT_ERR.");
        if (mshc->S_ERROR_INT_STAT.CUR_LMT_ERR)
            kprintf("\tCUR_LMT_ERR.");

        if (mshc->S_ERROR_INT_STAT.AUTO_CMD_ERR)
            kprintf("\tAUTO_CMD_ERR.");
        if (mshc->S_ERROR_INT_STAT.ADMA_ERR)
            kprintf("\tADMA_ERR.");
        if (mshc->S_ERROR_INT_STAT.TUNING_ERR)
            kprintf("\tTUNING_ERR.");
        if (mshc->S_ERROR_INT_STAT.RESP_ERR)
            kprintf("\tRESP_ERR.");
        if (mshc->S_ERROR_INT_STAT.BOOT_ACK_ERR)
            kprintf("\tBOOT_ACK_ERR.");
        if (mshc->S_ERROR_INT_STAT.VENDOR_ERR1)
            kprintf("\tVENDOR_ERR1.");
        if (mshc->S_ERROR_INT_STAT.VENDOR_ERR2)
            kprintf("\tVENDOR_ERR2.");
        if (mshc->S_ERROR_INT_STAT.VENDOR_ERR3)
            kprintf("\tVENDOR_ERR3.");
    }
}



int mshc_error_recover(MSHC_REG_T *mshc)
{
    int ret;
    uint32_t mask;
	struct mmcsd_cmd stop;
	volatile int wait_datline_i = 0;

	stop.cmd_code = STOP_TRANSMISSION;
	stop.arg = 0;
	stop.flags = RESP_SPI_R1B | RESP_R1B | CMD_AC;

	// 发送 CMD12 命令
    mshc->ARGUMENT = stop.arg;
    mshc->CMD = ((stop.cmd_code & 0xff) << 8) | (stop.flags & 0xff);

	mask = 0x3; /* MSHC_CMD_INHIBIT | MSHC_DATA_INHIBIT */

	ret = mshc_get_bus_status(mshc, mask);
	if (ret)
	{
		kprintf("ERROR: Busy when error recovery ! %d \r\n\r\n", ret);
		return -1;
	}

//	for(wait_datline_i=0; wait_datline_i<30000; wait_datline_i++){};
	DelayMicrosecond(50);

	// check DAT[7]-DAT[4], they should be high after error recovery
	if( ((mshc->PSTATE>>4) & 0xf) != 0xf){
		return 0;
	}
	else{
		// kprintf("DAT7-4 not high after error recover, DAT7-4 = %x ! \r\n", mshc->PSTATE>>4 & 0xf);
		return -1;
	}

    return 0;
}

/**
  * @brief  This function send command.
  * @param  sdio  rthw_sdio
  * @param  pkg   sdio package
  * @retval None
  */
static int mshc_send_command(MSHC_REG_T *mshc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    int ret;
    int err_recover_ret;
    uint32_t mask, flags, mode;
    volatile unsigned int time = 0;
    volatile unsigned int cmd_timeout, stat;

    DEBUG("[CMD:%d ARG:0x%08x] RESP_TYPE:0x%08x rw:%c addr:0x%08x len:%d blksize:%d\r\n",
          cmd->cmdidx,
          cmd->cmdarg,
          cmd->resp_type,
          data ? (data->flags & DATA_DIR_WRITE ?  'w' : 'r') : '-',
          data ? data->src : 0,
          data ? data->blocks * data->blocksize : 0,
          data ? data->blocksize : 0);

    mask = 0x3; /* MSHC_CMD_INHIBIT | MSHC_DATA_INHIBIT */
    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
        mask &= ~0x2;   /* MSHC_DATA_INHIBIT */

    ret = mshc_get_bus_status(mshc, mask);
    if (ret)
    {
        kprintf("ERROR: Busy %d \r\n", ret);
        ret = __LINE__;
        goto exit_mshc_send_command;
    }

    /* SDHCI_INT_ALL_MASK */
    mshc->NORMAL_INT_STAT = 0xFFFF;
    mshc->ERROR_INT_STAT = 0xFFFF;

    mask = 0x1;     /* SDHCI_INT_RESPONSE */

    if (!(cmd->resp_type & MMC_RSP_PRESENT))
        flags = MSHC_CMD_RESP_NONE;
    else if (cmd->resp_type & MMC_RSP_136)
        flags = MSHC_CMD_RESP_LONG;
    else if (cmd->resp_type & MMC_RSP_BUSY)
    {
        flags = MSHC_CMD_RESP_SHORT_BUSY;
        if (data)
            mask |= 0x2;    /* SDHCI_INT_DATA_END */
    }
    else
        flags = MSHC_CMD_RESP_SHORT;

    if (cmd->resp_type & MMC_RSP_CRC)
        flags |= MSHC_CMD_CRC;

    if (cmd->resp_type & MMC_RSP_OPCODE)
        flags |= MSHC_CMD_INDEX;

    /* Set Transfer mode regarding to data flag */
    if (data)
    {
        flags |= MSHC_CMD_DATA;

        mshc->S_TOUT_CTRL.TOUT_CNT = 0xE;

        mode = 0x2; /* SDHCI_TRNS_BLK_CNT_EN */
        if (data->blocks > 1)
            mode |= 0x20;   /* SDHCI_TRNS_MULTI */

        if (data->flags & DATA_DIR_READ)
        {
            mode |= 0x10;   /* SDHCI_TRNS_READ */
            mshc->SDMASA = (uint32_t)data->dest;
        }
        else
        {
            mshc->SDMASA = (uint32_t)data->src;
        }
		
		//dma enbale
        mode |= 0x1;
        mshc->S_HOST_CTRL1.DMA_SEL = 0; //SDMA is selected

        /* 512 Kbytes SDMA Buffer Boundary */
        mshc->S_BLOCKSIZE.SDMA_BUF_BDARY = 0x7;

        /* Set Block Size */
        mshc->S_BLOCKSIZE.XFER_BLOCK_SIZE = data->blocksize;

        /* Set Block count */
        mshc->S_BLOCKCOUNT.BLOCK_CNT = data->blocks;

        /* Set transfer mode */
        mshc->XFER_MODE = mode;
		
    }
    else if (cmd->resp_type & MMC_RSP_BUSY)
    {
        mshc->S_TOUT_CTRL.TOUT_CNT = 0xE;
    }
	// 发送命令
    mshc->ARGUMENT = cmd->cmdarg;
    mshc->CMD = ((cmd->cmdidx & 0xff) << 8) | (flags & 0xff);

	cmd_timeout = 10000000;
    time = 0;
    do
    {
        stat = mshc->NORMAL_INT_STAT;
        if (stat & 0x8000) /* SDHCI_INT_ERROR */
            break;

        if (time > cmd_timeout)
        {
            ret = __LINE__;
			kprintf("cmdidx %2d  ARGUMENT %x	CMDREG %X	flags %x \r\n",cmd->cmdidx,mshc->ARGUMENT,mshc->CMD,flags);
            kprintf("[%s]  timeout stat=%04x, mask=%04x\r\n", __func__, stat, mask);
            goto exit_mshc_send_command;
        }
        time++;
    }
    while ((stat & mask) != mask);

    if ((stat & (0x8000 | mask)) == mask)
    {
        mshc_send_commanddone(mshc, cmd);
        mshc_list_errors(mshc);

        // cmd->response[0] is card status based on SD protocol reference, significant 8 bit should be low when normal
        if( (cmd->cmdidx==SEND_STATUS || cmd->cmdidx==READ_SINGLE_BLOCK\
        		|| cmd->cmdidx==READ_MULTIPLE_BLOCK || cmd->cmdidx==WRITE_BLOCK\
				|| cmd->cmdidx==WRITE_MULTIPLE_BLOCK) && (cmd->response[0]>>24)&0xff ){
        	goto exit_mshc_send_command;
        }

        /* Send data */
        if (data)
        {
            ret = mshc_xfer_data(mshc, data);
			if(ret){
				kprintf("mshc_xfer_data err %d \r\n", ret);
				goto exit_mshc_send_command;
			}
        }
        stat = mshc->ERROR_INT_STAT;

        mshc->NORMAL_INT_STAT = mask;
        ret = 0;
    }
    else
    {
        //kprintf("[%s %d] Error. cmdid=%d not restored %08x %08x", __func__, __LINE__, cmd->cmdidx, stat, mask);
        ret = __LINE__;

        mshc_list_errors(mshc);
		
        goto exit_mshc_send_command;
    }

    /* SDHCI_INT_ALL_MASK */
    mshc->NORMAL_INT_STAT = 0xFFFF;
    mshc->ERROR_INT_STAT = 0xFFFF;

    if (ret)
    {
        kprintf("[%s] ret=%d cmd->cmdidx=%d, error=0x%x\r\n", __func__, ret, cmd->cmdidx, stat);
        ret = __LINE__;
        goto exit_mshc_send_command;
    }

    return 0;

exit_mshc_send_command:

    mshc_reset(mshc, MSHC_RESET_CMD);
    mshc_reset(mshc, MSHC_RESET_DATA);

    struct sd_recover *recover = &g_sd_recover;

    if(recover->times < ERR_RECOVER_MAX){

    	recover->times++;
		err_recover_ret = mshc_error_recover(mshc);

		if(err_recover_ret==0){
			kprintf("Error recover success !\r\n");
		}else{
			kprintf("Error recover may fail !\r\n");
		}

		if( cmd->cmdidx==READ_SINGLE_BLOCK || cmd->cmdidx==READ_MULTIPLE_BLOCK ){
			// if error happens on reading cmd, trigger reread
			ks_os_sem_post(recover->sem_sd_rd_recover);
		}else{
			// if not, trigger rewrite
			ks_os_sem_post(recover->sem_sd_wt_recover);
		}
    }
    else{
    	kprintf("Rewrite or Reread Fails !!! \r\n");
    }

    kprintf("[%s] cmdid=%d error line=%d \r\n", __func__, cmd->cmdidx, ret);

    return ret;
}


/**
  * @brief  This function send sdio request.
  * @param  host  mmcsd_host
  * @param  req   request
  * @retval None
  */
static void mshc_request(struct mmcsd_host *host, struct mmcsd_req *req)
{
    struct mshc * mshc = (struct mshc *)host->private_data;

    ks_exception_assert(host);
    ks_exception_assert(req);

    if (host->card)
    {
        if (host->card->card_type == CARD_TYPE_MMC)
        {
            mshc->base->S_EMMC_CTRL.CARD_IS_EMMC = 1;
            mshc->base->S_EMMC_CTRL.DISABLE_DATA_CRC_CHK = 1;
        }
        else
        {
            mshc->base->S_EMMC_CTRL.CARD_IS_EMMC = 0;
            mshc->base->S_EMMC_CTRL.DISABLE_DATA_CRC_CHK = 0;
        }
    }

    if (req->cmd != NULL)
    {
        struct mmc_cmd cmd;

        DEBUG("[%s%s%s]REQ: CMD:%d ARG:0x%08x RESP_TYPE:0x%08x, 0x%08x\r\n",
              (host->card == NULL) ? "Unknown" : "",
              (host->card) && (host->card->card_type == CARD_TYPE_MMC) ? "MMC" : "",
              (host->card) && (host->card->card_type == CARD_TYPE_SD) ? "SD" : "",
              req->cmd->cmd_code,
              req->cmd->arg,
              resp_type(req->cmd),
              mshc_get_cmd_resptype(resp_type(req->cmd)));

        memset(&cmd, 0, sizeof(struct mmc_cmd));

        cmd.cmdidx    = req->cmd->cmd_code;
        cmd.cmdarg    = req->cmd->arg;
        cmd.resp_type = mshc_get_cmd_resptype(resp_type(req->cmd));

        if (req->data != NULL)
        {
            struct mmc_data data;
            uint32_t size;
            char *buf_virtual = NULL;


            DEBUG("[%s]REQ: BUF:%08x FLAGS:0x%08x BLKSIZE:%d, BLKCOUNT:%d\r\n",
                  mshc->name,
                  req->data->buf,
                  req->data->flags,
                  req->data->blksize,
                  req->data->blks);

            memset(&data, 0, sizeof(struct mmc_data));

            data.dest = (char *) req->data->buf;
            data.flags = req->data->flags;
            data.blocksize = req->data->blksize;
            data.blocks = req->data->blks;

            size = data.blocksize * data.blocks;

            ks_exception_assert(size <= MSHC_BUFF_SIZE);

         	//copy data to ocmem 
            buf_virtual = (char *) mshc->cachebuf_virtual;
            data.dest = (char *) mshc->cachebuf_phy;
            if (data.flags & DATA_DIR_WRITE)
            {
                memcpy((void *)buf_virtual, req->data->buf, size);
				ks_cpu_dcache_clean((void *)buf_virtual, size);
            }


            req->cmd->err = mshc_send_command(mshc->base, &cmd, &data);


            if (!req->cmd->err)
            {
                if (data.flags & DATA_DIR_READ)
                {
                    ks_cpu_dcache_invalidate(buf_virtual, size);
                    memcpy(req->data->buf,buf_virtual, size);
                }
            }


        }
        else
        {
            req->cmd->err = mshc_send_command(mshc->base, &cmd, NULL);
        }

        /* Report response words */
        req->cmd->resp[3] = cmd.response[3];
        req->cmd->resp[2] = cmd.response[2];
        req->cmd->resp[1] = cmd.response[1];
        req->cmd->resp[0] = cmd.response[0];

        //kprintf("%x %x %x %x \r\n",req->cmd->resp[0],req->cmd->resp[1],req->cmd->resp[2],req->cmd->resp[3]);

    }

    if (req->stop != NULL)
    {
        struct mmc_cmd stop;
        memset(&stop, 0, sizeof(struct mmc_cmd));

        stop.cmdidx = req->stop->cmd_code;
        stop.cmdarg = req->stop->arg;
        stop.resp_type = mshc_get_cmd_resptype(resp_type(req->stop));

        req->stop->err = mshc_send_command(mshc->base, &stop, NULL);

        /* Report response words */
        req->stop->resp[3] = stop.response[3];
        req->stop->resp[2] = stop.response[2];
        req->stop->resp[1] = stop.response[1];
        req->stop->resp[0] = stop.response[0];

        //kprintf("%x %x %x %x \r\n",req->stop->resp[0],req->stop->resp[1],req->stop->resp[2],req->stop->resp[3]);

    }

    mmcsd_req_complete(host);
}

/**
  * @brief  This function config sdio.
  * @param  host    mmcsd_host
  * @param  io_cfg  mmcsd_io_cfg
  * @retval None
  */
static void mshc_iocfg(struct mmcsd_host *host, struct mmcsd_io_cfg *io_cfg)
{
    struct mshc * NuSdh;
    uint32_t clk = io_cfg->clock;
    MSHC_REG_T *mshc;

    ks_exception_assert(host);
    ks_exception_assert(io_cfg);

    NuSdh = (struct mshc *)host->private_data;
    mshc = NuSdh->base;

    DEBUG("[%s]clk:%d width:%s%s%s power:%s%s%s\r\n",
          NuSdh->name,
          clk,
          io_cfg->bus_width == MMCSD_BUS_WIDTH_8 ? "8" : "",
          io_cfg->bus_width == MMCSD_BUS_WIDTH_4 ? "4" : "",
          io_cfg->bus_width == MMCSD_BUS_WIDTH_1 ? "1" : "",
          io_cfg->power_mode == MMCSD_POWER_OFF ? "OFF" : "",
          io_cfg->power_mode == MMCSD_POWER_UP ? "UP" : "",
          io_cfg->power_mode == MMCSD_POWER_ON ? "ON" : "");

    /* Bus width */
    switch (io_cfg->bus_width)
    {
    case MMCSD_BUS_WIDTH_1:
    case MMCSD_BUS_WIDTH_4:
    case MMCSD_BUS_WIDTH_8:
        mshc_set_bus_width(mshc,  1 << io_cfg->bus_width);
        break;
    default:
        break;
    }

    /* Power */
    switch (io_cfg->power_mode)
    {
    case MMCSD_POWER_UP:
    case MMCSD_POWER_ON:
        mshc_set_power(mshc, 1);
        break;

    case MMCSD_POWER_OFF:
        mshc_set_power(mshc, 0);
        break;
    default:
        break;
    }
	//kprintf(" clk: %d kHz, freq_max %d kHz  freq_min %d kHz \r\n", clk / 1000,host->freq_max/ 1000,host->freq_min/ 1000);


    /* Clock */
    if (clk > host->freq_max)
        clk = host->freq_max;


    if (clk < host->freq_min)
        clk = host->freq_min;

    if (clk)
    {
        uint32_t u32SrcFreqInHz = 0, u32ModRealFreqInHz;
  
		u32SrcFreqInHz = ks_os_get_ahb_clock();

		//when set clk 50M ahb_clock 208M real caculate div(4) freq 34.6 ,so in order to increed speed div(3) clk set 52M 
		if(clk == 50000000){
			clk = 52000000;
		}

        u32ModRealFreqInHz = mshc_set_clock(mshc, u32SrcFreqInHz, clk);
        //u32ModRealFreqInHz = u32ModRealFreqInHz; //Avoid warning

        //kprintf("[%s] SrcClock: %d kHz, ExceptedFreq: %d kHz, RealFreq: %d kHz \r\n", NuSdh->name, u32SrcFreqInHz / 1000, clk / 1000, u32ModRealFreqInHz / 1000);
    }

}

int mshc_init_sd_detect_pin(U8 gpio_id)
{

	if(s_sd_detect_gpio <0 || s_sd_detect_gpio > GPIOA_MAX){
		return -1;
	}

	//gpio 不支持双极性触发，改成定时查询
	ks_driver_gpio_init(gpio_id);
	ks_driver_gpio_set_dir(gpio_id,GPIO_INPUT);
	//sd　未插入　高电平
	ks_driver_gpio_set_pull_dir(gpio_id,GPIO_PULL_UP);
	ks_driver_gpio_set_pull_enable(gpio_id,1);


#if 0
	//设置中断触发模式　触发极性
	//ks_driver_gpio_interrupt_config(gpio_id,TRIGGER_TYPE_EDGE,1);
	ks_driver_gpio_interrupt_bothedge(gpio_id,1);

	ks_driver_gpio_irq_create(mmcsd_gpio_isr_handler,(void*)mshc);
	ks_driver_gpio_interrupt_clear(gpio_id);
	//解除屏蔽中断
	ks_driver_gpio_interrupt_mask(gpio_id,0);
	ks_driver_gpio_interrupt_enable(gpio_id, 1);
	ks_driver_gpio_irq_enable();

	//ks_os_irq_create(mshc->irqn, mshc_isr, (void*)mshc);
	//ks_os_irq_enable(mshc->irqn);
	//ks_os_irq_map_target(mshc->irqn, 1);
#endif 
	return 0;

}

int mshc_read_sd_detect_pin()
{
	if(s_sd_detect_gpio <0 || s_sd_detect_gpio > GPIOA_MAX)
	{
		return 1;
	}else{
		uint32_t status = ks_driver_gpio_status();
		if(status&(1<<SDCARD_DETECT_PIN)){
			return 0;
		}else{
			return 1;
		}
	}
}

/**
  * @brief  This function detect sdcard.
  * @param  host    mmcsd_host
  * @retval 0x01
  */
//卡槽通过gpio	   通过电平检测sd卡是否插入
int mshc_card_detect(struct mmcsd_host *host)
{

	if(host==NULL) return -1;

	if(host->card_type == CARD_TYPE_SD)
	{
		return mshc_read_sd_detect_pin();
	 }else{
		return 1;
	 }
}

#if 0
/**
  * @brief  This function interrupt process function.
  * @param  host  mmcsd_host
  * @retval None
  */
static void mshc_isr(int vector, void *param)
{
    struct mshc * mshc = (struct mshc *)param;
    struct mmcsd_host *host = mshc->host;
    MSHC_REG_T *base = mshc->base;
    volatile unsigned int isr = base->NORMAL_INT_STAT;

    /* We just catch card detection here. */
    if (isr & 0xc0)
    {
        /* ready to change */
        mmcsd_change(host);
        base->NORMAL_INT_STAT = 0xC0;
    }
}
#endif

void mmcsd_gpio_isr_handler(void* arg)
{														 
	U32 status;
	U8 gpio;

 	struct mshc * mshc = (struct mshc *)arg;
    struct mmcsd_host *host = mshc->host;
	//读取中断状态以便确定是哪个GPIO发生中断
	status = ks_driver_gpio_interrupt_status();
	
	//先屏蔽对应的中断
	ks_driver_gpio_interrupt_mask_set(status);


	ks_driver_gpio_interrupt_clear(SDCARD_DETECT_PIN);
	mmcsd_status_change();


	ks_driver_gpio_interrupt_mask_set(0);

}

/**
  * @brief  This function update mshc interrupt.
  * @param  host    mmcsd_host
  * @param  enable
  * @retval None
  */
void mshc_irq_update(struct mmcsd_host *host, int32_t enable)
{
    struct mshc * mshc = (struct mshc *)host->private_data;
    MSHC_REG_T *mshc_base = mshc->base;

    if (enable)
    {
        //kprintf("Enable %s irq\r\n", mshc->name);

        /* Enable only interrupts served by the SD controller */
        /* mshc_base->NORMAL_INT_STAT_EN = 0x00FB; */
        mshc_base->S_NORMAL_INT_STAT_EN.CMD_COMPLETE_STAT_EN = 1;
        mshc_base->S_NORMAL_INT_STAT_EN.XFER_COMPLETE_STAT_EN = 1;
        mshc_base->S_NORMAL_INT_STAT_EN.DMA_INTERRUPT_STAT_EN = 1;

        mshc_base->S_NORMAL_INT_STAT_EN.BUF_WR_READY_STAT_EN = 1;
        mshc_base->S_NORMAL_INT_STAT_EN.BUF_RD_READY_STAT_EN = 1;
        mshc_base->S_NORMAL_INT_STAT_EN.CARD_INSERTION_STAT_EN = 1;
        mshc_base->S_NORMAL_INT_STAT_EN.CARD_REMOVAL_STAT_EN = 1;

        /* mshc_base->ERROR_INT_STAT_EN = 0x0271; */
        mshc_base->S_ERROR_INT_STAT_EN.CMD_TOUT_ERR_STAT_EN = 1;
        mshc_base->S_ERROR_INT_STAT_EN.DATA_TOUT_ERR_STAT_EN = 1;
        mshc_base->S_ERROR_INT_STAT_EN.DATA_CRC_ERR_STAT_EN = 1;
        mshc_base->S_ERROR_INT_STAT_EN.DATA_END_BIT_ERR_STAT_EN = 1;
        mshc_base->S_ERROR_INT_STAT_EN.ADMA_ERR_STAT_EN = 1;

        /* Mask all interrupt sources */
        /* mshc_base->NORMAL_INT_SIGNAL_EN = 0xC0; */
        mshc_base->S_NORMAL_INT_SIGNAL_EN.CARD_INSERTION_SIGNAL_EN = 1;
        mshc_base->S_NORMAL_INT_SIGNAL_EN.CARD_REMOVAL_SIGNAL_EN = 1;

        mshc_base->ERROR_INT_SIGNAL_EN = 0;

        //mshc_base->NORMAL_INT_STAT_EN = 0x7FFF;
        //mshc_base->ERROR_INT_STAT_EN = 0xFFFF;
        //mshc_base->NORMAL_INT_SIGNAL_EN=0x7FFF;
        //mshc_base->ERROR_INT_SIGNAL_EN=0xFFFF;
    }
    else
    {
       // kprintf("Disable %s irq\r\n", mshc->name);

        mshc_base->NORMAL_INT_STAT_EN = 0x0;
        mshc_base->ERROR_INT_STAT_EN = 0x0;
        mshc_base->NORMAL_INT_SIGNAL_EN = 0x0;
        mshc_base->ERROR_INT_SIGNAL_EN = 0x0;
    }
}

static const struct mmcsd_host_ops mshc_ops =
{
    mshc_request,
    mshc_iocfg,
    mshc_card_detect,
    mshc_irq_update,
};

/**
  * @brief  This function create mmcsd host.
  * @param  mshc  struct mshc *
  * @retval nuvton
  */
int mshc_host_initial(int card_type,struct mmcsd_host **host_out)
{
    struct mmcsd_host *host;
    int32_t ret = EOK;
	struct mshc * mshc = & g_mshc_ctx;
	mshc->base = ( MSHC_REG_T*) EMMC_BASE;
	
    mshc->cachebuf_phy = (uint8_t*)CACHE_BUF_ADDR_PHY;
    mshc->cachebuf_virtual = (uint8_t*)CACHE_BUF_ADDR;
	//mshc->cachebuf = malloc(MSHC_BUFF_SIZE);
	
	//mshc->cachebuf_phy = (uint8_t*)g_nocache_buffer;
    //mshc->cachebuf_virtual = (uint8_t*)g_nocache_buffer;
	
	if(card_type == CARD_TYPE_MMC)
	{
		mshc->name = "emmc";
	}
	else if(card_type == CARD_TYPE_SD)
	{
		mshc->name = "sdcard";

	}
	else{
		ks_exception_assert(0);
	}


    host = mmcsd_alloc_host();
    ks_exception_assert(host != NULL);
	
	host->card_type = card_type;

    /* Reset mshc at first. */
    mshc_reset(mshc->base, MSHC_RESET_ALL);

    /* set host default attributes */
    host->ops = &mshc_ops;

    host->freq_min = 400 * 1000;
	host->freq_max = 26 * 1000 * 1000;
//	host->freq_max = 40 * 1000 * 1000;

    host->valid_ocr = VDD_30_31 | VDD_31_32 | VDD_32_33 | VDD_33_34 |VDD_165_195;
    //host->valid_ocr = VDD_30_31 | VDD_31_32 | VDD_32_33 | VDD_33_34; // | VDD_165_195;

    host->flags = MMCSD_BUSWIDTH_4 | MMCSD_MUTBLKWRITE | MMCSD_SUP_SDIO_IRQ | MMCSD_SUP_HIGHSPEED;

    host->max_seg_size = MSHC_BUFF_SIZE;
    host->max_dma_segs = 1;
    host->max_blk_size = MSHC_BLOCK_SIZE;
    host->max_blk_count = (MSHC_MAX_MUTI_BLOCK_COUNT);

    /* link up host and sdio */
    host->private_data = mshc;
    mshc->host = host;

    /* Set initial state: high speed */
    mshc->base->S_HOST_CTRL1.HIGH_SPEED_EN = 1;

    /* Set SDR50 mode */
    mshc->base->S_HOST_CTRL2.UHS_MODE_SEL = 2;

	//uint32_t MBIU_CTRL_R = mshc->base->S_P_VENDOR_SPECIFIC_AREA.REG_OFFSET_ADDR + 0x10;

	//kprintf("MBIU_CTRL_R %x ", read_reg(MBIU_CTRL_R));	
	//write_reg(MBIU_CTRL_R,2);
	
    /* Install ISR. only sd card detect */
	//card detect pin
	//s_sd_detect_gpio =  SDCARD_DETECT_PIN;
	// <0 不判断sd_detect_gpio　表示sd 卡插入状态
	s_sd_detect_gpio =	-1;
	mshc_init_sd_detect_pin(s_sd_detect_gpio);

    /* Enable interrupt. */
    mshc_irq_update(host, 1);
	
	*host_out = host;
	
	mmcsd_status_change();
	
	return 0;
}




/* Print out register information:  Name, Offset, Current value, Reset value, match flag. */
#define DUMP_REG(BASE, NAME, DEFAULT)          kprintf("| %-24s | 0x%04x | 0x%08x | 0x%08x | %-6s |\r\n", #NAME, (uint32_t)&BASE->NAME - (uint32_t)BASE,  BASE->NAME, DEFAULT, ((BASE->NAME != DEFAULT) ? " N ":" Y ") )
#define PACK_MMC_CMD(CMD, IDX, ARG, RESP)  ( CMD.cmdidx=IDX,  CMD.cmdarg=ARG, CMD.resp_type=RESP )



void mshc_dump_reg(MSHC_REG_T *mshc)
{

    kprintf("========================================================================\r\n");
    kprintf("MSHC_REG_T(0x%08x): %d\n", mshc, sizeof(MSHC_REG_T));
    kprintf("========================================================================\r\n");
    kprintf("| %-24s | %-6s | %-10s | %-10s | %-6s |\r\n", "REG NAME", "OFFSET", "CURRENT", "DEFAULT", "MATCH?");
    kprintf("========================================================================\r\n");
    DUMP_REG(mshc, SDMASA, 0x0);
    DUMP_REG(mshc, BLOCKSIZE, 0x0);
    DUMP_REG(mshc, BLOCKCOUNT, 0x0);
    DUMP_REG(mshc, ARGUMENT, 0x0);
    DUMP_REG(mshc, XFER_MODE, 0x0);
    DUMP_REG(mshc, CMD, 0x0);
    DUMP_REG(mshc, RESP01, 0x0);
    DUMP_REG(mshc, RESP23, 0x0);
    DUMP_REG(mshc, RESP45, 0x0);
    DUMP_REG(mshc, RESP67, 0x0);
    DUMP_REG(mshc, BUF_DATA, 0x0);
    DUMP_REG(mshc, PSTATE, 0x0);
    DUMP_REG(mshc, HOST_CTRL1, 0x0);
    DUMP_REG(mshc, PWR_CTRL, 0x0);

    DUMP_REG(mshc, BGAP_CTRL, 0x0);
    DUMP_REG(mshc, WUP_CTRL, 0x0);
    DUMP_REG(mshc, CLK_CTRL, 0x0);
    DUMP_REG(mshc, TOUT_CTRL, 0x0);
    DUMP_REG(mshc, SW_RST, 0x0);
    DUMP_REG(mshc, NORMAL_INT_STAT, 0x0);
    DUMP_REG(mshc, ERROR_INT_STAT, 0x0);
    DUMP_REG(mshc, NORMAL_INT_STAT_EN, 0x0);
    DUMP_REG(mshc, ERROR_INT_STAT_EN, 0x0);
    DUMP_REG(mshc, NORMAL_INT_SIGNAL_EN, 0x0);

    DUMP_REG(mshc, ERROR_INT_SIGNAL_EN, 0x0);
    DUMP_REG(mshc, AUTO_CMD_STAT, 0x0);
    DUMP_REG(mshc, HOST_CTRL2, 0x0);
    DUMP_REG(mshc, CAPABILITIES1, 0x276EC898);
    DUMP_REG(mshc, CAPABILITIES2, 0x08008077);
    DUMP_REG(mshc, CURR_CAPABILITIES1, 0x0);
    DUMP_REG(mshc, CURR_CAPABILITIES2, 0x0);
    DUMP_REG(mshc, FORCE_AUTO_CMD_STAT, 0x0);
    DUMP_REG(mshc, FORCE_ERROR_INT_STAT, 0x0);
    DUMP_REG(mshc, ADMA_ERR_STAT, 0x0);
    DUMP_REG(mshc, ADMA_SA_LOW, 0x0);
    DUMP_REG(mshc, PRESET_INIT, 0x0);
    DUMP_REG(mshc, PRESET_DS, 0x0);
    DUMP_REG(mshc, PRESET_HS, 0x0);
    DUMP_REG(mshc, PRESET_SDR12, 0x0);
    DUMP_REG(mshc, PRESET_SDR25, 0x0);
    DUMP_REG(mshc, PRESET_SDR50, 0x0);
    DUMP_REG(mshc, PRESET_SDR104, 0x0);
    DUMP_REG(mshc, PRESET_DDR50, 0x0);
    DUMP_REG(mshc, PRESET_UHS2, 0x0);
    DUMP_REG(mshc, P_EMBEDDED_CNTRL, 0x0F6C);
    DUMP_REG(mshc, P_VENDOR_SPECIFIC_AREA, 0x0500);
    DUMP_REG(mshc, P_VENDOR2_SPECIFIC_AREA, 0x0180);
    DUMP_REG(mshc, SLOT_INTR_STATUS, 0x0000);
    DUMP_REG(mshc, HOST_CNTRL_VERS, 0x0005);
    DUMP_REG(mshc, EMBEDDED_CTRL, 0x00000000);
    DUMP_REG(mshc, MSHC_VER_ID, 0x3138302A);
    DUMP_REG(mshc, MSHC_VER_TYPE, 0x67612A2A);
    DUMP_REG(mshc, MSHC_CTRL, 0x01);
    DUMP_REG(mshc, MBIU_CTRL, 0x0F);
    DUMP_REG(mshc, EMMC_CTRL, 0x000C);
    DUMP_REG(mshc, BOOT_CTRL, 0x0000);

    DUMP_REG(mshc, AT_CTRL, 0x03000005);
    DUMP_REG(mshc, AT_STAT, 0x00000006);
    DUMP_REG(mshc, CQCAP,   0x000030C8);
    kprintf("========================================================================\r\n");
}



#define MSHC_CMD_MAX_TIMEOUT         3200
#define MSHC_CMD_DEFAULT_TIMEOUT     100
#define MSHC_MAX_DIV_SPEC_300        2046

int mshc_get_bus_status(MSHC_REG_T *mshc, uint32_t mask)
{
    volatile unsigned int time = 0;
    volatile unsigned int cmd_timeout = MSHC_CMD_DEFAULT_TIMEOUT;

    while (mshc->PSTATE & mask)
    {
        if (time >= cmd_timeout)
        {
            if (2 * cmd_timeout <= MSHC_CMD_MAX_TIMEOUT)
            {
                cmd_timeout += cmd_timeout;
            }
            else
            {
                return -1;
            }
        }
        DelayMicrosecond(1000);
        time++;
    }
    return 0;
}

int mshc_set_bus_width(MSHC_REG_T *mshc, uint32_t u32BusWidth)
{
    switch (u32BusWidth)
    {
    case 1:
        mshc->S_HOST_CTRL1.DAT_XFER_WIDTH = 0;
        mshc->S_HOST_CTRL1.EXT_DAT_XFER = 0;
        break;
    case 4:
        mshc->S_HOST_CTRL1.DAT_XFER_WIDTH = 1;
        mshc->S_HOST_CTRL1.EXT_DAT_XFER = 0;
        break;
    case 8:
        mshc->S_HOST_CTRL1.DAT_XFER_WIDTH = 1;
        mshc->S_HOST_CTRL1.EXT_DAT_XFER = 1;
        break;
    }
    return 0;
}


void mshc_regdump(void)
{

     mshc_dump_reg(g_mshc_ctx.base);
    
}


int mmcsd_card_detect(struct mmcsd_host *host)
{

	return mshc_card_detect(host);

}


int mmcsd_low_level_init(int card_type,struct mmcsd_host **host_out)
{
   return  mshc_host_initial(card_type, host_out);
}





/*open the mmcsd port mux */
void mmcsd_port_Init()
{
	U32 temp_value;


	ks_driver_sysctrl_set_mux_sel(GPIOA_0,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_1,MUX_V_FUN1);

	ks_driver_sysctrl_set_mux_sel(GPIOA_2,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_3,MUX_V_FUN1);

	ks_driver_sysctrl_set_mux_sel(GPIOA_4,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_5,MUX_V_FUN1);
	

	ks_driver_sysctrl_set_clock_enable(EMMC_CLK_EN,1);


}



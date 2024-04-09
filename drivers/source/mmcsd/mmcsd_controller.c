#include <string.h>
#include <ks_os.h>
#include <ks_printf.h>
#include "ks_mmcsd.h"
#include "mmcsd_controller.h"
#include "sd.h"
#include "mmc.h"
#include "mmcsd_low_level_driver.h"
#include "dlist.h"
#include "ks_shell.h"
#include "ks_taskdef.h"
#include "ks_cache.h"



#define MMCSD_STACK_SIZE 2048
#define SECTOR_SIZE                512


static OSHandle mmcsd_status_thread;
static uint8_t mmcsd_status_stack[MMCSD_STACK_SIZE];


int init_flag_mmcsd = 0;

struct list_node  g_MMCSDStatusChangeCallBackList;	

OSHandle  g_MMCSDTimer; 
OSHandle g_MMCSDStatusFlag;

struct mmcsd_host * g_mmcsd_host;

typedef struct  MMCSDStatusListCb
{
	struct list_node  cb_list;
	MMCSDStatusCbk cbfunc;
	int is_used;
}MMCSDStatusListCb;

MMCSDStatusListCb s_mmcsd_statuscb[4];

static uint32_t allocated_host_num = 0;

extern sd_recover_t g_sd_recover;


void mmcsd_host_lock(struct mmcsd_host *host)
{
	if(host->bus_lock==0) ks_exception_assert(0);
	
	ks_os_mutex_enter(host->bus_lock);
}

void mmcsd_host_unlock(struct mmcsd_host *host)
{
    ks_os_mutex_leave(host->bus_lock);
}

void mmcsd_req_complete(struct mmcsd_host *host)
{
    ks_os_sem_post(host->sem_ack);
}

void mmcsd_send_request(struct mmcsd_host *host, struct mmcsd_req *req)
{
    do {
        req->cmd->retries--;
        req->cmd->err = 0;
        req->cmd->mrq = req;
        if (req->data)
        {
            req->cmd->data = req->data;
            req->data->err = 0;
            req->data->mrq = req;
            if (req->stop)
            {
                req->data->stop = req->stop;
                req->stop->err = 0;
                req->stop->mrq = req;
            }
        }
        host->ops->request(host, req);
		
        ks_os_sem_pend(host->sem_ack, KS_OS_WAIT_FOREVER);

    } while(req->cmd->err && (req->cmd->retries > 0));


}

int32_t mmcsd_send_cmd(struct mmcsd_host *host,
                          struct mmcsd_cmd  *cmd,
                          int                   retries)
{
    struct mmcsd_req req;

    memset(&req, 0, sizeof(struct mmcsd_req));
    memset(cmd->resp, 0, sizeof(cmd->resp));
    cmd->retries = retries;

    req.cmd = cmd;
    cmd->data = NULL;

    mmcsd_send_request(host, &req);

    return cmd->err;
}

int32_t mmcsd_go_idle(struct mmcsd_host *host)
{
    int32_t err;
    struct mmcsd_cmd cmd;

    if (!controller_is_spi(host))/* 如果不是采用SPI方式访问SD卡 */
    {
        mmcsd_set_chip_select(host, MMCSD_CS_HIGH);
        ks_os_thread_sleep_msec(1);
    }

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = GO_IDLE_STATE;
    cmd.arg = 0;
    cmd.flags = RESP_SPI_R1 | RESP_NONE | CMD_BC;

    err = mmcsd_send_cmd(host, &cmd, 0);


    ks_os_thread_sleep_msec(1);

    if (!controller_is_spi(host))
    {
        mmcsd_set_chip_select(host, MMCSD_CS_IGNORE);
        ks_os_thread_sleep_msec(1);
    }

    return err;
}

int32_t mmcsd_spi_read_ocr(struct mmcsd_host *host,
                              int32_t            high_capacity,
                              uint32_t          *ocr)
{
    struct mmcsd_cmd cmd;
    int32_t err;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SPI_READ_OCR;
    cmd.arg = high_capacity ? (1 << 30) : 0;
    cmd.flags = RESP_SPI_R3;

    err = mmcsd_send_cmd(host, &cmd, 0);

    *ocr = cmd.resp[1];

    return err;
}

int32_t mmcsd_all_get_cid(struct mmcsd_host *host, uint32_t *cid)
{
    int32_t err;
    struct mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = ALL_SEND_CID;
    cmd.arg = 0;
    cmd.flags = RESP_R2 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 3);
    if (err)
        return err;

    memcpy(cid, cmd.resp, sizeof(uint32_t) * 4);

    return 0;
}

int32_t mmcsd_get_cid(struct mmcsd_host *host, uint32_t *cid)
{
    int32_t err, i;
    struct mmcsd_req req;
    struct mmcsd_cmd cmd;
    struct mmcsd_data data;
    uint32_t *buf = NULL;

    if (!controller_is_spi(host))
    {
        if (!host->card)
            return -ERROR;
        memset(&cmd, 0, sizeof(struct mmcsd_cmd));

        cmd.cmd_code = SEND_CID;
        cmd.arg = host->card->rca << 16;
        cmd.flags = RESP_R2 | CMD_AC;
        err = mmcsd_send_cmd(host, &cmd, 3);
        if (err)
            return err;

        memcpy(cid, cmd.resp, sizeof(uint32_t) * 4);

        return 0;
    }

    buf = (uint32_t *)malloc(16);
    if (!buf)
    {
        kprintf("allocate memory failed!");

        return -1;
    }

    memset(&req, 0, sizeof(struct mmcsd_req));
    memset(&cmd, 0, sizeof(struct mmcsd_cmd));
    memset(&data, 0, sizeof(struct mmcsd_data));

    req.cmd = &cmd;
    req.data = &data;

    cmd.cmd_code = SEND_CID;
    cmd.arg = 0;

    /* NOTE HACK:  the RESP_SPI_R1 is always correct here, but we
     * rely on callers to never use this with "native" calls for reading
     * CSD or CID.  Native versions of those commands use the R2 type,
     * not R1 plus a data block.
     */
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = 16;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = buf;
    /*
     * The spec states that CSR and CID accesses have a timeout
     * of 64 clock cycles.
     */
    data.timeout_ns = 0;
    data.timeout_clks = 64;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err)
    {
        free(buf);

        return -ERROR;
    }

    for (i = 0;i < 4;i++)
        cid[i] = buf[i];
    free(buf);

    return 0;
}


int32_t mmcsd_get_csd(struct mmcsd_card *card, uint32_t *csd)
{
    int32_t err, i;
    struct mmcsd_req req;
    struct mmcsd_cmd cmd;
    struct mmcsd_data data;
    uint32_t *buf = NULL;

    if (!controller_is_spi(card->host))
    {
        memset(&cmd, 0, sizeof(struct mmcsd_cmd));

        cmd.cmd_code = SEND_CSD;
        cmd.arg = card->rca << 16;
        cmd.flags = RESP_R2 | CMD_AC;
        err = mmcsd_send_cmd(card->host, &cmd, 3);
        if (err)
            return err;

        memcpy(csd, cmd.resp, sizeof(uint32_t) * 4);

        return 0;
    }

    buf = (uint32_t*)malloc(16);
    if (!buf)
    {
        kprintf("allocate memory failed!");

        return -1;
    }

    memset(&req, 0, sizeof(struct mmcsd_req));
    memset(&cmd, 0, sizeof(struct mmcsd_cmd));
    memset(&data, 0, sizeof(struct mmcsd_data));

    req.cmd = &cmd;
    req.data = &data;

    cmd.cmd_code = SEND_CSD;
    cmd.arg = 0;

    /* NOTE HACK:  the RESP_SPI_R1 is always correct here, but we
     * rely on callers to never use this with "native" calls for reading
     * CSD or CID.  Native versions of those commands use the R2 type,
     * not R1 plus a data block.
     */
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = 16;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = buf;

    /*
     * The spec states that CSR and CID accesses have a timeout
     * of 64 clock cycles.
     */
    data.timeout_ns = 0;
    data.timeout_clks = 64;

    mmcsd_send_request(card->host, &req);

    if (cmd.err || data.err)
    {
        free(buf);

        return -ERROR;
    }

    for (i = 0;i < 4;i++)
        csd[i] = buf[i];
    free(buf);

    return 0;
}

static int32_t _mmcsd_select_card(struct mmcsd_host *host,
                                     struct mmcsd_card *card)
{
    int32_t err;
    struct mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SELECT_CARD;

    if (card)
    {
        cmd.arg = card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
    }
    else
    {
        cmd.arg = 0;
        cmd.flags = RESP_NONE | CMD_AC;
    }

    err = mmcsd_send_cmd(host, &cmd, 3);
    if (err)
        return err;

    return 0;
}

int32_t mmcsd_select_card(struct mmcsd_card *card)
{
    return _mmcsd_select_card(card->host, card);
}

int32_t mmcsd_deselect_cards(struct mmcsd_card *card)
{
    return _mmcsd_select_card(card->host, NULL);
}

int32_t mmcsd_spi_use_crc(struct mmcsd_host *host, int32_t use_crc)
{
    struct mmcsd_cmd cmd;
    int32_t err;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SPI_CRC_ON_OFF;
    cmd.flags = RESP_SPI_R1;
    cmd.arg = use_crc;

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (!err)
        host->spi_use_crc = use_crc;

    return err;
}

void mmcsd_set_iocfg(struct mmcsd_host *host)
{
    struct mmcsd_io_cfg *io_cfg = &host->io_cfg;
#if 0
    kprintf("clock %uHz busmode %u powermode %u cs %u Vdd %u "
        "width %u \r\n",
         io_cfg->clock, io_cfg->bus_mode,
         io_cfg->power_mode, io_cfg->chip_select, io_cfg->vdd,
         io_cfg->bus_width);
#endif
    host->ops->set_iocfg(host, io_cfg);
}

/*
 * Control chip select pin on a host.
 */
void mmcsd_set_chip_select(struct mmcsd_host *host, int32_t mode)
{
    host->io_cfg.chip_select = mode;
    mmcsd_set_iocfg(host);
}

/*
 * Sets the host clock to the highest possible frequency that
 * is below "hz".
 */
void mmcsd_set_clock(struct mmcsd_host *host, uint32_t clk)
{
    if (clk < host->freq_min)
    {
        kprintf("clock too low!\r\n");
    }

    host->io_cfg.clock = clk;
    mmcsd_set_iocfg(host);
}

/*
 * Change the bus mode (open drain/push-pull) of a host.
 */
void mmcsd_set_bus_mode(struct mmcsd_host *host, uint32_t mode)
{
    host->io_cfg.bus_mode = mode;
    mmcsd_set_iocfg(host);
}

/*
 * Change data bus width of a host.
 */
void mmcsd_set_bus_width(struct mmcsd_host *host, uint32_t width)
{
    host->io_cfg.bus_width = width;
    mmcsd_set_iocfg(host);
}

void mmcsd_set_data_timeout(struct mmcsd_data       *data,
                            const struct mmcsd_card *card)
{
    uint32_t mult;
#if 0
    if (card->card_type == CARD_TYPE_SDIO)
    {
        data->timeout_ns = 1000000000;  /* SDIO card 1s */
        data->timeout_clks = 0;

        return;
    }
#endif
    /*
     * SD cards use a 100 multiplier rather than 10
     */
    mult = (card->card_type == CARD_TYPE_SD) ? 100 : 10;

    /*
     * Scale up the multiplier (and therefore the timeout) by
     * the r2w factor for writes.
     */
    if (data->flags & DATA_DIR_WRITE)
        mult <<= card->csd.r2w_factor;

    data->timeout_ns = card->tacc_ns * mult;
    data->timeout_clks = card->tacc_clks * mult;

    /*
     * SD cards also have an upper limit on the timeout.
     */
    if (card->card_type == CARD_TYPE_SD)
    {
        uint32_t timeout_us, limit_us;

        timeout_us = data->timeout_ns / 1000;
        timeout_us += data->timeout_clks * 1000 /
            (card->host->io_cfg.clock / 1000);

        if (data->flags & DATA_DIR_WRITE)
            /*
             * The limit is really 250 ms, but that is
             * insufficient for some crappy cards.
             */
            limit_us = 300000;
        else
            limit_us = 100000;

        /*
         * SDHC cards always use these fixed values.
         */
        if (timeout_us > limit_us || card->flags & CARD_FLAG_SDHC)
        {
            data->timeout_ns = limit_us * 1000; /* SDHC card fixed 250ms */
            data->timeout_clks = 0;
        }
    }

    if (controller_is_spi(card->host))
    {
        if (data->flags & DATA_DIR_WRITE)
        {
            if (data->timeout_ns < 1000000000)
                data->timeout_ns = 1000000000;  /* 1s */
        }
        else
        {
            if (data->timeout_ns < 100000000)
                data->timeout_ns =  100000000;  /* 100ms */
        }
    }
}


static inline int mmcsd__ffs(unsigned int x)
{
	if (x == 0){
		return 0;
	}

	int num = 1;
	if ((x & 0xffff) == 0) {
		num += 16;
		x >>= 16;
	}
	if ((x & 0xff) == 0) {
		num += 8;
		x >>= 8;
	}
	if ((x & 0xf) == 0) {
		num += 4;
		x >>= 4;
	}
	if ((x & 0x3) == 0) {
		num += 2;
		x >>= 2;
	}
	if ((x & 0x1) == 0){
		num += 1;
	}

return num;
}


/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */

static inline uint32_t mmcsd__fls(uint32_t val)
{
    uint32_t  bit = 32;

    if (!val)
        return 0;
    if (!(val & 0xffff0000u))
    {
        val <<= 16;
        bit -= 16;
    }
    if (!(val & 0xff000000u))
    {
        val <<= 8;
        bit -= 8;
    }
    if (!(val & 0xf0000000u))
    {
        val <<= 4;
        bit -= 4;
    }
    if (!(val & 0xc0000000u))
    {
        val <<= 2;
        bit -= 2;
    }
    if (!(val & 0x80000000u))
    {
        bit -= 1;
    }

    return bit;
}


/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
uint32_t mmcsd_select_voltage(struct mmcsd_host *host, uint32_t ocr)
{
    int bit;


    ocr &= host->valid_ocr;

    bit = mmcsd__ffs(ocr);
    if (bit)
    {
        bit -= 1;

        ocr &= 3 << bit;

        host->io_cfg.vdd = bit;
        mmcsd_set_iocfg(host);
    }
    else
    {
        kprintf("host doesn't support card's voltages!\r\n");
        ocr = 0;
    }

    return ocr;
}

static void mmcsd_power_up(struct mmcsd_host *host)
{
    int bit = mmcsd__fls(host->valid_ocr) - 1;

    host->io_cfg.vdd = bit;
    if (controller_is_spi(host))
    {
        host->io_cfg.chip_select = MMCSD_CS_HIGH;
        host->io_cfg.bus_mode = MMCSD_BUSMODE_PUSHPULL;
    }
    else
    {
        host->io_cfg.chip_select = MMCSD_CS_IGNORE;
        host->io_cfg.bus_mode = MMCSD_BUSMODE_OPENDRAIN;
    }
    host->io_cfg.power_mode = MMCSD_POWER_UP;
    host->io_cfg.bus_width = MMCSD_BUS_WIDTH_1;
    mmcsd_set_iocfg(host);

    /*
     * This delay should be sufficient to allow the power supply
     * to reach the minimum voltage.
     */
    ks_os_thread_sleep_msec(10);

    host->io_cfg.clock = host->freq_min;
    host->io_cfg.power_mode = MMCSD_POWER_ON;
    mmcsd_set_iocfg(host);

    /*
     * This delay must be at least 74 clock sizes, or 1 ms, or the
     * time required to reach a stable voltage.
     */
    ks_os_thread_sleep_msec(10);
}

static void mmcsd_power_off(struct mmcsd_host *host)
{
    host->io_cfg.clock = 0;
    host->io_cfg.vdd = 0;
    if (!controller_is_spi(host))
    {
        host->io_cfg.bus_mode = MMCSD_BUSMODE_OPENDRAIN;
        host->io_cfg.chip_select = MMCSD_CS_IGNORE;
    }
    host->io_cfg.power_mode = MMCSD_POWER_OFF;
    host->io_cfg.bus_width = MMCSD_BUS_WIDTH_1;
    mmcsd_set_iocfg(host);
}




struct mmcsd_host *mmcsd_alloc_host(void)
{
    struct mmcsd_host *host;

    host = malloc(sizeof(struct mmcsd_host));
    if (!host)
    {
        kprintf("alloc host failed \r\n");

        return NULL;
    }

    memset(host, 0, sizeof(struct mmcsd_host));

    host->max_seg_size = 65535;
    host->max_dma_segs = 1;
    host->max_blk_size = 512;
    host->max_blk_count = 4096;  
    host->id = allocated_host_num;
    allocated_host_num++;

	ks_os_mutex_create(&host->bus_lock, "sd_bus_lock");
    
    ks_os_sem_create(&host->sem_ack, "sd_ack", 0);

    return host;
}

void mmcsd_free_host(struct mmcsd_host *host)
{
	ks_os_mutex_delete(host->bus_lock);
	
    ks_os_sem_delete(host->sem_ack);
	
    free(host);
}



void mmcsd_detect_notify(uint32_t card_type,	uint32_t status)
{

	MMCSDStatusListCb * pcallback;
	char *name;

	list_for_every_entry( &g_MMCSDStatusChangeCallBackList, pcallback, MMCSDStatusListCb, cb_list )
	{	
		if(pcallback->is_used&&pcallback->cbfunc!=NULL){
			pcallback->cbfunc(card_type,status);
		}						
	}
}


int mmcsd_sd_detect(       struct mmcsd_host *host)
{
    int32_t  err;
    uint32_t  ocr;
	
	//kprintf("detect SD card \r\n");

	mmcsd_host_lock(host);
	mmcsd_power_up(host);  /* 配置SDIO外设电源控制器，power up, 即卡的时钟开启，同时配置SDIO外设时钟为低速模式 */

	mmcsd_go_idle(host);	/* 发送CMD0指令，使卡进入空闲状态 */

	//发送CMD8命令，主要是电压支持 查询SD卡接口条件 (获取OCR寄存器) 
	//CMD8 是SD卡标准V2.0 版本才有的新命令，所以如果主机有接收到响应，可以判断卡为V2.0 或更高版本SD 卡。
	mmcsd_send_if_cond(host, host->valid_ocr);
	
	err = mmcsd_send_app_op_cond(host, 0, &ocr);
	if (!err)
	{
		 if (init_sd(host, ocr)){
			 mmcsd_power_off(host);
		 }
		 
		host->card_status = MMCSD_HOST_PLUGED;
		// ok 
		mmcsd_detect_notify(host->card_type,host->card_status );
	}
	
	mmcsd_host_unlock(host);
	return 0 ;
}



int mmcsd_emmc_detect(       struct mmcsd_host *host)
{
    int32_t  err;
    uint32_t  ocr;
	//kprintf("detect mmc card \r\n");
	mmcsd_host_lock(host);

	mmcsd_power_up(host);  /* 配置SDIO外设电源控制器，power up, 即卡的时钟开启，同时配置SDIO外设时钟为低速模式 */

	mmcsd_go_idle(host);	/* 发送CMD0指令，使卡进入空闲状态 */

	err = mmc_send_op_cond(host, 0, &ocr);
    if (!err)
    {
        if (init_mmc(host, ocr)){
            mmcsd_power_off(host);
		}
		
		host->card_status = MMCSD_HOST_PLUGED;
		mmcsd_detect_notify(host->card_type,host->card_status);
    }

	mmcsd_host_unlock(host);
	return 0 ;
}

int mmcsd_card_remove(       struct mmcsd_host *host)
{


	mmcsd_host_lock(host);
	free(host->card);
	host->card = NULL;
	mmcsd_host_unlock(host);
	
	//kprintf("removed SD card \r\n");
	host->card_status = MMCSD_HOST_UNPLUGED;
	mmcsd_detect_notify(host->card_type,host->card_status);

	return 0;
}


void mmcsd_status_change(){

	ks_os_flag_set_bits(g_MMCSDStatusFlag, 1, SET_FLAG_MODE_OR);

}

void mmcsd_status_check_timer_callback(void* arg)
{
	ks_os_flag_set_bits(g_MMCSDStatusFlag, 1, SET_FLAG_MODE_OR);
}


void mmcsd_detect(void *param)
{

	int status;
	struct mmcsd_host *host ;
	int card_type;
	card_type= (int )param;

	if(card_type == CARD_TYPE_SD){
		int iret = ks_os_timer_coarse_create(&g_MMCSDTimer,"sd status timer",mmcsd_status_check_timer_callback,NULL, 100, 1000, 1);
		if(iret!= 0 ){
			kprintf("g_MMCSDTimer TimerCreate failuer %d \r\n",iret);
		}
	}

	while (1)
	{
		ks_os_flag_wait(g_MMCSDStatusFlag, 1, WAIT_FLAG_MODE_OR_CLEAR,KS_OS_WAIT_FOREVER);
		// 延时防止误触
		ks_os_thread_sleep_msec(20);

		host = g_mmcsd_host;
		
		if(host==NULL) continue;
		
  		//卡槽通过gpio	   通过电平检测sd卡是否插入
		status = mmcsd_card_detect(host);
		
		if(status == 1 ){
			// card has insert
			 if (host->card == NULL)
			 {
				  if(host->card_type == CARD_TYPE_SD){
					  mmcsd_sd_detect(host);
				  }else if(host->card_type == CARD_TYPE_MMC){
					  mmcsd_emmc_detect(host);
				  }else{
					  kprintf("detect unknow card \r\n");
				  }
			  }
		}else if(status == 0){
		
			if (host->card != NULL)
			{
	     		mmcsd_card_remove(host);
			}
		}else{
			kprintf("status err!! \r\n");
		}

	}

}

int mmcsd_add_change_cb(MMCSDStatusCbk cb_func)
{

	MMCSDStatusListCb* precvcb = NULL;
	int count = sizeof(s_mmcsd_statuscb)/sizeof(s_mmcsd_statuscb[0]);
	for(int i = 0;i<count;i++)
	{
		if(s_mmcsd_statuscb[i].is_used==0)
		{
			precvcb = &s_mmcsd_statuscb[i];
			precvcb->is_used=1;
			break;
		}
	}

	if(precvcb!=NULL){
		precvcb->cbfunc=cb_func;
		list_add_tail( &(g_MMCSDStatusChangeCallBackList), &(precvcb->cb_list) );
		if(g_mmcsd_host!=NULL && g_mmcsd_host->card_status!=0){
			cb_func(g_mmcsd_host->card_type,g_mmcsd_host->card_status);
		}
		return 0 ;
	}else{
		return -1;
	}

}


static int dumpreg(cmd_proc_t* ctx,int argc,  char **argv ){

	ks_shell_printf(ctx->uart_id, "\r\n");
	mshc_regdump();
	return 0;
	
}



static int sdidle(cmd_proc_t* ctx,int argc,  char **argv ){
    struct mmcsd_host *host;
	host = g_mmcsd_host;

	mmcsd_power_up(host);  /* 配置SDIO外设电源控制器，power up, 即卡的时钟开启，同时配置SDIO外设时钟为低速模式 */

	mmcsd_go_idle(host);  /* 发送CMD0指令，使卡进入空闲状态 */
	
	mmcsd_send_if_cond(host, host->valid_ocr);

	return 0;
	
}

static int sdocr(cmd_proc_t* ctx,int argc,  char **argv ){
    struct mmcsd_host *host;
	host = g_mmcsd_host;
    int32_t err;
	uint32_t		   ocr = 0;
	ks_shell_printf(ctx->uart_id, "\r\n");
	/* 发送CMD8指令，判断是否为V2.0或V2.0以上的卡，并获取OCR寄存器值 */
	err = mmcsd_send_if_cond(host, ocr);
	 if (!err) /* 如果是V2.0及以上版本的卡，将置为OCR的bit30位，表明主机支持高容量SDHC卡（OCR将在ACMD41指令时作为参数发送给卡） */
		 ocr |= 1 << 30;

	 err = mmcsd_send_app_op_cond(host, ocr, NULL);/* 发送ACMD41（ACMD41 = CMD55+CMD41）指令，发送主机容量支持信息，并询问卡的操作条件 */

	 return 0;
	 
 }

static int sdcid(cmd_proc_t* ctx,int argc,  char **argv ){
    struct mmcsd_host *host;
	host = g_mmcsd_host;
    int32_t err;
	uint32_t resp[4];


	err = mmcsd_all_get_cid(host, resp);/* 发送CMD2命令，获取CID寄存器值 */

	kprintf( "%x%x",resp[0],resp[1]);
	 return 0;
	 
 }

static int cardinfo(cmd_proc_t* ctx,int argc,  char **argv ){
    struct mmcsd_host *host;
	host = g_mmcsd_host;
	ks_shell_printf(ctx->uart_id, "\r\n");

	if(host->card!=NULL){
		ks_shell_printf(ctx->uart_id, "card_type %d \r\n",host->card->card_type);
		ks_shell_printf(ctx->uart_id, "hs_max_data_rate %d \r\n",host->card->hs_max_data_rate);
		ks_shell_printf(ctx->uart_id, "max_data_rate %d \r\n",host->card->max_data_rate);
		ks_shell_printf(ctx->uart_id, "card_sector_count %d \r\n",host->card->card_sector_count);
		ks_shell_printf(ctx->uart_id, "card_capacity %d \r\n",host->card->card_capacity);
		ks_shell_printf(ctx->uart_id, "card_blksize %d \r\n",host->card->card_blksize);
		ks_shell_printf(ctx->uart_id, "erase_size %d \r\n",host->card->erase_size);


		
		ks_shell_printf(ctx->uart_id, "cid: %x%x%x%x \r\n",host->card->resp_cid[0],host->card->resp_cid[1],host->card->resp_cid[2],host->card->resp_cid[3]);
		ks_shell_printf(ctx->uart_id, "csr: %x%x%x%x \r\n",host->card->resp_csd[0],host->card->resp_csd[1],host->card->resp_csd[2],host->card->resp_csd[3]);
		ks_shell_printf(ctx->uart_id, "scr: %x%x \r\n",host->card->resp_scr[0],host->card->resp_scr[1]);

		ks_shell_printf(ctx->uart_id, "scr: %x%x \r\n",host->card->resp_scr[0],host->card->resp_scr[1]);
		

	}

	 return 0;
	 
 }



static int sdcmd(cmd_proc_t* ctx,int argc,  char **argv ){
    struct mmcsd_host *host;
	host = g_mmcsd_host;
    int32_t err;
	uint32_t resp[4];
	int iret;
	uint32_t cmd;
	uint32_t ocr;
	ks_shell_printf(ctx->uart_id, "\r\n");

    if (argc > 1 )
    {
		iret = ks_shell_str2uint(argv[1], &cmd);
		if (iret != 0)
		{
		 	return CMD_ERR_PARAMS_FORMAT;
		}

		if(host==NULL)return CMD_ERR_DEVICE_OP_FAILED;

		if(cmd == 8){
			//发送CMD8命令，主要是电压支持 查询SD卡接口条件 (获取OCR寄存器) 
			//CMD8 是SD卡标准V2.0 版本才有的新命令，所以如果主机有接收到响应，可以判断卡为V2.0 或更高版本SD 卡。
			mmcsd_send_if_cond(host, host->valid_ocr);

		}else if(cmd == 55 ){
			mmcsd_send_app_op_cond(host, 0, &ocr);

		}

    }

	 return 0;
	 
 }

static int sddetect(cmd_proc_t* ctx,int argc,  char **argv ){
    struct mmcsd_host *host;
	host = g_mmcsd_host;

	int iret;

	ks_shell_printf(ctx->uart_id, "\r\n");

	iret = mmcsd_card_detect(host);

	if(iret ){

		ks_shell_printf(ctx->uart_id, "card has insert \r\n");
	}else{
		ks_shell_printf(ctx->uart_id, "card not insert \r\n");
	}

	 return 0;
	 
 }


static int sdchange(cmd_proc_t* ctx,int argc,  char **argv ){
    struct mmcsd_host *host;
	host = g_mmcsd_host;

	int iret;

	ks_shell_printf(ctx->uart_id, "\r\n");
	mmcsd_status_change();

	return 0;
	 
 }



//mmsd	块读写测试
static int mmcsdw(cmd_proc_t* ctx,int argc,	char **argv){

	char buffer[512];	
	int iret;
	uint32_t sector;
	
    for (uint32_t i = 0; i < 512; i++) {
        buffer[i] = i;
    }
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &sector);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	}else{
		sector = 1000 ;
	}

	iret = ks_driver_mmcsd_write(CARD_TYPE_MMC,sector,buffer,1);
	if(iret){

		ks_shell_printf(ctx->uart_id,"mmcsd_write  sector  at  %d \r\n",sector);

		ks_shell_printf_dump_hex(ctx->uart_id,(unsigned char * )buffer,512);
	}else{
		ks_shell_printf(ctx->uart_id,"mmcsd_write  sector  at  %d  erro \r\n",sector);
	}
	
	return 0;
}


static int mmcsdr(cmd_proc_t* ctx,int argc, char **argv){

	char buffer[512];	
	int iret;
	uint32_t sector;

	memset(buffer,0,sizeof(buffer));
	
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &sector);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	}else{
		sector = 1000 ;
	}
	
	iret = ks_driver_mmcsd_read(CARD_TYPE_MMC,sector,buffer,1);

	if(iret){
		
		ks_shell_printf(ctx->uart_id,"mmcsd_read  sector  at  %d \r\n",sector);
		
		ks_shell_printf_dump_hex(ctx->uart_id,(unsigned char* )buffer,512);

	}else{
		ks_shell_printf(ctx->uart_id,"mmcsd_read  sector  at  %d  erro \r\n",sector);

	}

	
	return 0;
	
}



static cmd_proc_t mmcsd_cmds[] = {
    {.cmd = "dumpmmc", .fn = dumpreg,  .help = "dumpmmc"},
	{.cmd = "idle", .fn = sdidle,	.help = "sdidle"},
	{.cmd = "ocr", .fn = sdocr,	.help = "sdocr"},
	{.cmd = "cid", .fn = sdcid, .help = "sdcid"},
	{.cmd = "card", .fn = cardinfo, .help = "cardinfo"},
	{.cmd = "sdcmd", .fn = sdcmd, .help = "sdcmd"},
	{.cmd = "sdd", .fn = sddetect, .help = "sddetect"},
	{.cmd = "sdc", .fn = sdchange, .help = "sdchange"},
	{.cmd = "mmcsdw", .fn = mmcsdw, .next = (void *)0, .help = "mmcsd_write"},
	{.cmd = "mmcsdr", .fn = mmcsdr, .next = (void *)0, .help = "mmcsd_read"},

};


int mmcsd_get_info(mmcsd_info* pinfo)
{
	if(init_flag_mmcsd == 0 || g_mmcsd_host==NULL){
		return -1;
	}
	struct mmcsd_host *host = g_mmcsd_host;
	pinfo->card_type  = host->card->card_type;
	pinfo->erase_size  = host->card->erase_size;
	pinfo->bytes_per_sector =host->card->card_blksize ;
	pinfo->sector_count = host->card->card_sector_count;
	return 0;

}


int mmcsd_init(int card_type)
{
	struct sd_recover *recover = &g_sd_recover;
	if(init_flag_mmcsd == 0){

		mmcsd_port_Init();
		
		ks_shell_add_cmds(mmcsd_cmds, sizeof(mmcsd_cmds) / sizeof(cmd_proc_t));
	
		list_initialize(&g_MMCSDStatusChangeCallBackList);

		ks_os_flag_create(&g_MMCSDStatusFlag, "mmcsd status");

		
		ks_os_thread_create(&mmcsd_status_thread, "mmcsd_detect", mmcsd_detect, (void*)card_type,_THREAD_PRI_MMCSD_DETECT,
						  &mmcsd_status_stack[0], MMCSD_STACK_SIZE, 0,1);

	    mmcsd_low_level_init(card_type,&g_mmcsd_host);

	    // sd recover variable initialization
	    recover->times = 0;
		ks_os_sem_create(&recover->sem_sd_rd_recover, "sd_rd_recover", 0);
		ks_os_sem_create(&recover->sem_sd_wt_recover, "sd_wt_recover", 0);

		init_flag_mmcsd = 1;

	}

    return 0;
}


// 块的读写操作　dir:0    读取　1　写操作
//sector:扇区号
//blks:扇区数
static int32_t mmcsd_req_blk(struct mmcsd_card *card,
                                 uint32_t           sector,
                                 void                 *buf,
                                 uint32_t             blks,
                                 uint8_t            dir)
{
    struct mmcsd_cmd  cmd, stop;
    struct mmcsd_data  data;
    struct mmcsd_req  req;
    struct mmcsd_host *host = card->host;
    uint32_t r_cmd, w_cmd;


    mmcsd_host_lock(host);
    memset(&req, 0, sizeof(struct mmcsd_req));
    memset(&cmd, 0, sizeof(struct mmcsd_cmd));
    memset(&stop, 0, sizeof(struct mmcsd_cmd));
    memset(&data, 0, sizeof(struct mmcsd_data));
    req.cmd = &cmd;
    req.data = &data;

    cmd.arg = sector;
    if (!(card->flags & CARD_FLAG_SDHC))
    {
        cmd.arg <<= 9;
    }
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = SECTOR_SIZE;
    data.blks  = blks;

    if (blks > 1)
    {
        if (!controller_is_spi(card->host) || !dir)
        {
            req.stop = &stop;
            stop.cmd_code = STOP_TRANSMISSION;
            stop.arg = 0;
            stop.flags = RESP_SPI_R1B | RESP_R1B | CMD_AC;
        }
        r_cmd = READ_MULTIPLE_BLOCK;
        w_cmd = WRITE_MULTIPLE_BLOCK;
    }
    else
    {
        req.stop = NULL;
        r_cmd = READ_SINGLE_BLOCK;
        w_cmd = WRITE_BLOCK;
    }

    if (!dir)
    {
        cmd.cmd_code = r_cmd;
        data.flags |= DATA_DIR_READ;
    }
    else
    {
        cmd.cmd_code = w_cmd;
        data.flags |= DATA_DIR_WRITE;
    }

    mmcsd_set_data_timeout(&data, card);
    data.buf = buf;
    mmcsd_send_request(host, &req);

	//when write  need  status cmd
    if (!controller_is_spi(card->host) && dir != 0)
    {
        do
        {
            int32_t err;

            cmd.cmd_code = SEND_STATUS;
            cmd.arg = card->rca << 16;
            cmd.flags = RESP_R1 | CMD_AC;
            err = mmcsd_send_cmd(card->host, &cmd, 1);
            if (err)
            {
                kprintf("error %d requesting status", err);
                break;
            }
            /*
             * Some cards mishandle the status bits,
             * so make sure to check both the busy
             * indication and the card state.
             */
         } while (!(cmd.resp[0] & R1_READY_FOR_DATA) ||
            (R1_CURRENT_STATE(cmd.resp[0]) == 7));
    }

    mmcsd_host_unlock(host);

    if (cmd.err || data.err || stop.err)
    {
        kprintf("mmcsd request blocks error");
        kprintf("%d,%d,%d, 0x%08x,0x%08x",
                   cmd.err, data.err, stop.err, data.flags, sector);

        return -ERROR;
    }

	//kprintf("mmcsd_req_blk　sector %d  blks %d  dir%d \r\n", sector,blks,dir);


	struct sd_recover *recover = &g_sd_recover;
	uint32_t sem_cnt = ks_os_sem_peek(recover->sem_sd_wt_recover);

	if(sem_cnt!=0){
		return -ERROR;
	}
    return EOK;
}


uint32_t mmcsd_write(	uint32_t card_type,
                                uint32_t    pos,
                                const void *buffer,
                                uint32_t   block)
{
    int32_t err = 0;
    uint32_t offset = 0;
    uint32_t req_size = 0;
    uint32_t remain_size = block;
    void *wr_ptr = (void *)buffer;

	uint32_t  max_req_block;

	if(g_mmcsd_host==NULL) return 0;
	if(card_type != g_mmcsd_host->card_type) return 0;

	struct mmcsd_card *card = g_mmcsd_host->card;

	max_req_block = card->host->max_blk_count;

	//kprintf("max_req_block %d  \r\n", max_req_block);

    while (remain_size)
    {
        req_size = (remain_size > max_req_block) ? max_req_block : remain_size;
        err = mmcsd_req_blk(card, pos + offset, wr_ptr, req_size, 1);
        if (err)
            break;
        offset += req_size;
        wr_ptr = (void *)((uint8_t *)wr_ptr + (req_size << 9));
        remain_size -= req_size;
    }


    /* the length of reading must align to SECTOR SIZE */
    if (err)
    {
        // set_errno(-err);
        return 0;
    }
    return block - remain_size;
}


uint32_t mmcsd_write_recover(	uint32_t card_type,
                                uint32_t    pos,
                                const void *buffer,
                                uint32_t   block)
{
	U32 ret, sem_cnt, retry;
	struct sd_recover *recover = &g_sd_recover;

	do{
		ret = mmcsd_write(card_type, pos, buffer, block);
		retry = ks_os_sem_peek(recover->sem_sd_wt_recover);

		while ((sem_cnt = ks_os_sem_peek(recover->sem_sd_wt_recover)) > 0){
			ks_os_sem_pend(recover->sem_sd_wt_recover, 0);
		}
	}while(retry);

	recover->times = 0;

	return ret;
}


uint32_t mmcsd_read(uint32_t card_type,
                               uint32_t    pos,
                               void       *buffer,
                               uint32_t   block)
{
    int32_t err = 0;
    uint32_t offset = 0;
    uint32_t req_size = 0;
    uint32_t remain_block = block;
    void *rd_ptr = (void *)buffer;


	if(g_mmcsd_host==NULL) return 0;
	if(card_type != g_mmcsd_host->card_type) return 0;


	struct mmcsd_card *card = g_mmcsd_host->card;

	uint32_t  max_req_block;

	max_req_block = card->host->max_blk_count;

	//kprintf("max_req_block %d  \r\n", max_req_block);

    while (remain_block)
    {
        req_size = (remain_block > max_req_block) ? max_req_block : remain_block;
        err = mmcsd_req_blk(card,  pos + offset, rd_ptr, req_size, 0);
        if (err)
            break;
        offset += req_size;
        rd_ptr = (void *)((uint8_t *)rd_ptr + (req_size << 9));
        remain_block -= req_size;
    }


    /* the length of reading must align to SECTOR SIZE */
    if (err)
    {
        // set_errno(-err);
        return 0;
    }
    return block - remain_block;
}


uint32_t mmcsd_read_recover(uint32_t card_type,
                               uint32_t    pos,
                               void       *buffer,
                               uint32_t   block)
{
	U32 ret, sem_cnt, retry;
	struct sd_recover *recover = &g_sd_recover;

	do{
		ret = mmcsd_read(card_type, pos, buffer, block);
		retry = ks_os_sem_peek(recover->sem_sd_rd_recover);

		while ((sem_cnt = ks_os_sem_peek(recover->sem_sd_rd_recover)) > 0){
			ks_os_sem_pend(recover->sem_sd_rd_recover, 0);
		}
	}while(retry);

	recover->times = 0;

	return ret;
}



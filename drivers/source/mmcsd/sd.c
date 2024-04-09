#include "ks_driver.h"
#include "ks_mmcsd.h"
#include "mmcsd_controller.h"
#include "sd.h"



static const uint32_t tran_unit[] =
{
    10000, 100000, 1000000, 10000000,
    0,     0,      0,       0
};

static const uint8_t tran_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const uint32_t tacc_uint[] =
{
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const uint8_t tacc_value[] =
{
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static inline uint32_t GET_BITS(uint32_t *resp,
                               uint32_t  start,
                               uint32_t  size)
{
        const int32_t __size = size;
        const uint32_t __mask = (__size < 32 ? 1 << __size : 0) - 1;
        const int32_t __off = 3 - ((start) / 32);
        const int32_t __shft = (start) & 31;
        uint32_t __res;

        __res = resp[__off] >> __shft;
        if (__size + __shft > 32)
            __res |= resp[__off-1] << ((32 - __shft) % 32);

        return __res & __mask;
}

static int32_t mmcsd_parse_csd(struct mmcsd_card *card)
{
    struct mmcsd_csd *csd = &card->csd;
    uint32_t *resp = card->resp_csd;

    csd->csd_structure = GET_BITS(resp, 126, 2);

    switch (csd->csd_structure)
    {
    case 0:
        csd->taac = GET_BITS(resp, 112, 8);
        csd->nsac = GET_BITS(resp, 104, 8);
        csd->tran_speed = GET_BITS(resp, 96, 8);
        csd->card_cmd_class = GET_BITS(resp, 84, 12);
        csd->rd_blk_len = GET_BITS(resp, 80, 4);
        csd->rd_blk_part = GET_BITS(resp, 79, 1);
        csd->wr_blk_misalign = GET_BITS(resp, 78, 1);
        csd->rd_blk_misalign = GET_BITS(resp, 77, 1);
        csd->dsr_imp = GET_BITS(resp, 76, 1);
        csd->c_size = GET_BITS(resp, 62, 12);
        csd->c_size_mult = GET_BITS(resp, 47, 3);
        csd->r2w_factor = GET_BITS(resp, 26, 3);
        csd->wr_blk_len = GET_BITS(resp, 22, 4);
        csd->wr_blk_partial = GET_BITS(resp, 21, 1);
        csd->csd_crc = GET_BITS(resp, 1, 7);

        card->card_blksize = 1 << csd->rd_blk_len;
		card->erase_size = 1 << csd->rd_blk_len;
		card->card_sector_count = (csd->c_size + 1) << (csd->c_size_mult + 2);
        //card->card_capacity = (csd->c_size + 1) << (csd->c_size_mult + 2);
        card->card_capacity =card->card_sector_count* card->card_blksize;
        card->card_capacity >>= 10; /* unit:KB */
        card->tacc_clks = csd->nsac * 100;
        card->tacc_ns = (tacc_uint[csd->taac&0x07] * tacc_value[(csd->taac&0x78)>>3] + 9) / 10;
        card->max_data_rate = tran_unit[csd->tran_speed&0x07] * tran_value[(csd->tran_speed&0x78)>>3];

        break;
    case 1:
        card->flags |= CARD_FLAG_SDHC;

        /*This field is fixed to 0Eh, which indicates 1 ms.
          The host should not use TAAC, NSAC, and R2W_FACTOR
          to calculate timeout and should uses fixed timeout
          values for read and write operations*/
        csd->taac = GET_BITS(resp, 112, 8);
        csd->nsac = GET_BITS(resp, 104, 8);
        csd->tran_speed = GET_BITS(resp, 96, 8);
        csd->card_cmd_class = GET_BITS(resp, 84, 12);
        csd->rd_blk_len = GET_BITS(resp, 80, 4);
        csd->rd_blk_part = GET_BITS(resp, 79, 1);
        csd->wr_blk_misalign = GET_BITS(resp, 78, 1);
        csd->rd_blk_misalign = GET_BITS(resp, 77, 1);
        csd->dsr_imp = GET_BITS(resp, 76, 1);
        csd->c_size = GET_BITS(resp, 48, 22);

        csd->r2w_factor = GET_BITS(resp, 26, 3);
        csd->wr_blk_len = GET_BITS(resp, 22, 4);
        csd->wr_blk_partial = GET_BITS(resp, 21, 1);
        csd->csd_crc = GET_BITS(resp, 1, 7);

        card->card_blksize = 512;
        card->card_capacity = (csd->c_size + 1) * 512;  /* unit:KB */
		card->card_sector_count = card->card_capacity * 2;
        card->tacc_clks = 0;
        card->tacc_ns = 0;
		card->erase_size = 512;
        card->max_data_rate = tran_unit[csd->tran_speed&0x07] * tran_value[(csd->tran_speed&0x78)>>3];

        break;
    default:
        kprintf("unrecognised CSD structure version %d!", csd->csd_structure);

        return -ERROR;
    }
    kprintf("SD card card_sector_count %d  capacity %d KB.\r\n", card->card_sector_count ,card->card_capacity);

    return 0;
}

static int32_t mmcsd_parse_scr(struct mmcsd_card *card)
{
    struct sd_scr *scr = &card->scr;
    uint32_t resp[4];

    resp[3] = card->resp_scr[1];
    resp[2] = card->resp_scr[0];
    scr->sd_version = GET_BITS(resp, 56, 4);
    scr->sd_bus_widths = GET_BITS(resp, 48, 4);

    return 0;
}

static int32_t mmcsd_switch(struct mmcsd_card *card)
{
    int32_t err;
    struct mmcsd_host *host = card->host;
    struct mmcsd_req req;
    struct mmcsd_cmd cmd;
    struct mmcsd_data data;
    uint8_t *buf;

    buf = (uint8_t*)malloc(64);
    if (!buf)
    {
        kprintf("alloc memory failed!");

        return -1;
    }

    if (card->card_type != CARD_TYPE_SD)
        goto err;
    if (card->scr.sd_version < SCR_SPEC_VER_1)
        goto err;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SD_SWITCH;
    cmd.arg = 0x00FFFFF1;
    cmd.flags = RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct mmcsd_data));

    mmcsd_set_data_timeout(&data, card);

    data.blksize = 64;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = (uint32_t *)buf;

    memset(&req, 0, sizeof(struct mmcsd_req));

    req.cmd = &cmd;
    req.data = &data;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err)
    {
        goto err1;
    }

    if (buf[13] & 0x02)
        card->hs_max_data_rate = 50000000;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SD_SWITCH;
    cmd.arg = 0x80FFFFF1;
    cmd.flags = RESP_R1 | CMD_ADTC;

    memset(&data, 0, sizeof(struct mmcsd_data));

    mmcsd_set_data_timeout(&data, card);

    data.blksize = 64;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = (uint32_t *)buf;

    memset(&req, 0, sizeof(struct mmcsd_req));

    req.cmd = &cmd;
    req.data = &data;

    mmcsd_send_request(host, &req);

    if (cmd.err || data.err)
    {
        goto err1;
    }

    if ((buf[16] & 0xF) != 1)
    {
        kprintf("switching card to high speed failed!");
        goto err;
    }

    card->flags |= CARD_FLAG_HIGHSPEED;

err:
    free(buf);
    return 0;

err1:
    if (cmd.err)
        err = cmd.err;
    if (data.err)
        err = data.err;

    return err;
}

static int32_t mmcsd_app_cmd(struct mmcsd_host *host,
                              struct mmcsd_card *card)
{
    int32_t err;
    struct mmcsd_cmd cmd = {0};

    cmd.cmd_code = APP_CMD;

    if (card)
    {
        cmd.arg = card->rca << 16;
        cmd.flags = RESP_R1 | CMD_AC;
    }
    else
    {
        cmd.arg = 0;
        cmd.flags = RESP_R1 | CMD_BCR;
    }

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (err){
		kprintf("mmcsd_send_cmd %x \r\n", err);
        return err;

	}

    /* Check that card supported application commands */
    if (!controller_is_spi(host) && !(cmd.resp[0] & R1_APP_CMD)){
		kprintf("!(cmd.resp[0] & R1_APP_CMD  %x \r\n",cmd.resp[0]);
		return -ERROR;
	}


    return EOK;
}


int32_t mmcsd_send_app_cmd(struct mmcsd_host *host,
                            struct mmcsd_card *card,
                            struct mmcsd_cmd  *cmd,
                            int                   retry)
{
    struct mmcsd_req req;
    int i;
    int32_t err;

    err = -ERROR;

    /*
     * We have to resend MMC_APP_CMD for each attempt so
     * we cannot use the retries field in mmc_command.
     */
    for (i = 0; i <= retry; i++)
    {
        memset(&req, 0, sizeof(struct mmcsd_req));

        err = mmcsd_app_cmd(host, card);
        if (err)
        {
            /* no point in retrying; no APP commands allowed */
            if (controller_is_spi(host))
            {
                if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
                    break;
            }
            continue;
        }

        memset(&req, 0, sizeof(struct mmcsd_req));

        memset(cmd->resp, 0, sizeof(cmd->resp));

        req.cmd = cmd;
        //cmd->data = NULL;

        mmcsd_send_request(host, &req);

        err = cmd->err;
        if (!cmd->err){
            break;
        }
        /* no point in retrying illegal APP commands */
        if (controller_is_spi(host))
        {
            if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
                break;
        }
    }

    return err;
}

int32_t mmcsd_app_set_bus_width(struct mmcsd_card *card, int32_t width)
{
    int32_t err;
    struct mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SD_APP_SET_BUS_WIDTH;
    cmd.flags = RESP_R1 | CMD_AC;

    switch (width)
    {
    case MMCSD_BUS_WIDTH_1:
        cmd.arg = MMCSD_BUS_WIDTH_1;
        break;
    case MMCSD_BUS_WIDTH_4:
        cmd.arg = MMCSD_BUS_WIDTH_4;
        break;
    default:
        return -ERROR;
    }

    err = mmcsd_send_app_cmd(card->host, card, &cmd, 3);
    if (err)
        return err;

    return EOK;
}

int32_t mmcsd_send_app_op_cond(struct mmcsd_host *host,
                                uint32_t           ocr,
                                uint32_t          *rocr)
{
    struct mmcsd_cmd cmd;
    uint32_t i;
    int32_t err = EOK;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SD_APP_OP_COND;
    if (controller_is_spi(host))
        cmd.arg = ocr & (1 << 30); /* SPI only defines one bit */
    else
        cmd.arg = ocr;
    cmd.flags = RESP_SPI_R1 | RESP_R3 | CMD_BCR;

    for (i = 100; i; i--)
    {
        err = mmcsd_send_app_cmd(host, NULL, &cmd, 3);
        if (err){
    		kprintf("mmcsd_send_app_cmd　err %x \r\n", err);
            break;
        }


        /* if we're just probing, do a single pass */
        if (ocr == 0)
            break;

        /* otherwise wait until reset completes */
        if (controller_is_spi(host))
        {
            if (!(cmd.resp[0] & R1_SPI_IDLE))
                break;
        }
        else
        {
            if (cmd.resp[0] & CARD_BUSY)
                break;
        }

        err = -ETIMEOUT;

        ks_os_thread_sleep_msec(10); //delay 10ms
    }

    if (rocr && !controller_is_spi(host))
        *rocr = cmd.resp[0];

    return err;
}

/*
 * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
 * before SD_APP_OP_COND. This command will harmlessly fail for
 * SD 1.0 cards.
 */
int32_t mmcsd_send_if_cond(struct mmcsd_host *host, uint32_t ocr)
{
    struct mmcsd_cmd cmd;
    int32_t err;
    uint8_t pattern;

    cmd.cmd_code = SD_SEND_IF_COND;
    cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | 0xAA;
    cmd.flags = RESP_SPI_R7 | RESP_R7 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 0);
    if (err)
        return err;

    if (controller_is_spi(host))
        pattern = cmd.resp[1] & 0xFF;
    else
        pattern = cmd.resp[0] & 0xFF;

    if (pattern != 0xAA)
        return -ERROR;

    return EOK;
}

int32_t mmcsd_get_card_addr(struct mmcsd_host *host, uint32_t *rca)
{
    int32_t err;
    struct mmcsd_cmd cmd;

    memset(&cmd, 0, sizeof(struct mmcsd_cmd));

    cmd.cmd_code = SD_SEND_RELATIVE_ADDR;
    cmd.arg = 0;
    cmd.flags = RESP_R6 | CMD_BCR;

    err = mmcsd_send_cmd(host, &cmd, 3);
    if (err)
        return err;

    *rca = cmd.resp[0] >> 16;

    return EOK;
}

#define be32_to_cpu(x) ((uint32_t)(              \
    (((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |        \
    (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |        \
    (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |        \
    (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

int32_t mmcsd_get_scr(struct mmcsd_card *card, uint32_t *scr)
{
    int32_t err;
    struct mmcsd_req req;
    struct mmcsd_cmd cmd;
    struct mmcsd_data data;

    err = mmcsd_app_cmd(card->host, card);
    if (err)
        return err;

    memset(&req, 0, sizeof(struct mmcsd_req));
    memset(&cmd, 0, sizeof(struct mmcsd_cmd));
    memset(&data, 0, sizeof(struct mmcsd_data));

    req.cmd = &cmd;
    req.data = &data;

    cmd.cmd_code = SD_APP_SEND_SCR;
    cmd.arg = 0;
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_ADTC;

    data.blksize = 8;
    data.blks = 1;
    data.flags = DATA_DIR_READ;
    data.buf = scr;

    mmcsd_set_data_timeout(&data, card);

    mmcsd_send_request(card->host, &req);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    scr[0] = be32_to_cpu(scr[0]);
    scr[1] = be32_to_cpu(scr[1]);

    return 0;
}


static int32_t mmcsd_sd_init_card(struct mmcsd_host *host,
                                     uint32_t           ocr)
{
    struct mmcsd_card *card;
    int32_t err;
    uint32_t resp[4];
    uint32_t max_data_rate;

    mmcsd_go_idle(host);/* 发送CMD0，复位SD卡，使卡进入空闲模式 */

    /*
     * If SD_SEND_IF_COND indicates an SD 2.0
     * compliant card and we should set bit 30
     * of the ocr to indicate that we can handle
     * block-addressed SDHC cards.
     */
    err = mmcsd_send_if_cond(host, ocr);/* 发送CMD8指令，判断是否为V2.0或V2.0以上的卡，并获取OCR寄存器值 */
    if (!err) /* 如果是V2.0及以上版本的卡，将置为OCR的bit30位，表明主机支持高容量SDHC卡（OCR将在ACMD41指令时作为参数发送给卡） */
        ocr |= 1 << 30;

    err = mmcsd_send_app_op_cond(host, ocr, NULL);/* 发送ACMD41（ACMD41 = CMD55+CMD41）指令，发送主机容量支持信息，并询问卡的操作条件 */
    if (err)
        goto err;

    if (controller_is_spi(host))
        err = mmcsd_get_cid(host, resp);
    else
        err = mmcsd_all_get_cid(host, resp);/* 发送CMD2命令，获取CID寄存器值 */
    if (err)
        goto err;

    card = malloc(sizeof(struct mmcsd_card)); /* 创建rt_mmcsd_card结构体，用于存储对应SD卡的CID寄存器内容 */
    if (!card)
    {
        kprintf("malloc card failed!");
        err = -1;
        goto err;
    }
    memset(card, 0, sizeof(struct mmcsd_card));

    card->card_type = CARD_TYPE_SD;
    card->host = host;
    memcpy(card->resp_cid, resp, sizeof(card->resp_cid));

    /*
     * For native busses:  get card RCA and quit open drain mode.
     */
    if (!controller_is_spi(host))
    {
        err = mmcsd_get_card_addr(host, &card->rca); /* 发送CMD3命令，获取RCA地址 */
        if (err)
            goto err1;
		
		/* 设置CMD总线为推挽输出模式，需要注意的是，MMC卡V3.31版本以前的卡，初始化阶段，CMD总线需要为开路模式，对于SD/SD I/O卡和MMC V4.2在初始化时也使用推挽驱动 */
        mmcsd_set_bus_mode(host, MMCSD_BUSMODE_PUSHPULL);
    }

    err = mmcsd_get_csd(card, card->resp_csd); /* 发送CMD9命令，获取CSD寄存器值 */
    if (err)
        goto err1;

    err = mmcsd_parse_csd(card);/* 解析SCR寄存器的值，并将解析结果存放在在card结构体内 */
    if (err)
        goto err1;

    if (!controller_is_spi(host))
    {
        err = mmcsd_select_card(card);
        if (err)
            goto err1;
    }

    err = mmcsd_get_scr(card, card->resp_scr);
    if (err)
        goto err1;

    mmcsd_parse_scr(card);

    if (controller_is_spi(host))
    {
        err = mmcsd_spi_use_crc(host, 1);
        if (err)
            goto err1;
    }

    /*
     * change SD card to high-speed, only SD2.0 spec
     */
    err = mmcsd_switch(card);/* 发送CMD6指令，切换卡访问速率由默认的12.5MB/Sec为25MB/Sec高速接口 */
    if (err)
        goto err1;

    /* set bus speed */
    max_data_rate = (unsigned int)-1;

    if (card->flags & CARD_FLAG_HIGHSPEED)
    {
        if (max_data_rate > card->hs_max_data_rate)
            max_data_rate = card->hs_max_data_rate;
    }
    else if (max_data_rate > card->max_data_rate)
    {
        max_data_rate = card->max_data_rate;
    }


    /*switch bus width*/
    if ((host->flags & MMCSD_BUSWIDTH_4) &&
        (card->scr.sd_bus_widths & SD_SCR_BUS_WIDTH_4)) /* 根据SD卡的SCR寄存器反馈的值，判断SD卡是否支持4线宽度访问模式，如果支持则切换为4线宽度访问模式 */
    {
        err = mmcsd_app_set_bus_width(card, MMCSD_BUS_WIDTH_4); /* 发送ACMD6（ACMD6=CMD55+CMD6）指令，通知SD卡切换为4线访问模式 */
        if (err)
            goto err1;

        mmcsd_set_bus_width(host, MMCSD_BUS_WIDTH_4); /* 修改SDIO外设配置为4线访问模式 */
    }

    mmcsd_set_clock(host, max_data_rate);/* 修改SDIO外设时钟速度 */
    host->card = card;/* 将card结构体数据与host结构体建立绑定关系 */

    return 0;

err1:
    free(card);
err:

    return err;
}

/*
 * Starting point for SD card init.
 */
int32_t init_sd(struct mmcsd_host *host, uint32_t ocr)
{
    int32_t err;
    uint32_t  current_ocr;
    /*
     * We need to get OCR a different way for SPI.
     */
    if (controller_is_spi(host))
    {
        mmcsd_go_idle(host);

        err = mmcsd_spi_read_ocr(host, 0, &ocr);
        if (err)
            goto err;
    }

    if (ocr & VDD_165_195)
    {
        kprintf(" SD card claims to support the "
               "incompletely defined 'low voltage range'. This "
               "will be ignored.\r\n");
        ocr &= ~VDD_165_195;
    }

    current_ocr = mmcsd_select_voltage(host, ocr);

    /*
     * Can we support the voltage(s) of the card(s)?
     */
    if (!current_ocr)
    {
        err = -ERROR;
        goto err;
    }

    /*
     * Detect and init the card.
     */
    err = mmcsd_sd_init_card(host, current_ocr);
    if (err)
        goto err;

    mmcsd_host_unlock(host);

	err = mmcsd_set_blksize(host->card);

    if (err)
        goto remove_card;
    mmcsd_host_lock(host);

    return 0;

remove_card:
    mmcsd_host_lock(host);
    //mmcsd_blk_remove(host->card);
    free(host->card);
    host->card = NULL;
err:

    kprintf("init SD card failed!\r\n");
    err = -1;
    return err;
}

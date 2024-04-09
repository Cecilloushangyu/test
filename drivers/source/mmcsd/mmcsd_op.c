
#include <ks_mmcsd.h>
#include "mmcsd_controller.h"
#include "mmcsd_low_level_driver.h"
#include "ks_driver.h"



#define BLK_MIN(a, b) ((a) < (b) ? (a) : (b))

extern struct mmcsd_host * g_mmcsd_host;
extern sd_recover_t g_sd_recover;


void set_errno(int32_t error){

	kprintf("set_errno %x \r\n",error);
}








int32_t mmcsd_set_blksize(struct mmcsd_card *card)
{
    struct mmcsd_cmd cmd;
    int err;

    /* Block-addressed cards ignore MMC_SET_BLOCKLEN. */
    if (card->flags & CARD_FLAG_SDHC)
        return 0;

    mmcsd_host_lock(card->host);
    cmd.cmd_code = SET_BLOCKLEN;
    cmd.arg = 512;
    cmd.flags = RESP_SPI_R1 | RESP_R1 | CMD_AC;
    err = mmcsd_send_cmd(card->host, &cmd, 1);
    mmcsd_host_unlock(card->host);

    if (err)
    {
        kprintf("MMCSD: unable to set block size to %d: %d", cmd.arg, err);

        return -ERROR;
    }

    return 0;
}



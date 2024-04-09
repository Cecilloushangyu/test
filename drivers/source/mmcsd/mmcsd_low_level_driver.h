
#ifndef __DRV_SDIO_H__
#define __DRV_SDIO_H__
#include <stddef.h>
#include "ks_datatypes.h"
#include "delos_soc_addr.h"
#include "synop_sdhreg.h"
#include "ks_driver.h"






#define  MSHC_POWER_ON            0x01
#define  MSHC_POWER_180           0x0A
#define  MSHC_POWER_300           0x0C
#define  MSHC_POWER_330           0x0E

#define  MSHC_RESET_ALL           0x01
#define  MSHC_RESET_CMD           0x02
#define  MSHC_RESET_DATA          0x04

#define  MSHC_CMD_RESP_MASK       0x03
#define  MSHC_CMD_CRC             0x08
#define  MSHC_CMD_INDEX           0x10
#define  MSHC_CMD_DATA            0x20
#define  MSHC_CMD_ABORTCMD        0xC0

#define  MSHC_CMD_RESP_NONE       0x00
#define  MSHC_CMD_RESP_LONG       0x01
#define  MSHC_CMD_RESP_SHORT      0x02
#define  MSHC_CMD_RESP_SHORT_BUSY 0x03


/* MMC command */
#define MMC_CMD_STOP_TRANSMISSION       12

/* MMC response */
#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136     (1 << 1)        /* 136 bit response */
#define MMC_RSP_CRC     (1 << 2)        /* expect valid crc */
#define MMC_RSP_BUSY    (1 << 3)        /* card may send busy */
#define MMC_RSP_OPCODE  (1 << 4)        /* response contains opcode */

#define MMC_RSP_NONE    (0)
#define MMC_RSP_R1  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
#define MMC_RSP_R2  (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3  (MMC_RSP_PRESENT)
#define MMC_RSP_R4  (MMC_RSP_PRESENT)
#define MMC_RSP_R5  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define MMC_DATA_READ       1
#define MMC_DATA_WRITE      2

#define MSHC_BLOCK_SIZE   512ul
//#define MSHC_MAX_MUTI_BLOCK_COUNT   64
#define MSHC_MAX_MUTI_BLOCK_COUNT   6

struct mmc_cmd
{
    unsigned short cmdidx;
    unsigned int   resp_type;
    unsigned int   cmdarg;
    unsigned int   response[4];
};

struct mmc_data
{
    union
    {
        char *dest;
        const char *src; /* src buffers don't get written to */
    };
    unsigned int flags;
    unsigned int blocks;
    unsigned int blocksize;
};

typedef struct sd_recover
{
	uint32_t times;
	OSHandle sem_sd_rd_recover;
	OSHandle sem_sd_wt_recover;
}sd_recover_t;

#define MSHC_ISCARDINSERTED(SDH)   (SDH->S_PSTATE.CARD_INSERTED && SDH->S_PSTATE.CARD_STABLE)



void mshc_regdump();
void mshc_reset(MSHC_REG_T *sdh, uint8_t u8Mask);
int mshc_set_bus_width(MSHC_REG_T *sdh, uint32_t u32BusWidth);
uint32_t mshc_set_clock(MSHC_REG_T *sdh, uint32_t u32SrcFreqInHz, uint32_t u32ExceptedFreqInHz);
int mshc_get_bus_status(MSHC_REG_T *sdh, uint32_t cmdidx);
void mshc_set_power(MSHC_REG_T *sdh, uint32_t u32OnOff);


void mmcsd_port_Init();
int mmcsd_low_level_init(int type,struct mmcsd_host **host_out);
int mmcsd_card_detect(struct mmcsd_host *host);


#endif

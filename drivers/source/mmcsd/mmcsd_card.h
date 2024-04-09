
#ifndef __MMCSD_CARD_H__
#define __MMCSD_CARD_H__

#include "mmcsd_host.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SD_SCR_BUS_WIDTH_1  (1 << 0)
#define SD_SCR_BUS_WIDTH_4  (1 << 2)

struct mmcsd_cid {
    uint8_t  mid;       /* ManufacturerID */
    uint8_t  prv;       /* Product Revision */
    uint16_t oid;       /* OEM/Application ID */
    uint32_t psn;       /* Product Serial Number */
    uint8_t  pnm[5];    /* Product Name */
    uint8_t  reserved1;/* reserved */
    uint16_t mdt;       /* Manufacturing Date */
    uint8_t  crc;       /* CID CRC */
    uint8_t  reserved2;/* not used, always 1 */
};

struct mmcsd_csd {
    uint8_t      csd_structure;  /* CSD register version */
    uint8_t      taac;
    uint8_t      nsac;
    uint8_t      tran_speed; /* max data transfer rate */
    uint16_t     card_cmd_class; /* card command classes */
    uint8_t      rd_blk_len; /* max read data block length */
    uint8_t      rd_blk_part;
    uint8_t      wr_blk_misalign;
    uint8_t      rd_blk_misalign;
    uint8_t      dsr_imp;    /* DSR implemented */
    uint8_t      c_size_mult;    /* CSD 1.0 , device size multiplier */
    uint32_t     c_size;     /* device size */
    uint8_t      r2w_factor;
    uint8_t      wr_blk_len; /* max wtire data block length */
    uint8_t      wr_blk_partial;
    uint8_t      csd_crc;

};

struct sd_scr {
    uint8_t      sd_version;
    uint8_t      sd_bus_widths;
};


#define SDIO_MAX_FUNCTIONS      7

#define CARD_FLAG_HIGHSPEED  (1 << 0)   /* SDIO bus speed 50MHz */
#define CARD_FLAG_SDHC       (1 << 1)   /* SDHC card */
#define CARD_FLAG_SDXC       (1 << 2)   /* SDXC card */


struct mmcsd_card {
    struct mmcsd_host *host;
    uint32_t rca;        /* card addr */
    uint32_t resp_cid[4];    /* card CID register */
    uint32_t resp_csd[4];    /* card CSD register */
    uint32_t resp_scr[2];    /* card SCR register */

    uint16_t tacc_clks;  /* data access time by ns */
    uint32_t tacc_ns;    /* data access time by clk cycles */
    uint32_t max_data_rate;  /* max data transfer rate */
    uint32_t card_capacity;  /* card capacity, unit:KB */
    uint32_t card_blksize;   /* card block size */
	uint32_t card_sector_count;   /* card sector count */
    uint32_t erase_size; /* erase size in sectors */
    uint16_t card_type;


    uint16_t flags;

    struct sd_scr    scr;
    struct mmcsd_csd csd;
    uint32_t     hs_max_data_rate;  /* max data transfer rate in high speed mode */


};

#ifdef __cplusplus
}
#endif

#endif

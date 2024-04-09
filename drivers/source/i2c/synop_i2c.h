
#ifndef __DW_IIC_H
#define __DW_IIC_H

#include "ks_driver.h"

#ifdef __cplusplus
extern "C" {
#endif



enum i2c_state_e {
    IIC_STATE_NONE = 0,          /* Send start + (first part of) address. */
    IIC_STATE_DATASEND,          /* Send data. */
    IIC_STATE_WFDATA,            /* Wait for data. */
    IIC_STATE_WFSTOPSENT,        /* Wait for STOP to have been transmitted. */
    IIC_STATE_DONE,              /* Transfer completed successfully. */
    IIC_STATE_SEND_DONE,         /* send completed successfully. */
    IIC_STATE_RECV_DONE,         /* receive completed successfully. */
    IIC_STATE_ERROR              /* Transfer error. */
};
/*
 * Define the interrupt type of I2C
 */
#define DW_IIC_INTR_RX_UNDER     0x001
#define DW_IIC_INTR_RX_OVER      0x002
#define DW_IIC_INTR_RX_FULL      0x004
#define DW_IIC_INTR_TX_OVER      0x008
#define DW_IIC_INTR_TX_EMPTY     0x010
#define DW_IIC_INTR_RD_REQ       0x020
#define DW_IIC_INTR_TX_ABRT      0x040
#define DW_IIC_INTR_RX_DONE      0x080
#define DW_IIC_INTR_ACTIVITY     0x100
#define DW_IIC_INTR_STOP_DET     0x200
#define DW_IIC_INTR_START_DET    0x400
#define DW_IIC_INTR_GEN_CALL     0x800

#define DW_IIC_INTR_DEFAULT_MASK     (DW_IIC_INTR_RX_FULL | \
                                      DW_IIC_INTR_TX_EMPTY | \
                                      DW_IIC_INTR_TX_ABRT | \
                                      DW_IIC_INTR_STOP_DET)

/*
 * I2C register bit definitions
 */
#define DW_IIC_DISABLE              0
#define DW_IIC_ENABLE               1
#define DW_IIC_FIFO_MAX_LV          64
#define DW_IIC_TXFIFO_LV            0x0
#define DW_IIC_RXFIFO_LV            0x0

#define DW_IIC_STATUS_SLV_ACTIVITY  (0x1 << 6)
#define DW_IIC_RXFIFO_FULL          (0x1 << 4)
#define DW_IIC_RXFIFO_NOT_EMPTY     (0x1 << 3)
#define DW_IIC_TXFIFO_EMPTY         (0x1 << 2)
#define DW_IIC_TXFIFO_NOT_FULL      (0x1 << 1)
#define DW_IIC_STATUS_ACTIVITY      0x1
#define DW_IIC_FIFO_RST_EN          0x01
/* IC_CON register */
#define DW_IIC_CON_DEFAUL           0x23
#define DW_IIC_CON_SLAVE_ENABLE     (1 << 6)
#define DW_IIC_CON_10BITADDR_SLAVE  (1 << 3)
#define DW_IIC_CON_MASTER_ENABLE    (1 << 0)

//DW_IIC_TX_ABRT_SOURCE_
//
#define DW_IIC_TX_ABRT_SOURCE_TX_FLUSH_CNT_MASK  (0xFF800000U)
#define DW_IIC_TX_ABRT_SOURCE_TX_FLUSH_CNT_SHIFT (23U)
#define DW_IIC_TX_ABRT_SOURCE_TX_FLUSH_CNT(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_TX_FLUSH_CNT_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_TX_FLUSH_CNT_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_RSVD_IC_TX_ABRT_SOURCE_MASK  (0x600000U)
#define DW_IIC_TX_ABRT_SOURCE_RSVD_IC_TX_ABRT_SOURCE_SHIFT (21U)
#define DW_IIC_TX_ABRT_SOURCE_RSVD_IC_TX_ABRT_SOURCE(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_RSVD_IC_TX_ABRT_SOURCE_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_RSVD_IC_TX_ABRT_SOURCE_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_WRITE_MASK  (0x100000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_WRITE_SHIFT (20U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_WRITE(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_WRITE_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_WRITE_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_SLVADDR_NOACK_MASK  (0x80000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_SLVADDR_NOACK_SHIFT (19U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_SLVADDR_NOACK(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_SLVADDR_NOACK_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_SLVADDR_NOACK_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_NOACK_MASK  (0x40000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_NOACK_SHIFT (18U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_NOACK(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_NOACK_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_DEVICE_NOACK_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SDA_STUCK_AT_LOW_MASK  (0x20000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SDA_STUCK_AT_LOW_SHIFT (17U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SDA_STUCK_AT_LOW(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_SDA_STUCK_AT_LOW_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_SDA_STUCK_AT_LOW_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_USER_ABRT_MASK  (0x10000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_USER_ABRT_SHIFT (16U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_USER_ABRT(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_USER_ABRT_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_USER_ABRT_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX_MASK  (0x8000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX_SHIFT (15U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_SLVRD_INTX_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST_MASK  (0x4000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST_SHIFT (14U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_SLV_ARBLOST_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_MASK  (0x2000U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_SHIFT (13U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_SLVFLUSH_TXFIFO_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ARB_LOST_MASK  (0x1000U)
#define DW_IIC_TX_ABRT_SOURCE_ARB_LOST_SHIFT (12U)
#define DW_IIC_TX_ABRT_SOURCE_ARB_LOST(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ARB_LOST_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ARB_LOST_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_MASTER_DIS_MASK  (0x800U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_MASTER_DIS_SHIFT (11U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_MASTER_DIS(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_MASTER_DIS_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_MASTER_DIS_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_MASK  (0x400U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_SHIFT (10U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_MASK  (0x200U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_SHIFT (9U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT_MASK  (0x100U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT_SHIFT (8U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_HS_NORSTRT_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_MASK  (0x80U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_SHIFT (7U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_MASK  (0x40U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_SHIFT (6U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_HS_ACKDET(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_HS_ACKDET_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_READ_MASK  (0x20U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_READ_SHIFT (5U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_READ(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_READ_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_READ_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_MASK  (0x10U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_SHIFT (4U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_MASK  (0x8U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_SHIFT (3U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_MASK  (0x4U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_SHIFT (2U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_MASK  (0x2U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_SHIFT (1U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_MASK  (0x1U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_SHIFT (0U)
#define DW_IIC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK(x)    (((uint32_t)(((uint32_t)(x)) << DW_IIC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_SHIFT)) & DW_IIC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_MASK)
//
#define DW_IIC_TX_ABRT_SOURCE_FLGA_MAS                 (0x1FFFFFU)

/*
 * IIC DMA control value definitions
 */
#define DW_IIC_DMACR_RDMAE_Pos (0U)
#define DW_IIC_DMACR_RDMAE_Msk (0x1U << DW_IIC_DMACR_RDMAE_Pos)
#define DW_IIC_DMACR_TDMAE_Pos (1U)
#define DW_IIC_DMACR_TDMAE_Msk (0x1U << DW_IIC_DMACR_TDMAE_Pos)

/* IIC default value definitions */
#define DW_IIC_TIMEOUT_DEF_VAL  0x1000

typedef struct {
    __IOM uint32_t IC_CON;                    /* Offset: 0x000 (R/W)  I2C Control */
    __IOM uint32_t IC_TAR;                    /* Offset: 0x004 (R/W)  I2C target address */
    __IOM uint32_t IC_SAR;                    /* Offset: 0x008 (R/W)  I2C slave address  */
    __IOM uint32_t IC_HS_MADDR;               /* Offset: 0x00c (R/W)  I2C HS Master Mode Code Address */
    __IOM uint32_t IC_DATA_CMD;               /* Offset: 0x010 (R/W)  I2C RX/TX Data Buffer and Command */
    __IOM uint32_t IC_SS_SCL_HCNT;            /* Offset: 0x014 (R/W)  Standard speed I2C Clock SCL High Count */
    __IOM uint32_t IC_SS_SCL_LCNT;            /* Offset: 0x018 (R/W)  Standard speed I2C Clock SCL Low Count */
    __IOM uint32_t IC_FS_SCL_HCNT;            /* Offset: 0x01c (R/W)  Fast speed I2C Clock SCL High Count */
    __IOM uint32_t IC_FS_SCL_LCNT;            /* Offset: 0x020 (R/W)  Fast speed I2C Clock SCL Low Count */
    __IOM uint32_t IC_HS_SCL_HCNT;            /* Offset: 0x024 (R/W)  High speed I2C Clock SCL High Count*/
    __IOM uint32_t IC_HS_SCL_LCNT;            /* Offset: 0x028 (R/W)  High speed I2C Clock SCL Low Count */
    __IM  uint32_t IC_INTR_STAT;              /* Offset: 0x02c (R)    I2C Interrupt Status */
    __IOM uint32_t IC_INTR_MASK;              /* Offset: 0x030 (R/W)  I2C Interrupt Mask */
    __IM  uint32_t IC_RAW_INTR_STAT;          /* Offset: 0x034 (R)    I2C Raw Interrupt Status */
    __IOM uint32_t IC_RX_TL;                  /* Offset: 0x038 (R/W)  I2C Receive FIFO Threshold */
    __IOM uint32_t IC_TX_TL;                  /* Offset: 0x03c (R/W)  I2C Transmit FIFO Threshold */
    __IM  uint32_t IC_CLR_INTR;               /* Offset: 0x040 (R)    Clear combined and individual interrupts*/
    __IM  uint32_t IC_CLR_RX_UNDER;           /* Offset: 0x044 (R)    I2C Clear RX_UNDER interrupt  */
    __IM  uint32_t IC_CLR_RX_OVER;            /* Offset: 0x048 (R)    I2C Clear RX_OVER interrupt  */
    __IM  uint32_t IC_CLR_TX_OVER;            /* Offset: 0x04c (R)    I2C Clear TX_OVER interrupt  */
    __IM  uint32_t IC_CLR_RD_REQ;             /* Offset: 0x050 (R)    I2C Clear RD_REQ interrupt  */
    __IM  uint32_t IC_CLR_TX_ABRT;            /* Offset: 0x054 (R)    I2C Clear TX_ABRT interrupt  */
    __IM  uint32_t IC_CLR_RX_DONE;            /* Offset: 0x058 (R)    I2C Clear RX_DONE interrupt  */
    __IM  uint32_t IC_CLR_ACTIVITY;           /* Offset: 0x05c (R)    I2C Clear ACTIVITY interrupt  */
    __IM  uint32_t IC_CLR_STOP_DET;           /* Offset: 0x060 (R)    I2C Clear STOP_DET interrupt  */
    __IM  uint32_t IC_CLR_START_DET;          /* Offset: 0x064 (R)    I2C Clear START_DET interrupt  */
    __IM  uint32_t IC_CLR_GEN_CALL;           /* Offset: 0x068 (R)    I2C Clear GEN_CAL interrupt  */
    __IOM uint32_t IC_ENABLE;                 /* Offset: 0x06c (R/W)  I2C enable */
    __IM  uint32_t IC_STATUS;                 /* Offset: 0x070 (R)    I2C status register */
    __IM  uint32_t IC_TXFLR;                  /* Offset: 0x074 (R)    Transmit FIFO Level register */
    __IM  uint32_t IC_RXFLR;                  /* Offset: 0x078 (R)    Receive FIFO Level Register */
    uint32_t RESERVED;                  /* Offset: 0x07c (R/ )   */
    __IOM uint32_t IC_TX_ABRT_SOURCE ;          /* 0x80, I2C Transmit Terminate Source Register */
    __IOM uint32_t IC_SLV_DATA_NACK_ONLY ;     /* 0x84, Generate Target Data NACK Register */
    __IOM uint32_t IC_DMA_CR;                 /* Offset: 0x088 (R/W)  DMA Control Register for transmit and receive handshaking interface  */
    __IOM uint32_t IC_DMA_TDLR;               /* Offset: 0x08c (R/W)  DMA Transmit Data Level */
    __IOM uint32_t IC_DMA_RDLR;               /* Offset: 0x090 (R/W)  DMA Receive Data Level */
    __IOM uint32_t IC_SDA_SETUP;             /* Offset: 0x094 (R/W)  I2C Slave Address2 */
    __IOM uint32_t IC_ACK_GENERAL_CALL; 		/* 0x98, I2C ACK General Call Register */
    __IOM uint32_t IC_ENABLE_STATUS; 			/* 0x9c, I2C Enable Status Register */
} _iic_reg_t;

#ifdef __cplusplus
}
#endif

#endif /* __DW_IIC_H */

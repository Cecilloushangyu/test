#pragma  once

#include <stdint.h>
#include "dlist.h"
#include "queue_array.h"
#include "ks_driver.h"
#include "synop_ethmac.h"




#define DMA_BUF_SIZE 1600
#define DMA_RX_BUFS 16
#define DMA_TX_BUFS 8



#define ETH_ERR_NOT_INIT     (-1)
#define ETH_ERR_NOT_LINK    (-2)
#define ETH_ERR_NO_PKT     (-3)



#define ETH_DUPLEX_MODE (0x1)
#define ETH_SPEED_100M  (0x2)
#define ETH_SPEED_1000M (0x4)



/* Connection setting */

#define ETH_FLAG_INIT_OK      (0x1)          /*if eth init successfully*/
#define ETH_FLAG_STRIP_CRC    (0x2)       
#define ETH_FLAG_EN_IRQ       (0x4)
#define ETH_FLAG_EN_DMA       (0x8)
#define ETH_FLAG_UNKNOWN_PHY  (0x10)
#define ETH_FLAG_AUTO_DELAY   (0x20)


#define PHY_AUTO_NEG        0x1000      /* Select Auto Negotiation           */




#define LAN8720A_PHY_ID1 0x0007
#define LAN8720A_PHY_ID2 0xc0f1

/* Common PHY Registers */

#define PHY_BCR                         ((uint16_t)0x0000)  /*!< Transceiver Basic Control Register   */
#define PHY_BSR                         ((uint16_t)0x0001)  /*!< Transceiver Basic Status Register    */
#define PHY_MSR                         ((uint16_t)0x0011)  /*!< mode   Status Register    */

#define PHY_RESET                       ((uint16_t)0x8000)  /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000)  /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100)  /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000)  /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100)  /*!< Set the full-duplex mode at 10 Mb/s  */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000)  /*!< Set the half-duplex mode at 10 Mb/s  */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000)  /*!< Enable auto-negotiation function     */
#define PHY_RESTAAUTONEGOTIATION     ((uint16_t)0x0200)  /*!< Restart auto-negotiation function    */
#define PHY_POWERDOWN                   ((uint16_t)0x0800)  /*!< Select the power down mode           */
#define PHY_ISOLATE                     ((uint16_t)0x0400)  /*!< Isolate PHY from MII                 */

#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020)  /*!< Auto-Negotiation process completed   */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004)  /*!< Valid link established               */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002)  /*!< Jabber condition detected            */
  

#define in_range(c, lo, up)  ((uint8_t)c >= lo && (uint8_t)c <= up)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define min(t1,t2)          (((t1)<(t2))?(t1):(t2))


enum Mii_Link_Status
{
	LINKDOWN	= 0,
	LINKUP		= 1,
};

enum Mii_Duplex_Mode
{
	HALFDUPLEX = 1,
	FULLDUPLEX = 2,
};

enum Mii_Link_Speed
{
	SPEED10     = 10,
	SPEED100    = 100,
	SPEED1000   = 1000,
	SPEEDAUTO   = 0,
};

enum PHY_Loop_Back
{
	NOLOOPBACK  = 0,
	MIILOOPBACK    = 1,
	PCSLOOPBACK1 = 2, 
	PCSLOOPBACK2 = 3, 
	PCSLOOPBACK3 = 4, 
	DIGLOOPBACK = 5, 
	ANGLOOPBACK = 6,
	EXTLOOPBACK = 7,
	REVLOOPBACK = 8,
};



typedef void (*eth_notify_cb)(int arg);
typedef void * (* mem_alloc)(size_t size, uint32_t align);


typedef void (*eth_data_recvlist_cb)( uint8_t * recvbuffer,int len);

typedef int (*eth_data_txbuffer_cb)( uint8_t * txbuffer_out,int len);




typedef struct DmaDescStruct    
{                               
	volatile uint32_t   status;        /* Status                  */
	volatile uint32_t   length;        /* Buffer 1  and Buffer 2 length             */
	volatile ubase_t   buffer;        /* Network Buffer 1 pointer (Dma-able)               */
	volatile ubase_t   next;          /* Network Buffer 2 pointer or next descriptor pointer (Dma-able)in chain structure  */
  /*!< Enhanced ETHERNET DMA PTP Descriptors */
	volatile uint32_t   ExtendedStatus;		  /*!< Extended status for PTP receive descriptor */
	volatile uint32_t   Reserved1;			  /*!< Reserved */
	volatile uint32_t   TimeStampLow;		  /*!< Time Stamp Low value for transmit and receive */
	volatile uint32_t   TimeStampHigh;		  /*!< Time Stamp High value for transmit and receive */
} DmaDesc;


struct net_device{
	uint32_t flag;
	uint32_t linkflag;
    uint32_t linkmode; 
    uint32_t linkspeed; 
	/**/
	uint32_t phy_addr;
	uint16_t phy_id1;
	uint16_t phy_id2;
	uint16_t rx_delay;
	uint16_t tx_delay;
	uint16_t mtu;
	uint8_t mac[6];

	/*statistics*/
    uint32_t irq_count;
	uint32_t rx_pkts;
	uint32_t tx_pkts;
	uint32_t rx_errors;
	uint32_t tx_errors;   

	/**/
	uint32_t  pbase;
    uint32_t  dmabase;
	uint16_t  wdg;
	uint16_t  irq;
	uint32_t  irq_src;
	uint16_t  rx_dma_num;
	uint16_t  tx_dma_num;
	DmaDesc * host_rxdesc;       /*first rxdesc addr*/
	DmaDesc * host_txdesc;
	DmaDesc * host_rxdesc_cur;   /*current rxdesc addr*/
	DmaDesc * host_txdesc_cur;
	
	/*callback*/
	eth_notify_cb cb_recv_proc;
	eth_notify_cb cb_xmit_proc;
	eth_notify_cb cb_link_proc;
	eth_notify_cb cb_all_proc;

	eth_data_recvlist_cb cb_recvlist; 
	eth_data_txbuffer_cb cb_txbuffer; 


	
	/*phy interface*/
	struct {
		int (*init)(struct net_device *dev);
		int  (*read_reg)(uint32_t phy_addr,uint32_t addr, uint16_t *pret );
		int  (*write_reg)(uint32_t phy_addr,uint32_t addr, uint16_t value );
	}phy_proc;
};

void EthPort_Init();
int EthPhy_UpdateStatus(struct net_device *dev);
int EthPhy_ReadPhyReg( ubase_t RegBase, uint32_t PhyBase, uint32_t RegOffset, uint16_t * data );
int EthPhy_WritePhyReg( ubase_t RegBase, uint32_t PhyBase, uint32_t RegOffset, uint16_t data );

int eth_link_status(struct net_device *netdev);
int eth_isready(struct net_device *netdev);

int eth_tx( struct net_device * dev );
int eth_rx( struct net_device * dev );
int eth_init( struct net_device *pdev);





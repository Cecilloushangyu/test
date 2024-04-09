
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ks_os.h"

#include "eth_controller.h"
#include "eth_low_level_driver.h"
#include "ks_shell.h"
#include "ks_gpio.h"
#include "ks_sysctrl.h"
#include "align_alloc.h"
#include "ks_printf.h"
#include "ks_mem.h"
#include "ks_cache.h"

/*
block : 1024 + 512 = 1536 bytes,  1.5k
mtu : 1518  =  18 (mac + type + crc) + 1500 ( ip + tcp + payload );
*/

// 收发dma 数据　通过切换pkt buffer 
#define  SWITCH_PKT_BUFF 1

__attribute__ ((weak)) void EthNoticeRecv(int flg);
__attribute__ ((weak)) void EthNoticeXmit(int flg);
__attribute__ ((weak)) void EthNoticeLink(int linkflag);


align_alloc_ctx g_dma_bufs_ctx;

//section(".dma_bufs") mean need set  MMU  NoCache 
DmaDesc  g_RXDmaDesc[DMA_RX_BUFS]  __attribute__((section(".dma_bufs"))) __attribute__((aligned(64)));
DmaDesc  g_TXDmaDesc[DMA_TX_BUFS]  __attribute__((section(".dma_bufs"))) __attribute__((aligned(64)));


//normal nocache memory mapping type
//uint8_t g_DmaBuff[(DMA_RX_BUFS+DMA_TX_BUFS)*DMA_BUF_SIZE]  __attribute__((section(".dma_bufs"))) __attribute__((aligned(8)));


#define plat_delay	ks_os_poll_delay_usec


static inline bool desc_owned_by_dma(DmaDesc *desc)
{
 return ((desc->status & DescOwnByDma) == DescOwnByDma) ;
}


static inline void desc_rx_reset_config(DmaDesc *desc,ubase_t lastdesc)
{
	desc->status = DescOwnByDma;
	desc->length = DescRxChain;
	desc->length |= ((DMA_BUF_SIZE<< DescSize1Shift) & DescSize1Mask);
	if( desc->next == lastdesc){
		desc->length |=DescRxEndOfRing;
	}
}

static inline void desc_tx_setup_config(DmaDesc *desc,int len,ubase_t lastdesc)
{
	
#if DMA_Descriptors_Normal
	desc->length = DescTxChain | DescTxFirst | DescTxLast | DescTxIntEnable  |DescTxDisableCrc;
	desc->length |= ((len<< DescSize1Shift) & DescSize1Mask );

	if( desc->next == lastdesc){
		desc->length |=DescTxEndOfRing;
	}

	desc->status = DescOwnByDma;
#else
	desc->length = ((len<< DescSize1Shift) & DescSize1Mask );
	desc->status =DescOwnByDma| DescTxIntEnable|DescTxLast|DescTxFirst|DescTxChain;

	if( desc->next == lastdesc){
		desc->status |=DescTxEndOfRing;
	}
#endif

}

static inline ubase_t desc_get_next(DmaDesc *desc)
{
	return desc->next ;
}

static inline bool desc_is_empty(DmaDesc *desc)
{
	return(((desc->length	& DescSize1Mask) == 0) && ((desc->length  & DescSize2Mask) == 0));
}

static inline void resume_rx_dma(uint32_t DmaBase)
{
	if(DmaBase!=0)
	write_reg( DmaBase + DmaRxPollDemand, 0 );
}

static inline void resume_tx_dma(uint32_t DmaBase)
{
	 if(DmaBase!=0)
	 write_reg( DmaBase + DmaTxPollDemand, 0 );
}

static inline bool is_last_tx_desc(DmaDesc *desc)
{
 return (((desc->length & DescTxEndOfRing) == DescTxEndOfRing));
}

static inline uint32_t tx_desc_len(DmaDesc *desc)
{
 return desc->length&DescSize1Mask;
}


static inline int desc_rx_is_valid(DmaDesc *resc)
{
 return ((resc->status&DescError) == 0) && ((resc->status&DescRxFirst) == DescRxFirst) && 
	 ((resc->status&DescRxLast) == DescRxLast);
}

static inline int desc_rx_is_first(DmaDesc *resc)
{
 return ((resc->status&DescRxFirst) == DescRxFirst);
}

static inline int desc_rx_is_last(DmaDesc *resc)
{
 return ((resc->status&DescRxLast) == DescRxLast);
}


static inline int desc_tx_is_valid(DmaDesc *resc)
{
 return ((resc->status&DescError) == 0);
}

static inline uint32_t desc_data_length(DmaDesc *resc)
{
 return (resc->status & DescFrameLengthMask) >> DescFrameLengthShift;
}



static inline void  EthMacSetBits( ubase_t addr, uint32_t BitPos )
{
  uint32_t data;
	
	data = read_reg( addr );
  data |= BitPos; 
  write_reg( addr, data );
  return;
}


static inline void EthMacClearBits( ubase_t addr, uint32_t BitPos )
{
  uint32_t data;

	data = read_reg( addr );
  data &= (~BitPos); 
  write_reg( addr, data );
  return;
}

static inline bool  EthMacCheckBits( ubase_t addr, uint32_t BitPos)
{
  uint32_t data = read_reg(addr);
  data &= BitPos; 
  if(data)  return true;
  else	    return false;

}


/**
  * Function to read the Phy register. The access to phy register
  * is a slow process as the data is moved accross MDI/MDO interface
  * @param[in] pointer to Register Base (It is the mac base in our case) .
  * @param[in] PhyBase register is the index of one of supported 32 PHY devices.
  * @param[in] Register offset is the index of one of the 32 phy register.
  * @param[out] uint16_t data read from the respective phy register (only valid iff return value is 0).
  * \return Returns 0 on success else return the error status.
  */
int  EthPhy_ReadPhyReg( ubase_t RegBase, uint32_t PhyBase, uint32_t RegOffset, uint16_t * data )
{
  uint32_t addr;
  uint32_t loop_variable;

  /* 
  write the address from where the data to be read in GmiiGmiiAddr register of EthMac ip 
  set Gmii clk to 20-35 Mhz and Gmii busy bit 
  */
  addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask);
  // MDC clock is CSRclock/124
  addr = addr | GmiiCsrClk5 | GmiiBusy;

  
  write_reg( RegBase + GmacGmiiAddr, addr ); 


  for (loop_variable = 0; loop_variable < 1000; loop_variable++)
  {
    //Wait till the busy bit gets cleared with in a certain amount of time
    if ( !(read_reg(RegBase + GmacGmiiAddr) & GmiiBusy) )
    {
      break;
    }
    plat_delay( 10 );
  }
	
  if( loop_variable >= 1000 )
  {
    return 1;
  }
  
  *data = (uint16_t)( read_reg(RegBase + GmacGmiiData) & 0xFFFF );
  return 0;
  
}



/**
  * Function to write to the Phy register. The access to phy register
  * is a slow process as the data is moved accross MDI/MDO interface
  * @param[in] pointer to Register Base (It is the mac base in our case) .
  * @param[in] PhyBase register is the index of one of supported 32 PHY devices.
  * @param[in] Register offset is the index of one of the 32 phy register.
  * @param[in] data to be written to the respective phy register.
  * \return Returns 0 on success else return the error status.
  */
int  EthPhy_WritePhyReg( ubase_t RegBase, uint32_t PhyBase, uint32_t RegOffset, uint16_t data )
{
  uint32_t addr;
  uint32_t loop_variable;

	/**/
  write_reg( RegBase + GmacGmiiData, data ); // write the data in to GmacGmiiData register of EthMac ip

  /* set Gmii clk to 20-35 Mhz and Gmii busy bit */
  addr = ((PhyBase << GmiiDevShift) & GmiiDevMask) | ((RegOffset << GmiiRegShift) & GmiiRegMask) | GmiiWrite;
  //addr = addr | GmiiCsrClk2 | GmiiBusy ;
  addr = addr | ( 5 << 2) | GmiiBusy;
  write_reg( RegBase + GmacGmiiAddr, addr );
	
  for ( loop_variable = 0; loop_variable < 1000; loop_variable++ )
  {
    if ( !(read_reg( RegBase + GmacGmiiAddr) & GmiiBusy) )
    {
      break;
    }
    plat_delay( 10 );
  }
  
  if ( loop_variable < 1000 )
  {
	  return 0;
	}
  else
  {
  	return 1;
  }
  
}



int EthPhy_Reset(struct net_device *dev)
{
	uint16_t temp = 0;
	int i;
	EthPhy_ReadPhyReg( dev->pbase, dev->phy_addr, 0, &temp );
	temp = temp | (0x8000);   
	EthPhy_WritePhyReg( dev->pbase, dev->phy_addr, 0, temp );

	for(i=0;i<10000;i++)
	//for(i=0;i<1000000000000;i++)
	{
		EthPhy_ReadPhyReg( dev->pbase, dev->phy_addr, 0, &temp );
		if((temp & 0x8000) == 0)
		{
			break;
		}
		plat_delay(10);
	}
	if(i == 1000)
		return 1;   /*reset fail*/

	return 0;
}


int EthPhy_UpdateStatus(struct net_device *dev){
	int  iret;
    uint16_t status,temp;
	iret = EthPhy_ReadPhyReg(dev->pbase, dev->phy_addr, PHY_BSR, &status);
	if(iret != 0)
		return 1;

	if (status & (1 << 2)) 
	{
		//kprintf("Connection Succeeded\r\n");
        if(dev->linkflag == LINKDOWN) {
            dev->linkflag = LINKUP;
         	if(dev->cb_link_proc != NULL)
				dev->cb_link_proc(dev->linkflag);
        }
    	temp = 0;
        EthPhy_ReadPhyReg (dev->pbase, dev->phy_addr, 31, &temp);
        if (temp & (1 << 12)) 
	    {
           //kprintf("PHY_AUTONEGO_COMPLETE \r\n");
        }
        
        temp = (temp>>2 & (0x7));
		if (temp == 1) 
		{
			//kprintf("half-duplex 10Mbps  connection\r\n");
            dev->linkmode= HALFDUPLEX;
            dev->linkspeed = SPEED10;
		}
        else if(temp == 2)
		{
			//kprintf("half-duplex 100Mbps  connection\r\n");
            dev->linkmode= HALFDUPLEX;
            dev->linkspeed = SPEED100;
        }
        else if(temp == 5)
        {
			//kprintf("full-duplex 10Mbps  connection\r\n");
            dev->linkmode= FULLDUPLEX;
            dev->linkspeed = SPEED10;
        }
        else if(temp == 6)
        {
			//kprintf("full-duplex 100Mbps  connection\r\n");
            dev->linkmode= FULLDUPLEX;
            dev->linkspeed = SPEED100;
        }

	}
	else
	{
        //kprintf(" Connection failed\r\n");
        if(dev->linkflag == LINKUP ) {
            dev->linkflag = LINKDOWN;
         	if(dev->cb_link_proc != NULL)
				dev->cb_link_proc(dev->linkflag);
            dev->linkmode= 0;
            dev->linkspeed = 0;
        }
	}

    return 0;
    

}


/*any phy device id will lead to init, and return phy init success,
  except can't get phy device id which means mdio-read function don't work
  */
int EthPhy_ScanAndLoop(struct net_device *dev)
{
	int  iret,i;
	uint16_t  id1,id2;

	dev->phy_addr = 0;
	/* identfier register */
	for(i=0; i<32; i++){

		iret = EthPhy_ReadPhyReg(dev->pbase, i, 2, &id1);
		if(iret != 0)
			return 1;

		iret = EthPhy_ReadPhyReg(dev->pbase, i, 3, &id2);
		if(iret != 0)
			return 1;

		if(((id1 == 0xFFFF) && (id2 == 0xFFFF)) || ((id1 == 0) && (id2 == 0)))
		{
			continue;
		}
		else
		{
			dev->phy_addr = i;
			dev->phy_id1 = id1;
			dev->phy_id2 = id2;
            //kprintf( "netdev->phy_id1  %x  netdev->phy_id2 %x  \r\n",dev->phy_id1,dev->phy_id2);

			break;
		}
	}

	if(i == 32)
	{
	    //kprintf("phy id is not recognized.\r\n");
		return 1; //phy id is not recognized.
	}
    
	return 0;
}




int EthDma_SetupTxDesc(struct net_device *dev)
{
	int i;
	DmaDesc *desc;
	DmaDesc *idesc;
	uint32_t first;
	/* desc */
	desc = &g_TXDmaDesc[0];
	if (desc == NULL)
	{
		return 1;
	}
	
	idesc =  (DmaDesc *)((ubase_t)desc);
	memset(idesc, 0, sizeof(DmaDesc) * DMA_TX_BUFS);

	/* set pkt buffer to null */
	for (i = 0; i < DMA_TX_BUFS; i++)
	{

		idesc->buffer = (ubase_t)align_alloc(&g_dma_bufs_ctx,DMA_BUF_SIZE,3);

		idesc->status = DescTxIntEnable|DescTxLast|DescTxFirst|DescTxChain;
		idesc->length = ((DMA_BUF_SIZE << DescSize1Shift) & DescSize1Mask);;



		/* loop back */
		if(i == DMA_TX_BUFS-1){
			idesc->next = (ubase_t)&desc[0];
			idesc->status |= DescTxEndOfRing;
		}else{
			idesc->next = (ubase_t)&desc[i+1];
		}
		
		idesc++;
	
	}
	
	ks_cpu_dcache_clean((void *)desc, sizeof(DmaDesc) * DMA_TX_BUFS);


	/**/
	dev->host_txdesc = &desc[0];
	dev->host_txdesc_cur = &desc[0];
	
	write_reg(dev->dmabase+ DmaTxBaseAddr, dev->host_txdesc);
	
	return 0;
}



int  EthDma_SetupRxDesc(struct net_device *dev)
{
	int  i;
	DmaDesc * desc;
	DmaDesc *idesc;
	uint32_t  first;

	/**/
	desc = &g_RXDmaDesc[0];
	if ( desc == NULL )
	{
		return 1;
	}
	
	idesc = (DmaDesc *)((ubase_t)desc);
	memset(idesc, 0, sizeof(DmaDesc) * dev->rx_dma_num);

	for ( i=0; i<DMA_RX_BUFS; i++ )
	{

		idesc->buffer = (ubase_t)align_alloc(&g_dma_bufs_ctx,DMA_BUF_SIZE,3);
		
		idesc->status = DescOwnByDma;
		idesc->length = DescRxChain;
		idesc->length |= ((DMA_BUF_SIZE << DescSize1Shift) & DescSize1Mask);

		/* loop back */
		if(i == DMA_RX_BUFS-1){
			idesc->next = (ubase_t)&desc[0];
			idesc->length |= DescRxEndOfRing;
		
		}else{
			idesc->next = (ubase_t)&desc[i+1];
		}
		
		idesc++;
	}

	ks_cpu_dcache_clean((void *)desc, sizeof(DmaDesc) * dev->rx_dma_num);

    dev->host_rxdesc_cur = &desc[0];
	dev->host_rxdesc = &desc[0];


    write_reg( dev->dmabase+ DmaRxBaseAddr, dev->host_rxdesc);

    return 0;
  
}


/**
  * Enable the DMA Reception.
  * @param[in] pointer to EthMacdevice.
  * \return returns void.
  */
void EthDma_EnableDmaRx( ubase_t DmaBase )
{
	uint32_t data;

	data = read_reg( DmaBase + DmaOperationMode);
 	data |= DmaRxStart; 
	write_reg( DmaBase + DmaOperationMode, data );
}


/**
  * Enable the DMA Transmission.
  * @param[in] pointer to EthMacdevice.
  * \return returns void.
  */
void EthDma_EnableDmaTx( ubase_t DmaBase )
{
	uint32_t data;
	data = read_reg( DmaBase + DmaOperationMode);
 	data |= DmaTxStart; 
	write_reg( DmaBase + DmaOperationMode, data );
}

void EthDma_DisableDmaRx( ubase_t DmaBase )
{
	uint32_t data;
	data = read_reg( DmaBase + DmaOperationMode);
 	data &= ~DmaRxStart; 
	write_reg( DmaBase + DmaOperationMode, data );
}

void EthDma_DisableDmaTx( ubase_t DmaBase )
{
	uint32_t data;
	data = read_reg( DmaBase + DmaOperationMode);
 	data &= ~DmaTxStart;
	write_reg( DmaBase + DmaOperationMode, data );
}

/**
 *   * GMAC strips the Pad/FCS field of incoming frames.
 *     * This is true only if the length field value is less than or equal to
 *       * 1500 bytes. All received frames with length field greater than or
 *       equal to
 *         * 1501 bytes are passed to the application without stripping the
 *         Pad/FCS field. 
 *           * @param[in] pointer to EthMacdevice.
 *             * \return returns void.
 *               */
void EthMacEnablePadCrcStrip(ubase_t pbase)
{
	EthMacSetBits(pbase+GmacConfig, GmacPadCrcStrip);
	EthMacSetBits(pbase+GmacConfig, GmacCrcStripEnable);
	return;
}
/**
 *   * GMAC doesnot strips the Pad/FCS field of incoming frames.
 *     * GMAC will pass all the incoming frames to Host unmodified. 
 *       * @param[in] pointer to EthMacdevice.
 *         * \return returns void.
 *           */
void EthMacDisablePadCrcStrip(ubase_t pbase)
{
	EthMacClearBits(pbase+GmacConfig, GmacPadCrcStrip);
	EthMacClearBits(pbase+GmacConfig, GmacCrcStripEnable);
	return;
}







void  EthMac_IrqHandler( void * data )
{
	uint32_t status,reg;
	struct net_device *dev = data;

    dev->irq_count++;
   
	status = read_reg(dev->dmabase + DmaStatus);
	//clear
	write_reg(dev->dmabase+DmaStatus, status);

	//kprintf( "EthMac_IrqHandler\r\n" );
	if (status & (DmaIntRxCompleted))
	{
		//DmaDesc* desc_reg = (DmaDesc*)read_reg(dev->dmabase+DmaRxCurrDesc);
		//kprintf("irq rx DmaDesc: %x status %x  \r\n",desc_reg,desc_reg->status);
        if(dev->cb_recv_proc!=NULL){
			dev->cb_recv_proc(status);
		}

	}

    if (status & (DmaIntRxNoBuffer))
	{
        //kprintf( "DmaIntRxNoBuffer %x \r\n",status );
        //resume_rx_dma( dev->dmabase);
		if(dev->cb_recv_proc!=NULL){
			dev->cb_recv_proc(status);
		}
	}
	
	if (status & (DmaIntTxCompleted))
	{
		//DmaDesc* desc_reg = (DmaDesc*)read_reg(dev->dmabase+DmaTxCurrDesc);
		//kprintf("irq tx DmaDesc: %x status %x \r\n",desc_reg,desc_reg->status);
		if(dev->cb_xmit_proc!=NULL){
			dev->cb_xmit_proc(status);
		}
	}

    
    if (status & (DmaIntTxNoBuffer))
	{
        //kprintf( "DmaIntTxNoBuffer %x \r\n",status );
        //resume_tx_dma( dev->dmabase);
	}

}


int  EthBuffs_Init(struct net_device * dev){

	uint32_t size = (DMA_RX_BUFS+DMA_TX_BUFS)*DMA_BUF_SIZE;
	ubase_t DmaBuff = (ubase_t)ks_mem_heap_malloc(size);
	align_init(&g_dma_bufs_ctx,DmaBuff,size);
	//align_init(&g_dma_bufs_ctx,(ubase_t)g_DmaBuff,sizeof(g_DmaBuff));

	return 0;
}

/*open the eth port mux */
void EthPort_Init()
{
	U32 temp_value;

	ks_driver_sysctrl_set_mux_sel(GPIOA_6,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_7,MUX_V_FUN1);

	ks_driver_sysctrl_set_mux_sel(GPIOA_8,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_9,MUX_V_FUN1);

	ks_driver_sysctrl_set_mux_sel(GPIOA_10,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_11,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_12,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_13,MUX_V_FUN1);
	ks_driver_sysctrl_set_mux_sel(GPIOA_14,MUX_V_FUN1);

	ks_driver_sysctrl_set_clock_enable(GMAC_CLK_EN,1);

	
}



void EthCtx_Init(struct net_device * netdev)
{
    netdev->flag= 0;
    netdev->linkflag = 0;
    netdev->pbase = GMAC_BASE;
    netdev->dmabase = GMAC_BASE + 0x1000;
    netdev->mtu = 1518;

    netdev->irq = IRQ_VEC_GMAC;
    netdev->irq_src = (DmaIntNormal | DmaIntAbnormal | DmaIntRxNoBuffer | DmaIntRxCompleted | DmaIntTxCompleted | DmaIntTxNoBuffer);
    netdev->flag = ETH_FLAG_STRIP_CRC | ETH_FLAG_EN_DMA | ETH_FLAG_AUTO_DELAY | ETH_FLAG_EN_IRQ;
    netdev->wdg = DMA_BUF_SIZE;

    netdev->rx_dma_num = DMA_RX_BUFS;
    netdev->tx_dma_num = DMA_TX_BUFS;
    
    netdev->cb_recv_proc = EthNoticeRecv;
    netdev->cb_xmit_proc = EthNoticeXmit;
    netdev->cb_link_proc = EthNoticeLink;


	ks_os_irq_create(netdev->irq, EthMac_IrqHandler, (void*)netdev);
	ks_os_irq_enable(netdev->irq);
	ks_os_irq_map_target(netdev->irq, 1);

	
}





int  EthPhy_Init( struct net_device  * netdev )
{
    int  iret;
    int i = 0;
    uint16_t temp, target_speed;

    // one reset
    iret = EthPhy_Reset(netdev);
    if(iret != 0){
        kprintf("EthPhy_Reset failuer %d \r\n",iret);
        return -1;
    }

    // scan
    iret = EthPhy_ScanAndLoop(netdev);
    if ( 0 != iret )
    {
        kprintf("phy id is not recognized.\r\n");
        return -2;
    }
    // set
    if(netdev->phy_id1 == LAN8720A_PHY_ID1)
    {
        //kprintf("phy id is  LAN8720A\r\n");
    }
    else
    {
      return -3;
    }

    // set speed target, pick one:
    // - PHY_FULLDUPLEX_100M
    // - PHY_HALFDUPLEX_100M
    // - PHY_FULLDUPLEX_10M
    // - PHY_HALFDUPLEX_10M
    // - PHY_AUTONEGOTIATION
    target_speed = PHY_AUTONEGOTIATION;

    if (PHY_AUTONEGOTIATION != target_speed){
        //清除自协商位
        EthPhy_ReadPhyReg(netdev->pbase, netdev->phy_addr, PHY_BCR, &temp);
        temp &= ~(1 << 12);
        EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, PHY_BCR, temp);

        //修改协商广告寄存器，关闭其他能力
        EthPhy_ReadPhyReg(netdev->pbase, netdev->phy_addr, 4, &temp);
        temp &= ~(0x1E0);       // 清除所有速度能力位
        temp |= target_speed;   // PHY_FULLDUPLEX_100M
                                // PHY_HALFDUPLEX_100M
                                // PHY_FULLDUPLEX_10M
                                // PHY_HALFDUPLEX_10M
                                // PHY_AUTONEGOTIATION
        EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, 4, temp);
    }

    //重新设置自协商位
    EthPhy_ReadPhyReg(netdev->pbase, netdev->phy_addr, PHY_BCR, &temp);
    temp |= (1 << 12);
    EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, PHY_BCR, temp);

    //等待自协商结束
    for (i=0; i < 0x1000; i++)
    {
        EthPhy_ReadPhyReg (netdev->pbase, netdev->phy_addr, PHY_BSR, &temp);
        if (temp & 0x0020)
        {
            kprintf("Auto-Negotiation Complete\r\n");
            EthPhy_UpdateStatus(netdev);
            break;
        }
    }

    //enable Link-down interrupt
    EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, 29, 1<<4);

    return 0;
}



int  EthMac_init(struct net_device *netdev )
{
	int ret;
	ubase_t  MacBase;
	ubase_t  dmabase;

	MacBase = netdev->pbase;
	dmabase = netdev->dmabase;
  
	EthMacClearBits(MacBase + GmacConfig, GmacWatchdog);
	EthMacSetBits(MacBase + GmacConfig, GmacJabberDisable);
	EthMacSetBits(MacBase + GmacConfig, GmacFrameBurst);
	EthMacClearBits(MacBase + GmacConfig, GmacJumboFrame);
	EthMacClearBits(MacBase + GmacConfig, GmacRxOwn);
	/*clear loopback*/
	EthMacClearBits(MacBase + GmacConfig, GmacLoopback);

	EthMacSetBits(MacBase + GmacConfig, GmacMiiGmii);	/*port select*/
    EthMacSetBits(MacBase + GmacConfig, GmacFESpeed100); /* 100M */
	//EthMacSetBits(MacBase + GmacConfig, GmacFESpeed10); /* 100M */
	EthMacSetBits(MacBase + GmacConfig, GmacFullDuplex); /*duplex*/
	// eth_back_off_limit(gmacdev,GmacBackoffLimit0);

	EthMacSetBits(MacBase + GmacConfig, GmacRetryDisable); /* disable retry */
	EthMacClearBits(MacBase + GmacConfig, GmacPadCrcStrip);
	EthMacClearBits(MacBase + GmacConfig, GmacDeferralCheck);
	//EthMacClearBits(MacBase + GmacConfig, GmacRxIpcOffload);
    

	/*Frame Filter Configuration*/
	EthMacClearBits(MacBase + GmacFrameFilter, GmacBroadcast);
	EthMacClearBits(MacBase + GmacFrameFilter, GmacSrcAddrFilter);
	EthMacClearBits(MacBase + GmacFrameFilter, GmacMulticastFilter);
	EthMacClearBits(MacBase + GmacFrameFilter, GmacDestAddrFilterNor);
	EthMacClearBits(MacBase + GmacFrameFilter, GmacMcastHashFilter);
	EthMacSetBits(MacBase + GmacFrameFilter, GmacPromiscuousMode | GmacFilterOff);

	/*Flow Control Configuration*/
	EthMacClearBits(MacBase + GmacOperationMode, GmacUnicastPauseFrame);
	EthMacClearBits(MacBase + GmacOperationMode, GmacRxFlowControl);
	EthMacClearBits(MacBase + GmacOperationMode, GmacTxFlowControl);

	/*MMC*/
	EthMacSetBits(MacBase + GmacMmcIntrMaskTx, 0xFFFFFFFF);
	EthMacSetBits(MacBase + GmacMmcIntrMaskRx, 0xFFFFFFFF);
	EthMacSetBits(MacBase + GmacMmcRxIpcIntrMask, 0xFFFFFFFF);
    
	EthMacEnablePadCrcStrip(MacBase);

  	/* watchdog */
  	uint32_t data = netdev->mtu | 0x10000;
  	write_reg( MacBase + GmacWatchdogReg, data );

	/*RGMII link-up*/
	EthMacSetBits( MacBase + GmacConfig, GmacTx);
	EthMacSetBits( MacBase + GmacConfig, GmacRx);
	EthMacSetBits( MacBase + GmacConfig, GmacLinkUp );


	return 0;
}



int EthDma_Reset(struct net_device *dev)
{
	int  cnt;
	ubase_t  DmaBase = dev->dmabase;
	uint32_t  data;

	/* Reset?? */
	write_reg( DmaBase + DmaBusMode, DmaResetOn );
	cnt = 1000;
	while(--cnt)
	{
		data = read_reg( DmaBase + DmaBusMode);
		if((data & DmaResetOn) == 0)
		{
			break;
		}
		plat_delay( 1000 );
	}
	if(cnt == 0)
	{
		return 1;
	}
	return 0;
}


int  EthDma_init( struct net_device *pdev )
{
	int  i;
  	int  iret;
  	ubase_t  MacBase = pdev->pbase;
  	ubase_t  DmaBase = pdev->dmabase;
  	uint32_t  data;


	/* Reset, DMA */
    EthDma_Reset(pdev);

	EthDma_DisableDmaRx(DmaBase);
	EthDma_DisableDmaTx(DmaBase);


    data = read_reg( DmaBase + DmaBusMode );

    
  	/* source address */
	data = (0xF7 << 8) | 0x7D;
  	write_reg( MacBase + GmacAddr0High, data );
	data = (0xB5 << 24) | (0x7B << 16) | (0x55 << 8) | 0x00;
  	write_reg( MacBase + GmacAddr0Low, data );
    
  

  	iret = EthDma_SetupTxDesc( pdev);
  	if ( 0 != iret )
  	{
    	return iret;
  	}

  	iret = EthDma_SetupRxDesc( pdev );
  	if ( 0 != iret )
  	{
    	return iret;
  	}

	/**/
	data = DmaTransPriority | DmaBurstLength32 | DmaDescriptorSkip0 | DmaArbitPr;
	write_reg( DmaBase + DmaBusMode, data );


        
	data = DmaRecvStoreAndForwd | DmaTranStoreAndForwd | DmaRxThreshCtrl32 | DmaFwdErrorFrames;
	//data = DmaFlushTxFifo;
	write_reg( DmaBase + DmaOperationMode, data );

  	/* enable int */
    write_reg( DmaBase + DmaInterrupt, (DmaIntNormal | DmaIntAbnormal | DmaIntRxNoBuffer | DmaIntRxCompleted | DmaIntTxCompleted|DmaIntTxNoBuffer ) );
	//write_reg( DmaBase + DmaInterrupt, (DmaIntNormal | DmaIntAbnormal  | DmaIntRxCompleted | DmaIntTxCompleted ) );
	
	/**/
	EthDma_EnableDmaRx( DmaBase );
	EthDma_EnableDmaTx( DmaBase );

	/**/
  	return 0;
}


void phy_set_speed(struct net_device *dev,uint32_t speed)
{
	uint16_t temp = 0;
	uint32_t pbase = dev->pbase;
	uint32_t phy_addr = dev->phy_addr;

	EthPhy_ReadPhyReg(pbase,phy_addr,0,&temp);
	temp &= ~0x3240;   /*MASK 6 9 12 13 bit*/
	if(speed == SPEED10)
		temp |= 0x00;
	else if(speed == SPEED100)		
		temp |= 0x2000;
	else if(speed == SPEED1000)
		temp |= 0x40;
	else //if(speed == SPEEDAUTO)
		temp |= 0x1000;

	//Full
	temp |= 0x100;
	EthPhy_WritePhyReg(pbase,phy_addr,0,temp);

}


/**
 * Sets the GMAC in loopback mode. 
  * When on GMAC operates in loop-back mode at GMII/MII.
  * @param[in] pointer to EthMacdevice.
  * \return returns void.
  * \note (G)MII Receive clock is required for loopback to work properly, as transmit clock is
  * not looped back internally.
  */
void eth_loopback_on(ubase_t pbase)
{
	EthMacSetBits(pbase+GmacConfig, GmacLoopback);
	return;
}
/**
  * Sets the GMAC in Normal mode. 
  * @param[in] pointer to EthMacdevice.
  * \return returns void.
  */
void eth_loopback_off(ubase_t pbase)
{
	EthMacClearBits(pbase+GmacConfig, GmacLoopback);
	return;
}

/**
  * Sets the GMAC core in Full-Duplex mode. 
  * @param[in] pointer to EthMacdevice.
  * \return returns void.
  */
void eth_set_full_duplex(ubase_t pbase)
{
	EthMacSetBits(pbase+GmacConfig, GmacDuplex);
	return;
}
/**
  * Sets the GMAC core in Half-Duplex mode. 
  * @param[in] pointer to EthMacdevice.
  * \return returns void.
  */
void eth_set_half_duplex(ubase_t pbase)
{
	EthMacClearBits(pbase+GmacConfig, GmacDuplex);
	return;
}

/**
  * Enables promiscous mode.
  * When enabled Address filter modules pass all incoming frames regardless of their Destination
  * and source addresses.
  * @param[in] pointer to EthMacdevice.
  * \return void.
  */
void eth_promisc_enable( ubase_t pbase)
{
	EthMacSetBits(pbase+GmacFrameFilter, GmacPromiscuousMode);
	return;
}
/**
  * Clears promiscous mode.
  * When called the GMAC falls back to normal operation from promiscous mode.
  * @param[in] pointer to EthMacdevice.
  * \return void.
  */
void eth_promisc_disable( ubase_t pbase)
{
	EthMacClearBits(pbase+GmacFrameFilter, GmacPromiscuousMode);
	return;
}



/*Return: link up/down*/
int eth_link_status(struct net_device *netdev)
{

   return  netdev->linkflag;

}

int eth_isready(struct net_device *netdev)
{
	if((netdev->flag & ETH_FLAG_INIT_OK)&& netdev->linkflag )
		return 1;
	else
		return 0;
}


void gmac_set_speed(uint32_t pbase,uint32_t speed)
{
	uint32_t temp,old;
	temp = read_reg(pbase+GmacConfig);
	old = temp;
	temp &= ~(GmacSelectMii | GmacFESpeed100);
	if(speed == SPEED10)
		temp = temp | GmacSelectMii | GmacFESpeed10;
	else if(speed == SPEED100)
		temp = temp | GmacSelectMii | GmacFESpeed100;
	else if(speed == SPEED1000)
		temp = temp | GmacSelectGmii;
	else
		temp = old;
	
	write_reg(pbase+GmacConfig, temp);
}

int eth_get_cs(struct net_device *dev, uint32_t *value)
{
	uint32_t temp,speed;

	speed = 0;
	temp = read_reg(dev->pbase + GmacRgmiiCS);
	if(temp & GmacLinkStatus)
	{
		speed = 0;
		
		if((temp & 0x6) == GmacSpeed_25)
			speed |= ETH_SPEED_100M;
		else if((temp & 0x6) == GmacSpeed_125)
			speed |= ETH_SPEED_1000M;
		
		if(temp & GmacModeDuplex)
			speed |= ETH_DUPLEX_MODE;
	}
	*value = speed;
	return 0;
}


int eth_rx(struct net_device* dev){

    int  iret = 0;

	DmaDesc * desc;
	DmaDesc * desc_reg;
	ubase_t   buffer_addr;

	uint32_t status;
    uint32_t length;

	while(1){

		// no cache desc 
		desc =(DmaDesc*)((ubase_t)dev->host_rxdesc_cur);
		if(desc == NULL){
			//dev->host_rxdesc_cur = read_reg(dev->dmabase+DmaRxCurrDesc);
			ks_exception_assert(0);
		}
		ks_cpu_dcache_invalidate((void *)desc, sizeof(DmaDesc));
		
		//desc_reg = (DmaDesc*)read_reg(dev->dmabase+DmaRxCurrDesc);
		//kprintf("rx host DmaDesc: %x status %x \r\n",desc,desc->status);
		//kprintf("rx rreg DmaDesc: %x status %x \r\n",desc_reg,desc_reg->status);
		
		if ( desc_owned_by_dma(desc) )
		{
		    //resume_rx_dma( dev->dmabase);
			goto finish_recv;
		}

	    /* if there are some errors, retry */
		if ( 0 == desc_rx_is_valid(desc) )
		{
			/* assign back to dma */
			desc_rx_reset_config(desc,(ubase_t)dev->host_rxdesc);
			ks_cpu_dcache_clean((void *)desc, sizeof(DmaDesc));
			dev->host_rxdesc_cur = (DmaDesc *)desc_get_next(desc);
	        dev->rx_errors++;
		    continue ;
		}


	    /* ok, get one pkt */
	    length = desc_data_length(desc);
	    //kprintf("length %d \r\n",length);
     	if(length < DMA_BUF_SIZE){
			//desc->buffer,length   handler 
			if(dev->cb_recvlist!=NULL){
				dev->cb_recvlist((uint8_t * )desc->buffer,length);
			}
		}
		
		desc_rx_reset_config(desc,(ubase_t)dev->host_rxdesc);
		ks_cpu_dcache_clean((void *)desc, sizeof(DmaDesc));
		
	    dev->host_rxdesc_cur = (DmaDesc *)desc_get_next(desc);
		
	    /* recv buffer unavailable */
	    dev->rx_pkts++;
	}


finish_recv:
	/**/


    return 1 ;

}




int  eth_tx( struct net_device * dev)
{
	DmaDesc *desc ;
	uint32_t  temp;
	int txcount ;
	txcount = 0;
	void * buffer; 
	int len ;
	DmaDesc * desc_reg;
	ubase_t   buffer_addr;

	
	while(1){

		desc = (DmaDesc*)((ubase_t)dev->host_txdesc_cur);
		if(desc == NULL ){
			ks_exception_assert(0);
		}

		ks_cpu_dcache_invalidate((void *)desc, sizeof(DmaDesc));
		//desc_reg = (DmaDesc*)read_reg(dev->dmabase+DmaTxCurrDesc);
		//kprintf("tx host DmaDesc: %x status %x \r\n",desc,desc->status);
		//kprintf("tx rreg DmaDesc: %x status %x \r\n",desc_reg,desc_reg->status);
		
		//判断当前发送的desc　是否处于dma发送
		if (desc_owned_by_dma(desc) )
		{
			return txcount;
		}

		//get buffer form queue array
		if(dev->cb_txbuffer!=NULL){
			len = dev->cb_txbuffer((uint8_t * )desc->buffer,DMA_BUF_SIZE);
			//kprintf( "cb_txbuffer DmaDesc: %x  len %d \r\n",desc,len);
			if(len == 0){
				return txcount;
			}
		}else{
			return txcount;
		}

		desc_tx_setup_config(desc,len,(ubase_t)dev->host_txdesc);
		//kprintf("txx host DmaDesc: %x status %x \r\n",desc,desc->status);
		//kprintf( "eth_tx DmaDesc: %x  len %d \r\n",desc,len);
		//ks_os_printf_hex(0,desc->buffer, len);
		
		ks_cpu_dcache_clean((void *)desc, sizeof(DmaDesc));

		dev->host_txdesc_cur = (DmaDesc *)desc_get_next(desc);

		resume_tx_dma(dev->dmabase);

		dev->tx_pkts++;
	}
	
	return txcount;
	
}



int eth_init( struct net_device *pdev)
{

	pdev->flag &= (~(ETH_FLAG_INIT_OK ));
	EthBuffs_Init(pdev);
	EthCtx_Init(pdev);
	EthPhy_Init(pdev);

	EthDma_init(pdev);
	EthMac_init(pdev);

	pdev->flag |= ETH_FLAG_INIT_OK;

	return 0 ;
}





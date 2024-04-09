
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ks_os.h"

#include "synop_ethmac.h"
#include "eth_controller.h"
#include "eth_low_level_driver.h"
#include "ks_shell.h"
#include "ks_printf.h"

extern struct net_device g_netdevice ;


int  eth_show_eth_ctx(struct net_device *netdev,  int tlen, char * pstr )
{
	int  ofs;

	uintptr_t pbase = netdev->pbase;
	uint32_t DmaBase = pbase+0x1000;
	uint32_t missedFr ;

	if(tlen < 512)
	{
		return -1;
	}
	/**/
	ofs = 0;
	missedFr = read_reg(DmaBase+DmaMissedFr);

	ofs += sprintf( pstr + ofs, "\r\nCtx \r\n");
    ofs += sprintf( pstr + ofs, "\t flag :%lu \r\n",netdev->flag);
    ofs += sprintf( pstr + ofs, "\t linkflag: %lu \r\n",netdev->linkflag);
    ofs += sprintf( pstr + ofs, "\t linkmode: %lu \r\n",netdev->linkmode);
    ofs += sprintf( pstr + ofs, "\t linkspeed:%lu\r\n",netdev->linkspeed);
    ofs += sprintf( pstr + ofs, "\t pbase: %x \r\n",netdev->pbase);
    ofs += sprintf( pstr + ofs, "\t dmabase: %x \r\n",netdev->dmabase);
    ofs += sprintf( pstr + ofs, "\t irq_count: %d \r\n",netdev->irq_count);
    ofs += sprintf( pstr + ofs, "\t tx_pkts: %d \r\n",netdev->tx_pkts);
    ofs += sprintf( pstr + ofs, "\t rx_pkts: %d \r\n",netdev->rx_pkts);
    ofs += sprintf( pstr + ofs, "\t phy_addr: %x \r\n",netdev->phy_addr);
    ofs += sprintf( pstr + ofs, "\t phy_id1: %x \r\n",netdev->phy_id1);
    ofs += sprintf( pstr + ofs, "\t phy_id2: %x \r\n",netdev->phy_id2);
	/**/
	return ofs;
	
}


int  eth_show_mmc_stat(struct net_device *netdev,  int tlen, char * pstr )
{
	int  ofs;

	uintptr_t pbase = netdev->pbase;
	uint32_t DmaBase = pbase+0x1000;
	uint32_t missedFr ;

	if(tlen < 512)
	{
		return -1;
	}
	/**/
	ofs = 0;
	missedFr = read_reg(DmaBase+DmaMissedFr);
    ofs += sprintf( pstr + ofs, "\r\n");
	ofs += sprintf( pstr + ofs, "\nMMC RX:\r\n");
	ofs += sprintf( pstr + ofs, "\tall frames : %lu\r\n",read_reg(pbase+GmacMmcRxFrameCountGb));
	ofs += sprintf( pstr + ofs, "\tCRC error frames : %lu\r\n",read_reg(pbase+GmacMmcRxCrcError));
	ofs += sprintf( pstr + ofs, "\trecv error frames : %lu\r\n",read_reg(pbase+GmacMmcRxRcvError));
	ofs += sprintf( pstr + ofs, "\tover1600 error frames : %lu\r\n",read_reg(pbase+GmacMmcRxWatchdobError));
	ofs += sprintf( pstr + ofs, "\tbytes of all frames: %lu\r\n",read_reg(pbase+GmacMmcRxOctetCountGb));
	ofs += sprintf( pstr + ofs, "\tbytes of good frames: %lu\r\n",read_reg(pbase+GmacMmcRxOctetCountG));
	ofs += sprintf( pstr + ofs, "\tmissed frame because recv-buffer unavailable : %lu\r\n",missedFr&DmaRecvBufUnavailMask);
	ofs += sprintf( pstr + ofs, "\nMMC TX:\r\n");
	ofs += sprintf( pstr + ofs, "\tall frames : %lu\r\n",read_reg(pbase+GmacMmcTxFrameCountGb));
	ofs += sprintf( pstr + ofs, "\tgood frames : %lu\r\n",read_reg(pbase+GmacMmcTxFrameCountG)); 
	ofs += sprintf( pstr + ofs, "\tall transmitted bytes : %lu\r\n",read_reg(pbase+GmacMmcTxOctetCountGb));

	ofs += sprintf( pstr + ofs, "\nSoft:\r\n");
	ofs += sprintf( pstr + ofs, "\tphy addr : %lu\r\n",netdev->phy_addr );
    ofs += sprintf( pstr + ofs, "\tirq count : %lu\r\n",netdev->irq_count);
	ofs += sprintf( pstr + ofs, "\trx packet : %lu\r\n",netdev->rx_pkts );
	ofs += sprintf( pstr + ofs, "\ttx packet : %lu\r\n",netdev->tx_pkts );
	ofs += sprintf( pstr + ofs, "\trx packet error : %lu\r\n",netdev->rx_errors);
	ofs += sprintf( pstr + ofs, "\ttx packet error : %lu\r\n",netdev->tx_errors);

	/**/
	return ofs;
	
}


int  eth_show_mac_reg(struct net_device *netdev,  int tlen, char * pstr )
{
	int  ofs;

	uintptr_t pbase = netdev->pbase;
	uint32_t DmaBase = pbase+0x1000;
	uint32_t missedFr ;

	if(tlen < 512)
	{
		return -1;
	}
	/**/
	ofs = 0;
	missedFr = read_reg(DmaBase+DmaMissedFr);

	ofs += sprintf( pstr + ofs, "\r\nMac \r\n");
    ofs += sprintf( pstr + ofs, "\t GmacConfig :%x \r\n",read_reg(pbase+GmacConfig));
	ofs += sprintf( pstr + ofs, "\t GmacGmiiAddr :%x \r\n",read_reg(pbase+GmacGmiiAddr));
    ofs += sprintf( pstr + ofs, "\t GmacFrameFilter: %x \r\n",read_reg(pbase+GmacFrameFilter));
    ofs += sprintf( pstr + ofs, "\t GmacOperationMode: %x \r\n",read_reg(pbase+GmacOperationMode));
    ofs += sprintf( pstr + ofs, "\t GmacRgmiiCS: %x \r\n",read_reg(pbase+GmacRgmiiCS));




	/**/
	return ofs;
	
}



int  eth_show_dma_reg(struct net_device *netdev,  int tlen, char * pstr )
{
	int  ofs;

	uintptr_t pbase = netdev->pbase;
	uint32_t DmaBase = pbase+0x1000;
	uint32_t missedFr ;

	if(tlen < 512)
	{
		return -1;
	}
	/**/ 
	ofs = 0;
	missedFr = read_reg(DmaBase+DmaMissedFr);

    ofs += sprintf( pstr + ofs, "\r\nDma \r\n");
    ofs += sprintf( pstr + ofs, "\t DmaBusMode: %x \r\n",read_reg(DmaBase+DmaBusMode));
    ofs += sprintf( pstr + ofs, "\t DmaStatus: %x \r\n",read_reg(DmaBase+DmaStatus));
    ofs += sprintf( pstr + ofs, "\t DmaOperationMode: %x \r\n",read_reg(DmaBase+DmaOperationMode));
    
    ofs += sprintf( pstr + ofs, "\t DmaInterrupt: %x \r\n",read_reg(DmaBase+DmaInterrupt));
    ofs += sprintf( pstr + ofs, "\t DmaTxCurrDesc: %x \r\n",read_reg(DmaBase+DmaTxCurrDesc));
    ofs += sprintf( pstr + ofs, "\t DmaRxCurrDesc: %x \r\n",read_reg(DmaBase+DmaRxCurrDesc));
    ofs += sprintf( pstr + ofs, "\t DmaTxCurrAddr: %x \r\n",read_reg(DmaBase+DmaTxCurrAddr));
    ofs += sprintf( pstr + ofs, "\t DmaRxCurrAddr: %x \r\n",read_reg(DmaBase+DmaRxCurrAddr));
    ofs += sprintf( pstr + ofs, "\t host_rxdesc_cur: %x \r\n",netdev->host_rxdesc_cur);
    ofs += sprintf( pstr + ofs, "\t host_txdesc_cur: %x \r\n",netdev->host_txdesc_cur);

	/**/
	return ofs;
	
}




int  eth_show_dma_txdes(struct net_device *netdev,  int tlen, char * pstr )
{
	int  ofs;
    int i = 0;
	uintptr_t pbase = netdev->pbase;
	uint32_t DmaBase = netdev->dmabase;
    uint32_t status ;
	if(tlen < 512)
	{
		return -1;
	}
	/**/
	ofs = 0;

    DmaDesc * tx_desc = netdev->host_txdesc;
    //DcacheInvalidate(tx_rxdesc,sizeof(DmaDesc) * netdev->tx_dma_num);

	ofs += sprintf( pstr + ofs, "host_txdesc_cur: %x \r\n",netdev->host_txdesc_cur);

   	for (i = 0; i < netdev->tx_dma_num; i++) 
	{
		ofs += sprintf( pstr + ofs, "\r\n tx[%d] DmaDesc %x\r\n",i,netdev->host_txdesc+i);
        ofs += sprintf( pstr + ofs, "\t status :%x \r\n",tx_desc->status);
        ofs += sprintf( pstr + ofs, "\t length: %x \r\n",tx_desc->length);
        ofs += sprintf( pstr + ofs, "\t buffer: %x \r\n",tx_desc->buffer);
        ofs += sprintf( pstr + ofs, "\t next: %x \r\n",tx_desc->next);
        //ofs += sprintf( pstr + ofs, "\t udata: %x \r\n",tx_rxdesc->udata);
        tx_desc++;
	}
    
	/**/
	return ofs;
	
}


int  eth_show_dma_rxdes(struct net_device *netdev,  int tlen, char * pstr )
{
	int  ofs;
    int i = 0;
	uintptr_t pbase = netdev->pbase;
	uint32_t DmaBase = netdev->dmabase;
    uint32_t status ;
	if(tlen < 512)
	{
		return -1;
	}
	/**/
	ofs = 0;

    DmaDesc * rx_desc = netdev->host_rxdesc;

	ofs += sprintf( pstr + ofs, "host_rxdesc_cur: %x \r\n",netdev->host_rxdesc_cur);

   	for (i = 0; i < netdev->rx_dma_num; i++) 
	{
		ofs += sprintf( pstr + ofs, "\r\n rx[%d] DmaDesc %x\r\n",i,netdev->host_rxdesc+i);
        ofs += sprintf( pstr + ofs, "\t status :%x \r\n",rx_desc->status);
        ofs += sprintf( pstr + ofs, "\t length: %x \r\n",rx_desc->length);
        ofs += sprintf( pstr + ofs, "\t buffer: %x \r\n",rx_desc->buffer);
        ofs += sprintf( pstr + ofs, "\t next: %x \r\n",rx_desc->next);
        //ofs += sprintf( pstr + ofs, "\t udata: %x \r\n",tx_rxdesc->udata);
        rx_desc++;
	}
    
	/**/
	return ofs;
	
}


int eth_send_test(cmd_proc_t* ctx,struct net_device *pdev, uint32_t cnt)
{
	int ret,counter;
	uint8_t *ptr;
	uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
/*
	uint8_t pkt[100] = {0x00,0xb0,0xc0,0xd0,0xe0,0x01,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x08,0x00,0x45,0x00,
0x00,0x2c,0x00,0x07,0x00,0x00,0xff,0x06,0xe9,0xa7,0xc0,0xa8,0xa8,0x65,0xc0,0xa8,
0xa8,0x66,0x27,0x0f,0x27,0x0f,0x00,0x00,0x19,0xb5,0x4e,0xb0,0x78,0xf5,0x60,0x12,
0x16,0xd0,0x7f,0xb0,0x00,0x00,0x02,0x04,0x05,0xb4};
*/
	uint8_t pkt[100] = {
	0x00,0xb0,0xc0,0xd0,0xe0,0x01,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x08,0x00,0x45,0x00,
	0x00,0x28,0x00,0x00,0x00,0x00,0xff,0x06,0xe9,0xb2,0xc0,0xa8,0xa8,0x65,0xc0,0xa8,
	0xa8,0x66,0x27,0x0f,0x27,0x0f,0x00,0x00,0x00,0x00,0x17,0x77,0x2e,0xa8,0x50,0x14,
	0x16,0xd0,0x32,0xa6,0x00,0x00};
	
	int pkt_len = 54;
	
	if(eth_isready(pdev) == 0)
	{
		ks_shell_printf(ctx->uart_id,"eth_isready failed\n");
        return -1;
    }
    counter = 0;

	uint8_t buffer[128];

#if 0
	ether_header* phead;

	phead = (ether_header*)buffer;

	memcpy(phead->ether_dhost,broadcast_mac,6);
	memcpy(phead->ether_shost,ks_driver_eth_get_mac(),6);
	phead->ether_type = (0x8888);
	
	for(int i = 0 ;i<100;i++){

		buffer[i+sizeof(ether_header)] = i;
	}
#endif
	memcpy(buffer,pkt,pkt_len);
	//ks_os_printf(0,"sendpkt etype %x  len %d \r\n",0x888,100);
	ks_os_printf_hex(0,(char*)buffer,pkt_len);


	//ks_driver_eth_poll_send_buffer(buffer,100);

	while(cnt--)
	{
		ret = ks_driver_eth_send_buffer(buffer,pkt_len);
		if(ret >=0)
		{
		    counter++;
			continue;
		}else{
	
    	    if(ret == -2)
    		{
    			ks_shell_printf(ctx->uart_id,"no link\n");
    			break;
    		}
        }
	}

    ks_shell_printf(ctx->uart_id,"eth_send_test counter %d \r\n",counter);
	return 0;
}


int ethctx(cmd_proc_t* ctx,int argc,  char **argv ){  
    char  ctxbuf[1024];    
    memset(ctxbuf,0,1024);    
    eth_show_eth_ctx(&g_netdevice,sizeof(ctxbuf),ctxbuf);    
    ks_shell_printf(ctx->uart_id,"%s \r\n",ctxbuf);    
	return 0;
}

int ethmac(cmd_proc_t* ctx,int argc,  char **argv ){  
    char  mmcbuf[1024];    
    memset(mmcbuf,0,1024);    
    eth_show_mac_reg(&g_netdevice,512,mmcbuf);    
    ks_shell_printf(ctx->uart_id,"%s \r\n",mmcbuf);    
	return 0;
}

int ethdma(cmd_proc_t* ctx,int argc,  char **argv ){  
    char dmabuf[1024];    
    memset(dmabuf,0,1024);    
    eth_show_dma_reg(&g_netdevice,512,dmabuf);    
    ks_shell_printf(ctx->uart_id,"%s \r\n",dmabuf);
    
    uint32_t Status = read_reg(g_netdevice.pbase+0x1000+DmaStatus);
    
    ks_shell_printf(ctx->uart_id,"DmaStatus: %x  \r\n",Status);
    if(Status & DmaIntRxNoBuffer){
        ks_shell_printf(ctx->uart_id,"\t DmaIntRxNoBuffer \r\n");
    }

     if(Status & DmaIntRxCompleted){
        ks_shell_printf(ctx->uart_id,"\t DmaIntRxCompleted \r\n");
    }
    
    if(Status & DmaIntTxNoBuffer){
        ks_shell_printf(ctx->uart_id,"\t DmaIntTxNoBuffer \r\n");
    }

    if(Status & DmaIntTxCompleted){
        ks_shell_printf(ctx->uart_id,"\t DmaIntTxCompleted \r\n");
    }

	//line
	if(Status & GmacLineIntfIntr)
	{
	    ks_shell_printf(ctx->uart_id,"\t GmacLineIntfIntr: \r\n");
	}
	return 0;
}


int ethmmc(cmd_proc_t* ctx,int argc,  char **argv ){
    char  mmcbuf[1024];    
    memset(mmcbuf,0,1024);   

    eth_show_mmc_stat(&g_netdevice,1024,mmcbuf);    
    ks_shell_printf(ctx->uart_id,"%s \r\n",mmcbuf);    
	return 0;
}



int ethtxdes(cmd_proc_t* ctx,int argc,  char **argv ){  
    char  desbuf[1024];    
    memset(desbuf,0,1024);   
    
    if(g_netdevice.host_txdesc != NULL){
        eth_show_dma_txdes(&g_netdevice,1024,desbuf);    
        ks_shell_printf(ctx->uart_id,"%s \r\n",desbuf);   
    } 
    return CMD_ERR_OK;
}

int ethrxdes(cmd_proc_t* ctx,int argc,  char **argv ){  
    //char  desbuf[1024];    
   // memset(desbuf,0,1024);   
    struct net_device *netdev = &g_netdevice;
	int i;
    if(g_netdevice.host_txdesc != NULL){

		uintptr_t pbase = netdev->pbase;
		uint32_t DmaBase = netdev->dmabase;
	    uint32_t status ;

	    DmaDesc * rx_desc = netdev->host_rxdesc;

		 ks_shell_printf(ctx->uart_id, "host_rxdesc_cur: %x \r\n",netdev->host_rxdesc_cur);

	   	for (i = 0; i < netdev->rx_dma_num; i++) 
		{
			 ks_shell_printf(ctx->uart_id, "\r\n rx[%d] DmaDesc %x\r\n",i,netdev->host_rxdesc+i);
	         ks_shell_printf(ctx->uart_id, "\t status :%x \r\n",rx_desc->status);
	         ks_shell_printf(ctx->uart_id, "\t length: %x \r\n",rx_desc->length);
	         ks_shell_printf(ctx->uart_id,"\t buffer: %x \r\n",rx_desc->buffer);
	         ks_shell_printf(ctx->uart_id,"\t next: %x \r\n",rx_desc->next);
	        //ofs += sprintf( pstr + ofs, "\t udata: %x \r\n",tx_rxdesc->udata);
	        rx_desc++;
		}
	    
	
        //eth_show_dma_rxdes(&g_netdevice,1024,desbuf);
 
    } 

	return CMD_ERR_OK;
}

static int ethsend(cmd_proc_t* ctx,int argc,  char **argv ){  
    if(g_netdevice.linkflag){
        eth_send_test(ctx,&g_netdevice,1);
    }else{
        ks_shell_printf(ctx->uart_id,"ethsend not link \r\n");    
    }
	return CMD_ERR_OK;
}



#if 0
void sdramtest(cmd_proc_t* ctx,int argc,  char **argv ){  

	uint8_t * ptr = (g_TestSdramMemBuff) ;
	ks_shell_printf(ctx->uart_id,"g_TestSdramMemBuff %x  \r\n",ptr);	  

    for(int i =1 ; i<= 100;i++){

		*ptr= i;
		 ptr++;
    }
	
	//kprintfHex(g_TestSdramMemBuff, 20);
  
}
#endif


static int  phybsr(cmd_proc_t* ctx,int argc,  char **argv ) {
	struct net_device *netdev = & g_netdevice;
    uint16_t status,temp;

	int iret = EthPhy_ReadPhyReg(netdev->pbase, netdev->phy_addr, PHY_BSR, &status);
	if(iret != 0){

		ks_shell_printf(ctx->uart_id, "EthPhy_ReadPhyReg erro!!!");
		return CMD_ERR_DEVICE_OP_FAILED;

	}
	
	ks_shell_printf(ctx->uart_id, "EthPhy_ReadPhyReg PHY_BSR %x \r\n",status);

	return 0;
	
}


static int phyid(cmd_proc_t* ctx,int argc,  char **argv)
{
	int  iret,i;
	uint16_t  id1,id2;
	struct net_device *dev = & g_netdevice;

	dev->phy_addr = 0;
	/* identfier register */
	for(i=0; i<32; i++){

		iret = EthPhy_ReadPhyReg(dev->pbase, i, 2, &id1);
		if(iret != 0)
			return CMD_ERR_DEVICE_OP_FAILED;

		iret = EthPhy_ReadPhyReg(dev->pbase, i, 3, &id2);
		if(iret != 0)
			return CMD_ERR_DEVICE_OP_FAILED;

		if(((id1 == 0xFFFF) && (id2 == 0xFFFF)) || ((id1 == 0) && (id2 == 0)))
		{
			continue;
		}
		else
		{
			dev->phy_addr = i;
			dev->phy_id1 = id1;
			dev->phy_id2 = id2;
            ks_shell_printf(ctx->uart_id, "netdev->phy_id1  %x  netdev->phy_id2 %x  \r\n",dev->phy_id1,dev->phy_id2);

			break;
		}
	}

	if(i == 32)
	{
	     ks_shell_printf(ctx->uart_id, "phy id is not recognized.\r\n");
		return CMD_ERR_DEVICE_OP_FAILED; //phy id is not recognized.
	}
    
	return 0;
}




static int ethcount(cmd_proc_t* ctx,int argc,  char **argv ){  

	struct net_device *netdev =  & g_netdevice;
    int iret;
	uint32_t enable;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &enable);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
	else
	{
	  	enable = 1;
	}
	
	if(enable){
		ks_shell_printf(ctx->uart_id, "\t irq_count: %d \r\n",netdev->irq_count);
		ks_shell_printf(ctx->uart_id,"\t tx_pkts: %d \r\n",netdev->tx_pkts);
		ks_shell_printf(ctx->uart_id,"\t rx_pkts: %d \r\n",netdev->rx_pkts);
		ks_shell_printf(ctx->uart_id,"\t rx_errors: %d \r\n",netdev->rx_errors);
		ks_shell_printf(ctx->uart_id,"\t tx_errors: %d \r\n",netdev->tx_errors);

	}else{
		netdev->irq_count = 0;
		netdev->tx_pkts = 0;
		netdev->rx_pkts = 0;
		netdev->rx_errors = 0;
		netdev->tx_errors = 0;
		ks_shell_printf(ctx->uart_id, "\t irq_count: %d \r\n",netdev->irq_count);
		ks_shell_printf(ctx->uart_id,"\t tx_pkts: %d \r\n",netdev->tx_pkts);
		ks_shell_printf(ctx->uart_id,"\t rx_pkts: %d \r\n",netdev->rx_pkts);
		ks_shell_printf(ctx->uart_id,"\t rx_errors: %d \r\n",netdev->rx_errors);
		ks_shell_printf(ctx->uart_id,"\t tx_errors: %d \r\n",netdev->tx_errors);

	}


	return 0;

}



static int phyloop(cmd_proc_t* ctx,int argc,  char **argv ){  

	struct net_device *netdev =  & g_netdevice;
    int iret;
	uint32_t enable;
    uint16_t status,temp;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &enable);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
	else
	{
	  	enable = 1;
	}



	 iret = EthPhy_ReadPhyReg(netdev->pbase, netdev->phy_addr, PHY_MSR, &status);
	if(iret != 0){

		ks_shell_printf(ctx->uart_id, "EthPhy_ReadPhyReg erro!!!");
		return CMD_ERR_DEVICE_OP_FAILED;

	}
	
	ks_shell_printf(ctx->uart_id, "EthPhy_ReadPhyReg PHY_MSR %x \r\n",status);

	
	if(enable){

		EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, PHY_MSR, status|1<<9);

	}else{

		EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, PHY_MSR, status&(~(1<<9)));

	}


	return 0;

}


static int macloop(cmd_proc_t* ctx,int argc,  char **argv ){  

	struct net_device *netdev =  & g_netdevice;
    int iret;
	uint32_t enable;
    uint16_t status,temp;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &enable);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
	else
	{
	  	enable = 1;
	}



	iret = EthPhy_ReadPhyReg(netdev->pbase, netdev->phy_addr, PHY_BCR, &status);
	if(iret != 0){

		ks_shell_printf(ctx->uart_id, "EthPhy_ReadPhyReg erro!!!");
		return CMD_ERR_DEVICE_OP_FAILED;

	}
	
	ks_shell_printf(ctx->uart_id, "EthPhy_ReadPhyReg PHY_BCR %x \r\n",status);

	
	if(enable){

		EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, PHY_BCR, status|1<<14);

	}else{

		EthPhy_WritePhyReg(netdev->pbase, netdev->phy_addr, PHY_BCR, status&(~(1<<14)));

	}


	return 0;

}

static cmd_proc_t eth_cmds[] = {
    {.cmd = "ethsend", .fn = ethsend,  .help = "ethsend"},
    {.cmd = "ethtxdes", .fn = ethtxdes,  .help = "ethtxdes"},
    {.cmd = "ethrxdes", .fn = ethrxdes,  .help = "ethrxdes"},
    {.cmd = "ethmmc", .fn = ethmmc,  .help = "ethmmc"},
    {.cmd = "ethdma", .fn = ethdma,  .help = "ethdma"},
    {.cmd = "ethmac", .fn = ethmac,  .help = "ethmac"},
    {.cmd = "ethctx", .fn = ethctx,  .help = "ethctx"},
	{.cmd = "ethcount", .fn = ethcount,  .help = "ethcount"},
	{.cmd = "macloop", .fn = macloop,  .help = "macloop"},

	{.cmd = "phybsr", .fn = phybsr,  .help = "phybsr"},
	{.cmd = "phyid", .fn = phyid,  .help = "phyid"},
	{.cmd = "phyloop", .fn = phyloop,  .help = "phyloop"},


};


void eth_shell_cmd_init()
{
	ks_shell_add_cmds(eth_cmds, sizeof(eth_cmds) / sizeof(cmd_proc_t));

}







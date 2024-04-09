
#pragma  once
#include <stdint.h>

#define  DMA_Descriptors_Normal   0



/**********************************************************
 * GMAC registers Map
 * For Pci based system address is BARx + GmacRegisterBase
 * For any other system translation is done accordingly
 **********************************************************/
enum GmacRegisters              
{
  GmacConfig          = 0x0000,    /* Mac config Register                       */
  GmacFrameFilter     = 0x0004,    /* Mac frame filtering controls              */
  GmacHashHigh        = 0x0008,    /* Multi-cast hash table high                */
  GmacHashLow         = 0x000C,    /* Multi-cast hash table low                 */
  GmacGmiiAddr        = 0x0010,    /* GMII address Register(ext. Phy)           */
  GmacGmiiData        = 0x0014,    /* GMII data Register(ext. Phy)              */
  GmacOperationMode     = 0x0018,    /* Operation Mode Register                     */
  GmacVlan            = 0x001C,    /* VLAN tag Register (IEEE 802.1Q)           */
  
  GmacVersion         = 0x0020,    /* GMAC Core Version Register                */ 
  GmacDebug           = 0x0024,    /* GMAC Core Debug Register                */ 
  GmacWakeupAddr      = 0x0028,    /* GMAC wake-up frame filter adrress reg     */ 
  GmacPmtCtrlStatus   = 0x002C,    /* PMT control and status register           */ 
 
  GmacInterruptStatus = 0x0038,    /* Mac Interrupt ststus register             */  
  GmacInterruptMask   = 0x003C,    /* Mac Interrupt Mask register               */  
 
  GmacAddr0High       = 0x0040,    /* Mac address0 high Register                */
  GmacAddr0Low        = 0x0044,    /* Mac address0 low Register                 */
  GmacAddr1High       = 0x0048,    /* Mac address1 high Register                */
  GmacAddr1Low        = 0x004C,    /* Mac address1 low Register                 */
  GmacAddr2High       = 0x0050,    /* Mac address2 high Register                */
  GmacAddr2Low        = 0x0054,    /* Mac address2 low Register                 */
  GmacAddr3High       = 0x0058,    /* Mac address3 high Register                */
  GmacAddr3Low        = 0x005C,    /* Mac address3 low Register                 */
  GmacAddr4High       = 0x0060,    /* Mac address4 high Register                */
  GmacAddr4Low        = 0x0064,    /* Mac address4 low Register                 */
  GmacAddr5High       = 0x0068,    /* Mac address5 high Register                */
  GmacAddr5Low        = 0x006C,    /* Mac address5 low Register                 */
  GmacAddr6High       = 0x0070,    /* Mac address6 high Register                */
  GmacAddr6Low        = 0x0074,    /* Mac address6 low Register                 */
  GmacAddr7High       = 0x0078,    /* Mac address7 high Register                */
  GmacAddr7Low        = 0x007C,    /* Mac address7 low Register                 */
  GmacAddr8High       = 0x0080,    /* Mac address8 high Register                */
  GmacAddr8Low        = 0x0084,    /* Mac address8 low Register                 */
  GmacAddr9High       = 0x0088,    /* Mac address9 high Register                */
  GmacAddr9Low        = 0x008C,    /* Mac address9 low Register                 */
  GmacAddr10High      = 0x0090,    /* Mac address10 high Register               */
  GmacAddr10Low       = 0x0094,    /* Mac address10 low Register                */
  GmacAddr11High      = 0x0098,    /* Mac address11 high Register               */
  GmacAddr11Low       = 0x009C,    /* Mac address11 low Register                */
  GmacAddr12High      = 0x00A0,    /* Mac address12 high Register               */
  GmacAddr12Low       = 0x00A4,    /* Mac address12 low Register                */
  GmacAddr13High      = 0x00A8,    /* Mac address13 high Register               */
  GmacAddr13Low       = 0x00AC,    /* Mac address13 low Register                */
  GmacAddr14High      = 0x00B0,    /* Mac address14 high Register               */
  GmacAddr14Low       = 0x00B4,    /* Mac address14 low Register                */
  GmacAddr15High      = 0x00B8,    /* Mac address15 high Register               */
  GmacAddr15Low       = 0x00BC,    /* Mac address15 low Register                */
  GmacRgmiiCS         = 0x00D8,	   /* Mac RGMII/SGMII/SMII control and status reg*/
  GmacWatchdogReg         = 0x00DC,   /* Mac Watchdog reg                          */
  
};
enum Gmac_Rgmii_cs
{
  GmacLinkStatus     = 0x8,     /*link status     */
  GmacSpeed_2_5      = 0x0,       /*current link speed 2.5M  */
  GmacSpeed_25       = 0x2,        /*current link speed 25M   */
  GmacSpeed_125      = 0x4,         /*current link pseed 125M  */
  GmacModeDuplex         = 0x1,            /* current mode of link  half/full duplex */
};

/* GmacConfig              = 0x0000,    Mac config Register  Layout */
enum GmacConfigReg      
{ 
                                           /* Bit description                      Bits         R/W   Reset value  */
  GmacSrcMacInsert_address0 = 0x20000000,
  GmacSrcMacInsert_address1 = 0x60000000,
  GmacSrcMacReplace_address0 = 0x30000000,
  GmacSrcMacReplace_address1 = 0x70000000,

  GmacCrcStripEnable     = 0X02000000,
  GmacWatchdog       = 0x00800000,
  GmacWatchdogDisable      = 0x00800000,     /* (WD)Disable watchdog timer on Rx      23           RW                */
  GmacWatchdogEnable       = 0x00000000,     /* Enable watchdog timer                                        0       */

  GmacJabber       = 0x00400000,
  GmacJabberDisable        = 0x00400000,     /* (JD)Disable jabber timer on Tx        22           RW                */
  GmacJabberEnable         = 0x00000000,     /* Enable jabber timer                                          0       */

  GmacFrameBurst           = 0x00200000,
  GmacFrameBurstEnable     = 0x00200000,     /* (BE)Enable frame bursting during Tx   21           RW                */
  GmacFrameBurstDisable    = 0x00000000,     /* Disable frame bursting                                       0       */
  
  GmacJumboFrame           = 0x00100000,
  GmacJumboFrameEnable     = 0x00100000,     /* (JE)Enable jumbo frame for Tx         20           RW                */
  GmacJumboFrameDisable    = 0x00000000,     /* Disable jumbo frame                                          0       */

  GmacInterFrameGap7       = 0x000E0000,     /* (IFG) Config7 - 40 bit times          19:17        RW                */
  GmacInterFrameGap6       = 0x000C0000,     /* (IFG) Config6 - 48 bit times                                         */
  GmacInterFrameGap5       = 0x000A0000,     /* (IFG) Config5 - 56 bit times                                         */
  GmacInterFrameGap4       = 0x00080000,     /* (IFG) Config4 - 64 bit times                                         */
  GmacInterFrameGap3       = 0x00040000,     /* (IFG) Config3 - 72 bit times                                         */
  GmacInterFrameGap2       = 0x00020000,     /* (IFG) Config2 - 80 bit times                                         */
  GmacInterFrameGap1       = 0x00010000,     /* (IFG) Config1 - 88 bit times                                         */
  GmacInterFrameGap0       = 0x00000000,     /* (IFG) Config0 - 96 bit times                                 000     */
 
  GmacDisableCrs     = 0x00010000, 
  GmacMiiGmii      = 0x00008000,
  GmacSelectMii            = 0x00008000,     /* (PS)Port Select-MII mode              15           RW                */
  GmacSelectGmii           = 0x00000000,     /* GMII mode                                                    0       */

  GmacFESpeed100           = 0x00004000,     /*(FES)Fast Ethernet speed 100Mbps       14           RW                */ 
  GmacFESpeed10            = 0x00000000,     /* 10Mbps                                                       0       */ 

  GmacRxOwn      = 0x00002000, 
  GmacDisableRxOwn         = 0x00002000,     /* (DO)Disable receive own packets       13           RW                */
  GmacEnableRxOwn          = 0x00000000,     /* Enable receive own packets                                   0       */
  
  GmacLoopback       = 0x00001000,
  GmacLoopbackOn           = 0x00001000,     /* (LM)Loopback mode for GMII/MII        12           RW                */
  GmacLoopbackOff          = 0x00000000,     /* Normal mode                                                  0       */

  GmacDuplex       = 0x00000800,
  GmacFullDuplex           = 0x00000800,     /* (DM)Full duplex mode                  11           RW                */
  GmacHalfDuplex           = 0x00000000,     /* Half duplex mode                                             0       */

  GmacRxIpcOffload     = 0x00000400,     /*IPC checksum offload         10           RW        0       */

  GmacRetry      = 0x00000200,
  GmacRetryDisable         = 0x00000200,     /* (DR)Disable Retry                      9           RW                */
  GmacRetryEnable          = 0x00000000,     /* Enable retransmission as per BL                              0       */

  GmacLinkUp               = 0x00000100,     /* (LUD)Link UP                           8           RW                */ 
  GmacLinkDown             = 0x00000100,     /* Link Down                                                    0       */ 
  
  GmacPadCrcStrip    = 0x00000080,
  GmacPadCrcStripEnable    = 0x00000080,     /* (ACS) Automatic Pad/Crc strip enable   7           RW                */
  GmacPadCrcStripDisable   = 0x00000000,     /* Automatic Pad/Crc stripping disable                          0       */
  
  GmacBackoffLimit     = 0x00000060,
  GmacBackoffLimit3        = 0x00000060,     /* (BL)Back-off limit in HD mode          6:5         RW                */
  GmacBackoffLimit2        = 0x00000040,     /*                                                                      */
  GmacBackoffLimit1        = 0x00000020,     /*                                                                      */
  GmacBackoffLimit0        = 0x00000000,     /*                                                              00      */

  GmacDeferralCheck    = 0x00000010,
  GmacDeferralCheckEnable  = 0x00000010,     /* (DC)Deferral check enable in HD mode   4           RW                */
  GmacDeferralCheckDisable = 0x00000000,     /* Deferral check disable                                       0       */
   
  GmacTx       = 0x00000008,
  GmacTxEnable             = 0x00000008,     /* (TE)Transmitter enable                 3           RW                */
  GmacTxDisable            = 0x00000000,     /* Transmitter disable                                          0       */

  GmacRx       = 0x00000004,
  GmacRxEnable             = 0x00000004,     /* (RE)Receiver enable                    2           RW                */
  GmacRxDisable            = 0x00000000,     /* Receiver disable                                             0       */

	GmacPreLen5			= 0x01,
	GmacPreLen3         =  0x02,
	GmacPreLen7         = 0x00,
    GmacPreLenMask		   = 0x03,

};


/* GmacFrameFilter    = 0x0004,     Mac frame filtering controls Register Layout*/
enum GmacFrameFilterReg 
{
  GmacFilter               = 0x80000000,
  GmacFilterOff            = 0x80000000,     /* (RA)Receive all incoming packets       31         RW                 */
  GmacFilterOn             = 0x00000000,     /* Receive filtered packets only                                0       */

  GmacVlanTagEn            = 0x00010000,
  
  GmacHashPerfectFilter    = 0x00000400,     /*Hash or Perfect Filter enable           10         RW         0       */

  GmacSrcAddrFilter        = 0x00000200,
  GmacSrcAddrFilterEnable  = 0x00000200,     /* (SAF)Source Address Filter enable       9         RW                 */
  GmacSrcAddrFilterDisable = 0x00000000,     /*                                                              0       */

  GmacSrcInvaAddrFilter    = 0x00000100,
  GmacSrcInvAddrFilterEn   = 0x00000100,     /* (SAIF)Inv Src Addr Filter enable        8         RW                 */
  GmacSrcInvAddrFilterDis  = 0x00000000,     /*                                                              0       */

  GmacPassControl          = 0x000000C0,
  GmacPassControl3         = 0x000000C0,     /* (PCS)Forwards ctrl frms that pass AF    7:6       RW                 */
  GmacPassControl2         = 0x00000080,     /* Forwards all control frames                                          */
  GmacPassControl1         = 0x00000040,     /* Does not pass control frames                                         */
  GmacPassControl0         = 0x00000000,     /* Does not pass control frames                                 00      */

  GmacBroadcast            = 0x00000020,
  GmacBroadcastDisable     = 0x00000020,     /* (DBF)Disable Rx of broadcast frames     5         RW                 */
  GmacBroadcastEnable      = 0x00000000,     /* Enable broadcast frames                                      0       */

  GmacMulticastFilter      = 0x00000010,
  GmacMulticastFilterOff   = 0x00000010,     /* (PM) Pass all multicast packets         4         RW                 */
  GmacMulticastFilterOn    = 0x00000000,     /* Pass filtered multicast packets                              0       */

  GmacDestAddrFilter       = 0x00000008,
  GmacDestAddrFilterInv    = 0x00000008,     /* (DAIF)Inverse filtering for DA          3         RW                 */
  GmacDestAddrFilterNor    = 0x00000000,     /* Normal filtering for DA                                      0       */

  GmacMcastHashFilter      = 0x00000004,
  GmacMcastHashFilterOn    = 0x00000004,     /* (HMC)perfom multicast hash filtering    2         RW                 */
  GmacMcastHashFilterOff   = 0x00000000,     /* perfect filtering only                                       0       */

  GmacUcastHashFilter      = 0x00000002,
  GmacUcastHashFilterOn    = 0x00000002,     /* (HUC)Unicast Hash filtering only        1         RW                 */
  GmacUcastHashFilterOff   = 0x00000000,     /* perfect filtering only                                       0       */

  GmacPromiscuousMode      = 0x00000001,
  GmacPromiscuousModeOn    = 0x00000001,     /* Receive all frames                      0         RW                 */
  GmacPromiscuousModeOff   = 0x00000000,     /* Receive filtered packets only                                0       */
};


/*GmacFlowControl    = 0x0018,    Flow control Register   Layout                  */
enum GmacFlowControlReg  
{                                          
  GmacPauseTimeMask        = 0xFFFF0000,     /* (PT) PAUSE TIME field in the control frame  31:16   RW       0x0000  */
  GmacPauseTimeShift       = 16,
  
  GmacPauseLowThresh     = 0x00000030,
  GmacPauseLowThresh3      = 0x00000030,     /* (PLT)thresh for pause tmr 256 slot time      5:4    RW               */
  GmacPauseLowThresh2      = 0x00000020,     /*                           144 slot time                              */
  GmacPauseLowThresh1      = 0x00000010,     /*                            28 slot time                              */
  GmacPauseLowThresh0      = 0x00000000,     /*                             4 slot time                       000    */

  GmacUnicastPauseFrame    = 0x00000008,
  GmacUnicastPauseFrameOn  = 0x00000008,     /* (UP)Detect pause frame with unicast addr.     3    RW                */
  GmacUnicastPauseFrameOff = 0x00000000,     /* Detect only pause frame with multicast addr.                   0     */

  GmacRxFlowControl    = 0x00000004,
  GmacRxFlowControlEnable  = 0x00000004,     /* (RFE)Enable Rx flow control                   2    RW                */
  GmacRxFlowControlDisable = 0x00000000,     /* Disable Rx flow control                                        0     */

  GmacTxFlowControl        = 0x00000002,
  GmacTxFlowControlEnable  = 0x00000002,     /* (TFE)Enable Tx flow control                   1    RW                */
  GmacTxFlowControlDisable = 0x00000000,     /* Disable flow control                                           0     */

  GmacFlowControlBackPressure= 0x00000001,
  GmacSendPauseFrame       = 0x00000001,     /* (FCB/PBA)send pause frm/Apply back pressure   0    RW          0     */
};

enum GmacAddressReg
{
	GmacAddrEn = 0x80000000,
	GmacAddrSrc = 0x40000000,
	GmacAddrMaskBytesMask = 0x3f,
	GmacAddrMaskBytesShift = 24,
};


/**********************************************************
 * GMAC DMA registers
 * For Pci based system address is BARx + GmaDmaBase
 * For any other system translation is done accordingly
 **********************************************************/

enum DmaRegisters             
{
  DmaBusMode        = 0x0000,    /* CSR0 - Bus Mode Register                          */
  DmaTxPollDemand   = 0x0004,    /* CSR1 - Transmit Poll Demand Register              */
  DmaRxPollDemand   = 0x0008,    /* CSR2 - Receive Poll Demand Register               */
  DmaRxBaseAddr     = 0x000C,    /* CSR3 - Receive Descriptor list base address       */
  DmaTxBaseAddr     = 0x0010,    /* CSR4 - Transmit Descriptor list base address      */
  DmaStatus         = 0x0014,    /* CSR5 - Dma status Register                        */
  DmaOperationMode  = 0x0018,    /* CSR6 - Dma Operation Mode Register                */
  DmaInterrupt      = 0x001C,    /* CSR7 - Interrupt enable                           */
  DmaMissedFr       = 0x0020,    /* CSR8 - Missed Frame & Buffer overflow Counter     */
  DmaAxiBusmd       = 0x0028,    /* -     */
  DmaTxCurrDesc     = 0x0048,    /*      - Current host Tx Desc Register              */ 
  DmaRxCurrDesc     = 0x004C,    /*      - Current host Rx Desc Register              */ 
  DmaTxCurrAddr     = 0x0050,    /* CSR20 - Current host transmit buffer address      */
  DmaRxCurrAddr     = 0x0054,    /* CSR21 - Current host receive buffer address       */
 
};

/*DmaBusMode               = 0x0000,    CSR0 - Bus Mode */
enum DmaBusModeReg         
{           
                                          /* Bit description                                Bits     R/W   Reset value */
  DmaTransPriority        = 0x08000000,   /* (TXPR) Transmit Priority                         27     RW        00      */ 
  DmaUseSeparatePBL       = 0x00800000,  /* Use separate PBL */
  DmaFixedBurstEnable     = 0x00010000,   /* (FB)Fixed Burst SINGLE, INCR4, INCR8 or INCR16   16     RW                */
  DmaFixedBurstDisable    = 0x00000000,   /*             SINGLE, INCR                                          0       */

  DmaTxPriorityRatio11    = 0x00000000,   /* (PR)TX:RX DMA priority ratio 1:1                15:14   RW        00      */ 
  DmaTxPriorityRatio21    = 0x00004000,   /* (PR)TX:RX DMA priority ratio 2:1                                          */ 
  DmaTxPriorityRatio31    = 0x00008000,   /* (PR)TX:RX DMA priority ratio 3:1                                          */ 
  DmaTxPriorityRatio41    = 0x0000C000,   /* (PR)TX:RX DMA priority ratio 4:1                                          */ 
  
  DmaBurstLengthx8        = 0x01000000,   /* When set mutiplies the PBL by 8                  24      RW        0      */ 
  
  DmaBurstLength256       = 0x01002000,   /*(DmaBurstLengthx8 | DmaBurstLength32) = 256      [24]:13:8                 */  
  DmaBurstLength128       = 0x01001000,   /*(DmaBurstLengthx8 | DmaBurstLength16) = 128      [24]:13:8                 */
  DmaBurstLength64        = 0x01000800,   /*(DmaBurstLengthx8 | DmaBurstLength8) = 64        [24]:13:8                 */
  DmaBurstLength32        = 0x00002000,   /* (PBL) programmable Dma burst length = 32        13:8    RW                */
  DmaBurstLength16        = 0x00001000,   /* Dma burst length = 16                                                     */
  DmaBurstLength8         = 0x00000800,   /* Dma burst length = 8                                                      */
  DmaBurstLength4         = 0x00000400,   /* Dma burst length = 4                                                      */
  DmaBurstLength2         = 0x00000200,   /* Dma burst length = 2                                                      */
  DmaBurstLength1         = 0x00000100,   /* Dma burst length = 1                                                      */
  DmaBurstLength0         = 0x00000000,   /* Dma burst length = 0                                               0x00   */

  DmaDescriptor8Words     = 0x00000080,   /* Enh Descriptor works  1=> 8 word descriptor      7                  0    */
  DmaDescriptor4Words     = 0x00000000,   /* Enh Descriptor works  0=> 4 word descriptor      7                  0    */

  DmaDescriptorSkip16     = 0x00000040,   /* (DSL)Descriptor skip length (no.of dwords)       6:2     RW               */
  DmaDescriptorSkip8      = 0x00000020,   /* between two unchained descriptors                                         */
  DmaDescriptorSkip4      = 0x00000010,   /*                                                                           */
  DmaDescriptorSkip2      = 0x00000008,   /*                                                                           */
  DmaDescriptorSkip1      = 0x00000004,   /*                                                                           */
  DmaDescriptorSkip0      = 0x00000000,   /*                                                                    0x00   */

  DmaArbitRr              = 0x00000000,   /* (DA) DMA RR arbitration                            1     RW         0     */ 
  DmaArbitPr              = 0x00000002,   /* Rx has priority over Tx                                                   */  
  
  DmaResetOn              = 0x00000001,   /* (SWR)Software Reset DMA engine                     0     RW               */
  DmaResetOff             = 0x00000000,   /*                                                                      0    */
};

/*DmaControl        = 0x0018,     CSR6 - Dma Operation Mode Register                */
enum DmaControlReg        
{ 
  DmaDisableDropTcpCs     = 0x04000000,   /* (DT) Dis. drop. of tcp/ip CS error frames        26      RW        0       */

  DmaRecvStoreAndForwd    = 0x02000000,   /* (RSF) Receive Store and Forward                  25      RW        0       */
  DmaTranStoreAndForwd    = 0x00200000,   /* (SF)Store and forward                            21      RW        0       */
  
  DmaFlushTxFifo          = 0x00100000,   /* (FTF)Tx FIFO controller is reset to default      20      RW        0       */ 
  
  DmaTxThreshCtrl         = 0x0001C000,   /* (TTC)Controls thre Threh of MTL tx Fifo          16:14   RW                */ 
  DmaTxThreshCtrl16       = 0x0001C000,   /* (TTC)Controls thre Threh of MTL tx Fifo 16       16:14   RW                */ 
  DmaTxThreshCtrl24       = 0x00018000,   /* (TTC)Controls thre Threh of MTL tx Fifo 24       16:14   RW                */ 
  DmaTxThreshCtrl32       = 0x00014000,   /* (TTC)Controls thre Threh of MTL tx Fifo 32       16:14   RW                */ 
  DmaTxThreshCtrl40       = 0x00010000,   /* (TTC)Controls thre Threh of MTL tx Fifo 40       16:14   RW                */   
  DmaTxThreshCtrl256      = 0x0000c000,   /* (TTC)Controls thre Threh of MTL tx Fifo 256      16:14   RW                */   
  DmaTxThreshCtrl192      = 0x00008000,   /* (TTC)Controls thre Threh of MTL tx Fifo 192      16:14   RW                */   
  DmaTxThreshCtrl128      = 0x00004000,   /* (TTC)Controls thre Threh of MTL tx Fifo 128      16:14   RW                */   
  DmaTxThreshCtrl64       = 0x00000000,   /* (TTC)Controls thre Threh of MTL tx Fifo 64       16:14   RW        000     */ 
  
  DmaTxStart              = 0x00002000,   /* (ST)Start/Stop transmission                      13      RW        0       */

  DmaRxFlowCtrlDeact      = 0x00401800,   /* (RFD)Rx flow control deact. threhold             [22]:12:11   RW                 */ 
  DmaRxFlowCtrlDeact1K    = 0x00000000,   /* (RFD)Rx flow control deact. threhold (1kbytes)   [22]:12:11   RW        00       */ 
  DmaRxFlowCtrlDeact2K    = 0x00000800,   /* (RFD)Rx flow control deact. threhold (2kbytes)   [22]:12:11   RW                 */ 
  DmaRxFlowCtrlDeact3K    = 0x00001000,   /* (RFD)Rx flow control deact. threhold (3kbytes)   [22]:12:11   RW                 */ 
  DmaRxFlowCtrlDeact4K    = 0x00001800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */  
  DmaRxFlowCtrlDeact5K    = 0x00400000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */  
  DmaRxFlowCtrlDeact6K    = 0x00400800,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */  
  DmaRxFlowCtrlDeact7K    = 0x00401000,   /* (RFD)Rx flow control deact. threhold (4kbytes)   [22]:12:11   RW                 */  
  
  DmaRxFlowCtrlAct        = 0x00800600,   /* (RFA)Rx flow control Act. threhold              [23]:10:09   RW                 */ 
  DmaRxFlowCtrlAct1K      = 0x00000000,   /* (RFA)Rx flow control Act. threhold (1kbytes)    [23]:10:09   RW        00       */ 
  DmaRxFlowCtrlAct2K      = 0x00000200,   /* (RFA)Rx flow control Act. threhold (2kbytes)    [23]:10:09   RW                 */ 
  DmaRxFlowCtrlAct3K      = 0x00000400,   /* (RFA)Rx flow control Act. threhold (3kbytes)    [23]:10:09   RW                 */ 
  DmaRxFlowCtrlAct4K      = 0x00000300,   /* (RFA)Rx flow control Act. threhold (4kbytes)    [23]:10:09   RW                 */   
  DmaRxFlowCtrlAct5K      = 0x00800000,   /* (RFA)Rx flow control Act. threhold (5kbytes)    [23]:10:09   RW                 */   
  DmaRxFlowCtrlAct6K      = 0x00800200,   /* (RFA)Rx flow control Act. threhold (6kbytes)    [23]:10:09   RW                 */   
  DmaRxFlowCtrlAct7K      = 0x00800400,   /* (RFA)Rx flow control Act. threhold (7kbytes)    [23]:10:09   RW                 */   
  
  DmaRxThreshCtrl         = 0x00000018,   /* (RTC)Controls thre Threh of MTL rx Fifo          4:3   RW                */ 
  DmaRxThreshCtrl64       = 0x00000000,   /* (RTC)Controls thre Threh of MTL tx Fifo 64       4:3   RW                */ 
  DmaRxThreshCtrl32       = 0x00000008,   /* (RTC)Controls thre Threh of MTL tx Fifo 32       4:3   RW                */ 
  DmaRxThreshCtrl96       = 0x00000010,   /* (RTC)Controls thre Threh of MTL tx Fifo 96       4:3   RW                */ 
  DmaRxThreshCtrl128      = 0x00000018,   /* (RTC)Controls thre Threh of MTL tx Fifo 128      4:3   RW                */ 

  DmaEnHwFlowCtrl         = 0x00000100,   /* (EFC)Enable HW flow control                      8       RW                 */ 
  DmaDisHwFlowCtrl        = 0x00000000,   /* Disable HW flow control                                            0        */ 
        
  DmaFwdErrorFrames       = 0x00000080,   /* (FEF)Forward error frames                        7       RW        0       */
  DmaFwdUnderSzFrames     = 0x00000040,   /* (FUF)Forward undersize frames                    6       RW        0       */
  DmaTxSecondFrame        = 0x00000004,   /* (OSF)Operate on second frame                     4       RW        0       */
  DmaRxStart              = 0x00000002,   /* (SR)Start/Stop reception                         1       RW        0       */
};
  
/*DmaStatus         = 0x0014,    CSR5 - Dma status Register                        */
enum DmaStatusReg         
{ 
  /*Bit 28 27 and 26 indicate whether the interrupt due to PMT GMACMMC or GMAC LINE Remaining bits are DMA interrupts*/                      

  #ifdef LPI_SUPPORT
  GmacLPIIntr             = 0x40000000,   /* GMC LPI interrupt                                  31     RO       0       */ 
  #endif
  
  GmacPmtIntr             = 0x10000000,   /* (GPI)Gmac subsystem interrupt                      28     RO       0       */ 
  GmacMmcIntr             = 0x08000000,   /* (GMI)Gmac MMC subsystem interrupt                  27     RO       0       */ 
  GmacLineIntfIntr        = 0x04000000,   /* Line interface interrupt                           26     RO       0       */

  DmaErrorBit2            = 0x02000000,   /* (EB)Error bits 0-data buffer, 1-desc. access       25     RO       0       */
  DmaErrorBit1            = 0x01000000,   /* (EB)Error bits 0-write trnsf, 1-read transfr       24     RO       0       */
  DmaErrorBit0            = 0x00800000,   /* (EB)Error bits 0-Rx DMA, 1-Tx DMA                  23     RO       0       */

  DmaTxState              = 0x00700000,   /* (TS)Transmit process state                         22:20  RO               */
  DmaTxStopped            = 0x00000000,   /* Stopped - Reset or Stop Tx Command issued                         000      */
  DmaTxFetching           = 0x00100000,   /* Running - fetching the Tx descriptor                                       */
  DmaTxWaiting            = 0x00200000,   /* Running - waiting for status                                               */
  DmaTxReading            = 0x00300000,   /* Running - reading the data from host memory                                */
  DmaTxSuspended          = 0x00600000,   /* Suspended - Tx Descriptor unavailabe                                       */
  DmaTxClosing            = 0x00700000,   /* Running - closing Rx descriptor                                            */

  DmaRxState              = 0x000E0000,   /* (RS)Receive process state                         19:17  RO                */
  DmaRxStopped            = 0x00000000,   /* Stopped - Reset or Stop Rx Command issued                         000      */
  DmaRxFetching           = 0x00020000,   /* Running - fetching the Rx descriptor                                       */
  DmaRxWaiting            = 0x00060000,   /* Running - waiting for packet                                               */
  DmaRxSuspended          = 0x00080000,   /* Suspended - Rx Descriptor unavailable                                      */
  DmaRxClosing            = 0x000A0000,   /* Running - closing descriptor                                               */
  DmaRxQueuing            = 0x000E0000,   /* Running - queuing the recieve frame into host memory                       */

  DmaIntNormal            = 0x00010000,   /* (NIS)Normal interrupt summary                     16     RW        0       */
  DmaIntAbnormal          = 0x00008000,   /* (AIS)Abnormal interrupt summary                   15     RW        0       */

  DmaIntEarlyRx           = 0x00004000,   /* Early receive interrupt (Normal)       RW        0       */
  DmaIntBusError          = 0x00002000,   /* Fatal bus error (Abnormal)             RW        0       */
  DmaIntEarlyTx           = 0x00000400,   /* Early transmit interrupt (Abnormal)    RW        0       */
  DmaIntRxWdogTO          = 0x00000200,   /* Receive Watchdog Timeout (Abnormal)    RW        0       */
  DmaIntRxStopped         = 0x00000100,   /* Receive process stopped (Abnormal)     RW        0       */
  DmaIntRxNoBuffer        = 0x00000080,   /* Receive buffer unavailable (Abnormal)  RW        0       */
  DmaIntRxCompleted       = 0x00000040,   /* Completion of frame reception (Normal) RW        0       */
  DmaIntTxUnderflow       = 0x00000020,   /* Transmit underflow (Abnormal)          RW        0       */
  DmaIntRcvOverflow       = 0x00000010,   /* Receive Buffer overflow interrupt      RW        0       */
  DmaIntTxJabberTO        = 0x00000008,   /* Transmit Jabber Timeout (Abnormal)     RW        0       */
  DmaIntTxNoBuffer        = 0x00000004,   /* Transmit buffer unavailable (Normal)   RW        0       */
  DmaIntTxStopped         = 0x00000002,   /* Transmit process stopped (Abnormal)    RW        0       */
  DmaIntTxCompleted       = 0x00000001,   /* Transmit completed (Normal)            RW        0       */
};

enum DmaMissedFrReg
{
	DmaFifoverOverbit       = 0x10000000,    /*Overflow bit for FIFO overflow counter*/
	DmaFIFOoverCounterMask  =  0xffe0000,
	DmaFIFOoverCounterShift = 17,
	DmaRecvBufUnavailOverbit       = 0x10000,       /*Overflow bit for recv-buffer unavailable counter*/
	DmaRecvBufUnavailMask   = 0xffff,
};


/*

********** Default Descritpor structure  ****************************
DmaRxBaseAddr     = 0x000C,   CSR3 - Receive Descriptor list base address       
DmaRxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below 

Similarly 
DmaTxBaseAddr     = 0x0010,  CSR4 - Transmit Descriptor list base address
DmaTxBaseAddr is the pointer to the first Rx Descriptors. the Descriptor format in Little endian with a
32 bit Data bus is as shown below
                  --------------------------------------------------------------------
    RDES0/TDES0  |OWN (31)| Status                                                   |
      --------------------------------------------------------------------
    RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1          |
      --------------------------------------------------------------------
    RDES2/TDES2  |  Buffer 1 Address                                                 |
      --------------------------------------------------------------------
    RDES3/TDES3  |  Buffer 2 Address / Next Descriptor Address                       |
      --------------------------------------------------------------------
*/
enum DmaDescriptorStatus    /* status word of DMA descriptor */
{
	DescOwnByDma          = 0x80000000,   /* (OWN)Descriptor is owned by DMA engine            31      RW                  */

	DescDAFilterFail      = 0x40000000,   /* (AFM)Rx - DA Filter Fail for the rx frame         30                          */

	DescFrameLengthMask   = 0x3FFF0000,   /* (FL)Receive descriptor frame length               29:16                       */
	DescFrameLengthShift  = 16,

	DescError             = 0x00008000,   /* (ES)Error summary bit  - OR of the follo. bits:   15                          */
	      /*  DE || OE || IPC || LC || RWT || RE || CE */
	DescRxTruncated       = 0x00004000,   /* (DE)Rx - no more descriptors for receive frame    14                          */
	DescSAFilterFail      = 0x00002000,   /* (SAF)Rx - SA Filter Fail for the received frame   13                          */
	DescRxLengthError 	  = 0x00001000,   /* (LE)Rx - frm size not matching with len field     12                          */
	DescRxDamaged         = 0x00000800,   /* (OE)Rx - frm was damaged due to buffer overflow   11                          */
	DescRxVLANTag         = 0x00000400,   /* (VLAN)Rx - received frame is a VLAN frame         10                          */
	DescRxFirst           = 0x00000200,   /* (FS)Rx - first descriptor of the frame             9                          */
	DescRxLast            = 0x00000100,   /* (LS)Rx - last descriptor of the frame              8                          */
	//  DescRxLongFrame       = 0x00000080,   /* (Giant Frame)Rx - frame is longer than 1518/1522   7                          */
	DescRxCollision       = 0x00000040,   /* (LC)Rx - late collision occurred during reception  6                          */
	DescRxFrameEther      = 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5                          */
	DescRxWatchdog        = 0x00000010,   /* (RWT)Rx - watchdog timer expired during reception  4                          */
	DescRxMiiError        = 0x00000008,   /* (RE)Rx - error reported by MII interface           3                          */
	DescRxDribbling       = 0x00000004,   /* (DE)Rx - frame contains non int multiple of 8 bits 2                          */
	DescRxCrc             = 0x00000002,   /* (CE)Rx - CRC error                                 1                          */
	//  DescRxMacMatch        = 0x00000001,   /* (RX MAC Address) Rx mac address reg(1 to 15)match  0                          */ 


	//Rx Descriptor Checksum Offload engine (type 2) encoding
	DescRxPayChkError     = 0x00000001,   /* ()  Rx - Rx Payload Checksum Error                 0                          */  
	DescRxIpv4ChkError    = 0x00000080,   /* (IPC CS ERROR)Rx - Ipv4 header checksum error      7                          */

	DescRxChkBit0   = 0x00000001,   /*()  Rx - Rx Payload Checksum Error                  0                          */
	DescRxChkBit7   = 0x00000080,   /* (IPC CS ERROR)Rx - Ipv4 header checksum error      7                          */
	DescRxChkBit5   = 0x00000020,   /* (FT)Rx - Frame type - Ethernet, otherwise 802.3    5                          */

	DescTxIpv4ChkError    = 0x00010000,   /* (IHE) Tx Ip header error                           16                         */
	DescTxTimeout         = 0x00004000,   /* (JT)Tx - Transmit jabber timeout                   14                         */
	DescTxFrameFlushed    = 0x00002000,   /* (FF)Tx - DMA/MTL flushed the frame due to SW flush 13                         */
	DescTxPayChkError     = 0x00001000,   /* (PCE) Tx Payload checksum Error                    12                         */
	DescTxLostCarrier     = 0x00000800,   /* (LC)Tx - carrier lost during tramsmission          11                         */
	DescTxNoCarrier       = 0x00000400,   /* (NC)Tx - no carrier signal from the tranceiver     10                         */
	DescTxLateCollision   = 0x00000200,   /* (LC)Tx - transmission aborted due to collision      9                         */
	DescTxExcCollisions   = 0x00000100,   /* (EC)Tx - transmission aborted after 16 collisions   8                         */
	DescTxVLANFrame       = 0x00000080,   /* (VF)Tx - VLAN-type frame                            7                         */

	DescTxCollMask        = 0x00000078,   /* (CC)Tx - Collision count                           6:3                        */
	DescTxCollShift       = 3,

	DescTxExcDeferral     = 0x00000004,   /* (ED)Tx - excessive deferral                          2                        */
	DescTxUnderflow       = 0x00000002,   /* (UF)Tx - late data arrival from the memory           1                        */
	DescTxDeferred        = 0x00000001,   /* (DB)Tx - frame transmision deferred                  0                        */

	/*
	This explains the RDES1/TDES1 bits layout
	--------------------------------------------------------------------
	RDES1/TDES1  | Control Bits | Byte Count Buffer 2 | Byte Count Buffer 1          |
	--------------------------------------------------------------------

	*/  
	//DmaDescriptorLength     length word of DMA descriptor


#if  DMA_Descriptors_Normal
	DescTxIntEnable       = 0x80000000,   /* (IC)Tx - interrupt on completion                    31                       */
	DescTxLast            = 0x40000000,   /* (LS)Tx - Last segment of the frame                  30                       */
	DescTxFirst           = 0x20000000,   /* (FS)Tx - First segment of the frame                 29                       */
	DescTxDisableCrc      = 0x04000000,   /* (DC)Tx - Add CRC disabled (first segment only)      26                       */
	DescTxDisablePadd     = 0x00800000,   /* (DP)disable padding, added by - reyaz               23                       */
	RxDisIntCompl         = 0x80000000,   /* (Disable Rx int on completion)       31      */

	DescTxEndOfRing		= 0x02000000,	/* (TER)End of descriptors ring 					   25						  */
	DescRxEndOfRing 	    = 0x02000000,   /* (TER)End of descriptors ring  */
	DescTxChain			= 0x01000000,	 /* (TCH)Second buffer address is chain address 		24						 */
	DescRxChain           = 0x01000000,   /* (TCH)Second buffer address is chain address         24 				  	*/

	DescTxCisMask         = 0x18000000,   /* Tx checksum offloading control mask      28:27     */
	DescTxCisBypass       = 0x00000000,   /* Checksum bypass                */
	DescTxCisIpv4HdrCs    = 0x08000000, /* IPv4 header checksum               */
	DescTxCisTcpOnlyCs    = 0x10000000, /* TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present */
	DescTxCisTcpPseudoCs  = 0x18000000, /* TCP/UDP/ICMP checksum fully in hardware including pseudo header    */


#else  //
	DescTxIntEnable       = 0x40000000,   /* (IC)Tx - interrupt on completion                    30                       */
	DescTxLast            = 0x20000000,   /* (LS)Tx - Last segment of the frame                  29                       */
	DescTxFirst           = 0x10000000,   /* (FS)Tx - First segment of the frame                 28                       */
	DescTxDisableCrc      = 0x08000000,   /* (DC)Tx - Add CRC disabled (first segment only)      27                       */
	DescTxDisablePadd     = 0x04000000,   /* (DP)disable padding, added by - reyaz               26                       */
	DescTTSEEnable        = 0x02000000,   /* TTSE: Transmit Timestamp Enable              		 25 						*/

	RxDisIntCompl         = 0x80000000,   /* (Disable Rx int on completion)       31      */


	DescRxChain           = 0x00004000,   /* (TCH)Second buffer address is chain address         	14 						*/
	DescRxEndOfRing 	    = 0x00008000,   /* (TER)End of descriptors ring  										*/
	DescTxEndOfRing 	  = 0x00200000,   /* (TER)End of descriptors ring  							21			*/
	DescTxChain           = 0x00100000,   /* (TCH)Second buffer address is chain address         	20 						*/

	DescTxCisMask         = 0x00c00000,   /* Tx checksum offloading control mask      22:23     */
	DescTxCisBypass       = 0x00000000,   /* Checksum bypass                */
	DescTxCisIpv4HdrCs    = 0x00400000, /* IPv4 header checksum               */
	DescTxCisTcpOnlyCs    = 0x00800000, /* TCP/UDP/ICMP checksum. Pseudo header checksum is assumed to be present */
	DescTxCisTcpPseudoCs  = 0x00c00000, /* TCP/UDP/ICMP checksum fully in hardware including pseudo header    */

#endif
	DescSize2Mask         = 0x003FF800,   /* (TBS2) Buffer 2 size                                21:11                    */
	DescSize2Shift        = 11,
	DescSize1Mask         = 0x000007FF,   /* (TBS1) Buffer 1 size                                10:0                     */
	DescSize1Shift        = 0,



};



/*GmacGmiiAddr             = 0x0010,    GMII address Register(ext. Phy) Layout          */
enum GmacGmiiAddrReg      
{
  GmiiDevMask              = 0x0000F800,     /* (PA)GMII device address                 15:11     RW         0x00    */
  GmiiDevShift             = 11,

  GmiiRegMask              = 0x000007C0,     /* (GR)GMII register in selected Phy       10:6      RW         0x00    */
  GmiiRegShift             = 6,
  
  GmiiCsrClkMask           = 0x0000001C,     /*CSR Clock bit Mask      4:2           */
  GmiiCsrClk5              = 0x00000014,     /* (CR)CSR Clock Range     250-300 MHz      4:2      RW         000     */
  GmiiCsrClk4              = 0x00000010,     /*                         150-250 MHz                                  */
  GmiiCsrClk3              = 0x0000000C,     /*                         35-60 MHz                                    */
  GmiiCsrClk2              = 0x00000008,     /*                         20-35 MHz                                    */
  GmiiCsrClk1              = 0x00000004,     /*                         100-150 MHz                                  */
  GmiiCsrClk0              = 0x00000000,     /*                         60-100 MHz                                   */

  GmiiWrite                = 0x00000002,     /* (GW)Write to register                      1      RW                 */
  GmiiRead                 = 0x00000000,     /* Read from register                                            0      */

  GmiiBusy                 = 0x00000001,     /* (GB)GMII interface is busy                 0      RW          0      */
};

/* GmacGmiiData            = 0x0014,    GMII data Register(ext. Phy) Layout             */
enum GmacGmiiDataReg      
{
  GmiiDataMask             = 0x0000FFFF,     /* (GD)GMII Data                             15:0    RW         0x0000  */
};


/**********************************************************
 * Mac Management Counters (MMC)
 **********************************************************/

enum MMC_ENABLE
{
	GmacMmcCntrl			= 0x0100,	/* mmc control for operating mode of MMC						*/
	GmacMmcIntrRx			= 0x0104,	/* maintains interrupts generated by rx counters					*/
	GmacMmcIntrTx			= 0x0108,	/* maintains interrupts generated by tx counters					*/
	GmacMmcIntrMaskRx		= 0x010C,	/* mask for interrupts generated from rx counters					*/
	GmacMmcIntrMaskTx		= 0x0110,	/* mask for interrupts generated from tx counters					*/
};
enum MMC_TX
{
	GmacMmcTxOctetCountGb		= 0x0114,	/*Bytes Tx excl. of preamble and retried bytes     (Good or Bad)			*/
	GmacMmcTxFrameCountGb		= 0x0118,	/*Frames Tx excl. of retried frames	           (Good or Bad)			*/
	GmacMmcTxBcFramesG		= 0x011C,	/*Broadcast Frames Tx 				   (Good)				*/
	GmacMmcTxMcFramesG		= 0x0120,	/*Multicast Frames Tx				   (Good)				*/
	
	GmacMmcTx64OctetsGb		= 0x0124,	/*Tx with len 64 bytes excl. of pre and retried    (Good or Bad)			*/
	GmacMmcTx65To127OctetsGb	= 0x0128,	/*Tx with len >64 bytes <=127 excl. of pre and retried    (Good or Bad)			*/
	GmacMmcTx128To255OctetsGb	= 0x012C,	/*Tx with len >128 bytes <=255 excl. of pre and retried   (Good or Bad)			*/
	GmacMmcTx256To511OctetsGb	= 0x0130,	/*Tx with len >256 bytes <=511 excl. of pre and retried   (Good or Bad)			*/
	GmacMmcTx512To1023OctetsGb	= 0x0134,	/*Tx with len >512 bytes <=1023 excl. of pre and retried  (Good or Bad)			*/
	GmacMmcTx1024ToMaxOctetsGb	= 0x0138,	/*Tx with len >1024 bytes <=MaxSize excl. of pre and retried (Good or Bad)		*/
	
	GmacMmcTxUcFramesGb		= 0x013C,	/*Unicast Frames Tx 					 (Good or Bad)			*/
	GmacMmcTxMcFramesGb		= 0x0140,	/*Multicast Frames Tx				   (Good and Bad)			*/
	GmacMmcTxBcFramesGb		= 0x0144,	/*Broadcast Frames Tx 				   (Good and Bad)			*/
	GmacMmcTxUnderFlowError		= 0x0148,	/*Frames aborted due to Underflow error							*/
	GmacMmcTxSingleColG		= 0x014C,	/*Successfully Tx Frames after singel collision in Half duplex mode			*/
	GmacMmcTxMultiColG		= 0x0150,	/*Successfully Tx Frames after more than singel collision in Half duplex mode		*/
	GmacMmcTxDeferred		= 0x0154,	/*Successfully Tx Frames after a deferral in Half duplex mode				*/
	GmacMmcTxLateCol		= 0x0158,	/*Frames aborted due to late collision error						*/
	GmacMmcTxExessCol		= 0x015C,	/*Frames aborted due to excessive (16) collision errors					*/
	GmacMmcTxCarrierError		= 0x0160,	/*Frames aborted due to carrier sense error (No carrier or Loss of carrier)		*/
	GmacMmcTxOctetCountG		= 0x0164,	/*Bytes Tx excl. of preamble and retried bytes     (Good) 				*/
	GmacMmcTxFrameCountG		= 0x0168,	/*Frames Tx 				           (Good)				*/
	GmacMmcTxExessDef		= 0x016C,	/*Frames aborted due to excessive deferral errors (deferred for more than 2 max-sized frame times)*/
	
	GmacMmcTxPauseFrames		= 0x0170,	/*Number of good pause frames Tx.							*/
	GmacMmcTxVlanFramesG		= 0x0174,	/*Number of good Vlan frames Tx excl. retried frames					*/
};
enum MMC_RX
{
	GmacMmcRxFrameCountGb		= 0x0180,	/*Frames Rx 				           (Good or Bad)			*/
	GmacMmcRxOctetCountGb		= 0x0184,	/*Bytes Rx excl. of preamble and retried bytes     (Good or Bad)			*/
	GmacMmcRxOctetCountG		= 0x0188,	/*Bytes Rx excl. of preamble and retried bytes     (Good) 				*/
	GmacMmcRxBcFramesG		= 0x018C,	/*Broadcast Frames Rx 				   (Good)				*/
	GmacMmcRxMcFramesG		= 0x0190,	/*Multicast Frames Rx				   (Good)				*/
	
	GmacMmcRxCrcError		= 0x0194,	/*Number of frames received with CRC error						*/
	GmacMmcRxAlignError		= 0x0198,	/*Number of frames received with alignment (dribble) error. Only in 10/100mode		*/
	GmacMmcRxRuntError		= 0x019C,	/*Number of frames received with runt (<64 bytes and CRC error) error			*/
	GmacMmcRxJabberError		= 0x01A0,	/*Number of frames rx with jabber (>1518/1522 or >9018/9022 and CRC) 			*/
	GmacMmcRxUnderSizeG		= 0x01A4,	/*Number of frames received with <64 bytes without any error				*/
	GmacMmcRxOverSizeG		= 0x01A8,	/*Number of frames received with >1518/1522 bytes without any error			*/
	
	GmacMmcRx64OctetsGb		= 0x01AC,	/*Rx with len 64 bytes excl. of pre and retried    (Good or Bad)			*/
	GmacMmcRx65To127OctetsGb	= 0x01B0,	/*Rx with len >64 bytes <=127 excl. of pre and retried    (Good or Bad)			*/
	GmacMmcRx128To255OctetsGb	= 0x01B4,	/*Rx with len >128 bytes <=255 excl. of pre and retried   (Good or Bad)			*/
	GmacMmcRx256To511OctetsGb	= 0x01B8,	/*Rx with len >256 bytes <=511 excl. of pre and retried   (Good or Bad)			*/
	GmacMmcRx512To1023OctetsGb	= 0x01BC,	/*Rx with len >512 bytes <=1023 excl. of pre and retried  (Good or Bad)			*/
	GmacMmcRx1024ToMaxOctetsGb	= 0x01C0,	/*Rx with len >1024 bytes <=MaxSize excl. of pre and retried (Good or Bad)		*/
	
	GmacMmcRxUcFramesG		= 0x01C4,	/*Unicast Frames Rx 					 (Good)				*/
	GmacMmcRxLengthError		= 0x01C8,	/*Number of frames received with Length type field != frame size			*/
	GmacMmcRxOutOfRangeType		= 0x01CC,	/*Number of frames received with length field != valid frame size			*/
	
	GmacMmcRxPauseFrames		= 0x01D0,	/*Number of good pause frames Rx.							*/
	GmacMmcRxFifoOverFlow		= 0x01D4,	/*Number of missed rx frames due to FIFO overflow					*/
	GmacMmcRxVlanFramesGb		= 0x01D8,	/*Number of good Vlan frames Rx 							*/
	
	GmacMmcRxWatchdobError		= 0x01DC,	/*Number of frames rx with error due to watchdog timeout error				*/
	GmacMmcRxRcvError       = 0x1E0,
};
enum MMC_IP_RELATED
{
	GmacMmcRxIpcIntrMask		= 0x0200,	/*Maintains the mask for interrupt generated from rx IPC statistic counters 		*/
	GmacMmcRxIpcIntr		= 0x0208,	/*Maintains the interrupt that rx IPC statistic counters generate			*/
	
	GmacMmcRxIpV4FramesG		= 0x0210,	/*Good IPV4 datagrams received								*/
	GmacMmcRxIpV4HdrErrFrames	= 0x0214,	/*Number of IPV4 datagrams received with header errors					*/
	GmacMmcRxIpV4NoPayFrames	= 0x0218,	/*Number of IPV4 datagrams received which didnot have TCP/UDP/ICMP payload		*/
	GmacMmcRxIpV4FragFrames		= 0x021C,	/*Number of IPV4 datagrams received with fragmentation					*/
	GmacMmcRxIpV4UdpChkDsblFrames	= 0x0220,	/*Number of IPV4 datagrams received that had a UDP payload checksum disabled		*/
	
	GmacMmcRxIpV6FramesG		= 0x0224,	/*Good IPV6 datagrams received								*/
	GmacMmcRxIpV6HdrErrFrames	= 0x0228,	/*Number of IPV6 datagrams received with header errors					*/
	GmacMmcRxIpV6NoPayFrames	= 0x022C,	/*Number of IPV6 datagrams received which didnot have TCP/UDP/ICMP payload		*/
	
	GmacMmcRxUdpFramesG		= 0x0230,	/*Number of good IP datagrams with good UDP payload					*/
	GmacMmcRxUdpErrorFrames		= 0x0234,	/*Number of good IP datagrams with UDP payload having checksum error			*/
	
	GmacMmcRxTcpFramesG		= 0x0238,	/*Number of good IP datagrams with good TDP payload					*/
	GmacMmcRxTcpErrorFrames		= 0x023C,	/*Number of good IP datagrams with TCP payload having checksum error			*/

	GmacMmcRxIcmpFramesG		= 0x0240,	/*Number of good IP datagrams with good Icmp payload					*/
	GmacMmcRxIcmpErrorFrames	= 0x0244,	/*Number of good IP datagrams with Icmp payload having checksum error			*/
	
	GmacMmcRxIpV4OctetsG		= 0x0250,	/*Good IPV4 datagrams received excl. Ethernet hdr,FCS,Pad,Ip Pad bytes			*/
	GmacMmcRxIpV4HdrErrorOctets	= 0x0254,	/*Number of bytes in IPV4 datagram with header errors					*/
	GmacMmcRxIpV4NoPayOctets	= 0x0258,	/*Number of bytes in IPV4 datagram with no TCP/UDP/ICMP payload				*/
	GmacMmcRxIpV4FragOctets		= 0x025C,	/*Number of bytes received in fragmented IPV4 datagrams 				*/
	GmacMmcRxIpV4UdpChkDsblOctets	= 0x0260,	/*Number of bytes received in UDP segment that had UDP checksum disabled		*/
	
	GmacMmcRxIpV6OctetsG		= 0x0264,	/*Good IPV6 datagrams received excl. Ethernet hdr,FCS,Pad,Ip Pad bytes			*/
	GmacMmcRxIpV6HdrErrorOctets	= 0x0268,	/*Number of bytes in IPV6 datagram with header errors					*/
	GmacMmcRxIpV6NoPayOctets	= 0x026C,	/*Number of bytes in IPV6 datagram with no TCP/UDP/ICMP payload				*/
	
	GmacMmcRxUdpOctetsG		= 0x0270,	/*Number of bytes in IP datagrams with good UDP payload					*/
	GmacMmcRxUdpErrorOctets		= 0x0274,	/*Number of bytes in IP datagrams with UDP payload having checksum error		*/
	
	GmacMmcRxTcpOctetsG		= 0x0278,	/*Number of bytes in IP datagrams with good TDP payload					*/
	GmacMmcRxTcpErrorOctets		= 0x027C,	/*Number of bytes in IP datagrams with TCP payload having checksum error		*/
	
	GmacMmcRxIcmpOctetsG		= 0x0280,	/*Number of bytes in IP datagrams with good Icmp payload				*/
	GmacMmcRxIcmpErrorOctets	= 0x0284,	/*Number of bytes in IP datagrams with Icmp payload having checksum error		*/
};


enum MMC_CNTRL_REG_BIT_DESCRIPTIONS
{
	GmacMmcCounterFreeze		= 0x00000008,		/* when set MMC counters freeze to current value				*/
	GmacMmcCounterResetOnRead	= 0x00000004,		/* when set MMC counters will be reset to 0 after read				*/
	GmacMmcCounterStopRollover	= 0x00000002,		/* when set counters will not rollover after max value				*/
	GmacMmcCounterReset		= 0x00000001,		/* when set all counters wil be reset (automatically cleared after 1 clk)	*/
	
};

enum MMC_RX_INTR_MASK_AND_STATUS_BIT_DESCRIPTIONS
{
	GmacMmcRxWDInt			= 0x00800000,		/* set when rxwatchdog error reaches half of max value				*/
	GmacMmcRxVlanInt		= 0x00400000,		/* set when GmacMmcRxVlanFramesGb counter reaches half of max value		*/
	GmacMmcRxFifoOverFlowInt	= 0x00200000,		/* set when GmacMmcRxFifoOverFlow counter reaches half of max value		*/
	GmacMmcRxPauseFrameInt		= 0x00100000,		/* set when GmacMmcRxPauseFrames counter reaches half of max value		*/
	GmacMmcRxOutOfRangeInt		= 0x00080000,		/* set when GmacMmcRxOutOfRangeType counter reaches half of max value		*/
	GmacMmcRxLengthErrorInt		= 0x00040000,		/* set when GmacMmcRxLengthError counter reaches half of max value		*/
	GmacMmcRxUcFramesInt		= 0x00020000,		/* set when GmacMmcRxUcFramesG counter reaches half of max value		*/
	GmacMmcRx1024OctInt		= 0x00010000,		/* set when GmacMmcRx1024ToMaxOctetsGb counter reaches half of max value	*/
	GmacMmcRx512OctInt		= 0x00008000,		/* set when GmacMmcRx512To1023OctetsGb counter reaches half of max value	*/
	GmacMmcRx256OctInt		= 0x00004000,		/* set when GmacMmcRx256To511OctetsGb counter reaches half of max value		*/
	GmacMmcRx128OctInt		= 0x00002000,		/* set when GmacMmcRx128To255OctetsGb counter reaches half of max value		*/
	GmacMmcRx65OctInt		= 0x00001000,		/* set when GmacMmcRx65To127OctetsG counter reaches half of max value		*/
	GmacMmcRx64OctInt		= 0x00000800,		/* set when GmacMmcRx64OctetsGb counter reaches half of max value		*/
	GmacMmcRxOverSizeInt		= 0x00000400,		/* set when GmacMmcRxOverSizeG counter reaches half of max value		*/
	GmacMmcRxUnderSizeInt		= 0x00000200,		/* set when GmacMmcRxUnderSizeG counter reaches half of max value		*/
	GmacMmcRxJabberErrorInt		= 0x00000100,		/* set when GmacMmcRxJabberError counter reaches half of max value		*/
	GmacMmcRxRuntErrorInt		= 0x00000080,		/* set when GmacMmcRxRuntError counter reaches half of max value		*/
	GmacMmcRxAlignErrorInt		= 0x00000040,		/* set when GmacMmcRxAlignError counter reaches half of max value		*/
	GmacMmcRxCrcErrorInt		= 0x00000020,		/* set when GmacMmcRxCrcError counter reaches half of max value			*/
	GmacMmcRxMcFramesInt		= 0x00000010,		/* set when GmacMmcRxMcFramesG counter reaches half of max value		*/
	GmacMmcRxBcFramesInt		= 0x00000008,		/* set when GmacMmcRxBcFramesG counter reaches half of max value		*/
	GmacMmcRxOctetGInt		= 0x00000004,		/* set when GmacMmcRxOctetCountG counter reaches half of max value		*/
	GmacMmcRxOctetGbInt		= 0x00000002,		/* set when GmacMmcRxOctetCountGb counter reaches half of max value		*/
	GmacMmcRxFrameInt		= 0x00000001,		/* set when GmacMmcRxFrameCountGb counter reaches half of max value		*/
};

enum MMC_TX_INTR_MASK_AND_STATUS_BIT_DESCRIPTIONS
{

	GmacMmcTxVlanInt		= 0x01000000,		/* set when GmacMmcTxVlanFramesG counter reaches half of max value		*/
	GmacMmcTxPauseFrameInt		= 0x00800000,		/* set when GmacMmcTxPauseFrames counter reaches half of max value		*/
	GmacMmcTxExessDefInt		= 0x00400000,		/* set when GmacMmcTxExessDef counter reaches half of max value			*/
	GmacMmcTxFrameInt		= 0x00200000,		/* set when GmacMmcTxFrameCount counter reaches half of max value		*/
	GmacMmcTxOctetInt		= 0x00100000,		/* set when GmacMmcTxOctetCountG counter reaches half of max value		*/
	GmacMmcTxCarrierErrorInt	= 0x00080000,		/* set when GmacMmcTxCarrierError counter reaches half of max value		*/
	GmacMmcTxExessColInt		= 0x00040000,		/* set when GmacMmcTxExessCol counter reaches half of max value			*/
	GmacMmcTxLateColInt		= 0x00020000,		/* set when GmacMmcTxLateCol counter reaches half of max value			*/
	GmacMmcTxDeferredInt		= 0x00010000,		/* set when GmacMmcTxDeferred counter reaches half of max value			*/
	GmacMmcTxMultiColInt		= 0x00008000,		/* set when GmacMmcTxMultiColG counter reaches half of max value		*/
	GmacMmcTxSingleCol		= 0x00004000,		/* set when GmacMmcTxSingleColG	counter reaches half of max value		*/
	GmacMmcTxUnderFlowErrorInt	= 0x00002000,		/* set when GmacMmcTxUnderFlowError counter reaches half of max value		*/
	GmacMmcTxBcFramesGbInt 		= 0x00001000,		/* set when GmacMmcTxBcFramesGb	counter reaches half of max value		*/
	GmacMmcTxMcFramesGbInt 		= 0x00000800,		/* set when GmacMmcTxMcFramesGb	counter reaches half of max value		*/
	GmacMmcTxUcFramesInt 		= 0x00000400,		/* set when GmacMmcTxUcFramesGb counter reaches half of max value		*/
	GmacMmcTx1024OctInt 		= 0x00000200,		/* set when GmacMmcTx1024ToMaxOctetsGb counter reaches half of max value	*/
	GmacMmcTx512OctInt 		= 0x00000100,		/* set when GmacMmcTx512To1023OctetsGb counter reaches half of max value	*/
	GmacMmcTx256OctInt 		= 0x00000080,		/* set when GmacMmcTx256To511OctetsGb counter reaches half of max value		*/
	GmacMmcTx128OctInt 		= 0x00000040,		/* set when GmacMmcTx128To255OctetsGb counter reaches half of max value		*/
	GmacMmcTx65OctInt 		= 0x00000020,		/* set when GmacMmcTx65To127OctetsGb counter reaches half of max value		*/
	GmacMmcTx64OctInt 		= 0x00000010,		/* set when GmacMmcTx64OctetsGb	counter reaches half of max value		*/
	GmacMmcTxMcFramesInt 		= 0x00000008,		/* set when GmacMmcTxMcFramesG counter reaches half of max value		*/
	GmacMmcTxBcFramesInt 		= 0x00000004,		/* set when GmacMmcTxBcFramesG counter reaches half of max value		*/
	GmacMmcTxFrameGbInt 		= 0x00000002,		/* set when GmacMmcTxFrameCountGb counter reaches half of max value		*/
	GmacMmcTxOctetGbInt 		= 0x00000001,		/* set when GmacMmcTxOctetCountGb counter reaches half of max value		*/
	
};


typedef struct dw_gmac_dma_portmap {
  volatile uint32_t bus_mode;      // (0x00) bus mode
  volatile uint32_t tx_poll;       // (0x04) tx poll demand
  volatile uint32_t rx_poll;       // (0x08) rx poll demand
  volatile uint32_t rx_dla;        // (0x0c) rx descriptor list addr
  volatile uint32_t tx_dla;        // (0x10) tx descriptor list addr
  volatile uint32_t status;        // (0x14) status
  volatile uint32_t opmode;        // (0x18) operation mode
  volatile uint32_t intenable;     // (0x1c) interrupt enable
  volatile uint32_t missfrm_cnt;   // (0x20) missed frame and buffer overflow counter
  volatile uint32_t rxint_wdt;     // (0x24) rx interrupt watchdog timer
  volatile uint32_t axi_busmd;     // (0x28) AXI bus mode
  volatile uint32_t bus_status;    // (0x2c) AHB or AXI status
  volatile uint32_t rsvd[6];       // (0x30)
  volatile uint32_t txdsc;         // (0x44) current host tx descriptor
  volatile uint32_t rxdsc;         // (0x48) current host rx descriptor
  volatile uint32_t txbufadd;      // (0x4c) host tx buffer address
  volatile uint32_t rxbufadd;      // (0x50) host rx buffer address
  volatile uint32_t hwcfg;         // (0x54) hw feature
}DwGmacDmaPortmap;

typedef struct dw_gmac_portmap {
  volatile uint32_t mac_config;           // MAC configuration
  volatile uint32_t mac_fmfilter;         // MAC Frame filter
  volatile uint32_t hashtable_high;       // Hashtable high
  volatile uint32_t hashtable_low;        // Hashtable low
  volatile uint32_t gmii_addr;            // GMII address
  volatile uint32_t gmii_data;            // GMII data
  volatile uint32_t flow_ctrl;            // flow control
  volatile uint32_t vlan_tag;             // VLAN tag
  volatile uint32_t version;              // Vendor version
  volatile uint32_t debug;                // Debug register
  volatile uint32_t wkup_fmfilter;        // Remote wakeup frame filter
  volatile uint32_t pmt_ctrl_sts;         // PMT control and status
  volatile uint32_t lpi_ctrl_sts;         // LPI control and status
  volatile uint32_t lpitmr_ctrl;          // LPI timers control
  volatile uint32_t intsts;               // Interrupt status
  volatile uint32_t intmsk;               // Interrupt mask
  volatile uint32_t mac_addr0_high;       // MAC address0 high
  volatile uint32_t mac_addr0_low;        // MAC address0 low
  volatile uint32_t mac_addr1_high;       // MAC address1 high
  volatile uint32_t mac_addr1_low;        // MAC address1 low
  volatile uint32_t mac_addr2_high;       // MAC address2 high
  volatile uint32_t mac_addr2_low;        // MAC address2 low
  volatile uint32_t mac_addr3_high;       // MAC address3 high
  volatile uint32_t mac_addr3_low;        // MAC address3 low
  volatile uint32_t mac_addr4_high;       // MAC address4 high
  volatile uint32_t mac_addr4_low;        // MAC address4 low
  volatile uint32_t mac_addr5_high;       // MAC address5 high
  volatile uint32_t mac_addr5_low;        // MAC address5 low
  volatile uint32_t mac_addr6_high;       // MAC address6 high
  volatile uint32_t mac_addr6_low;        // MAC address6 low
  volatile uint32_t mac_addr7_high;       // MAC address7 high
  volatile uint32_t mac_addr7_low;        // MAC address7 low
  volatile uint32_t mac_addr8_high;       // MAC address8 high
  volatile uint32_t mac_addr8_low;        // MAC address8 low
  volatile uint32_t mac_addr9_high;       // MAC address9 high
  volatile uint32_t mac_addr9_low;        // MAC address9 low
  volatile uint32_t mac_addr10_high;      // MAC address10 high
  volatile uint32_t mac_addr10_low;       // MAC address10 low
  volatile uint32_t mac_addr11_high;      // MAC address11 high
  volatile uint32_t mac_addr11_low;       // MAC address11 low
  volatile uint32_t mac_addr12_high;      // MAC address12 high
  volatile uint32_t mac_addr12_low;       // MAC address12 low
  volatile uint32_t mac_addr13_high;      // MAC address13 high
  volatile uint32_t mac_addr13_low;       // MAC address13 low
  volatile uint32_t mac_addr14_high;      // MAC address14 high
  volatile uint32_t mac_addr14_low;       // MAC address14 low
  volatile uint32_t mac_addr15_high;      // MAC address15 high
  volatile uint32_t mac_addr15_low;       // MAC address15 low
  volatile uint32_t anctrl;               // AN control
  volatile uint32_t ansts;                // AN status
  volatile uint32_t autonego_adv;         // AN advertisement
  volatile uint32_t autonego_lpa;         // AN link partner ability
  volatile uint32_t autonego_expan;       // AN expansion
  volatile uint32_t tbi_ext_sts;          // TBI extended status
  volatile uint32_t sgmii_ctrl_sts;       // SGMII/RGMII/SMII C/S
  volatile uint32_t wdt;                  // Watchdog timeout
  volatile uint32_t gpio;                 // GPIO
}DwGmacPortmap;



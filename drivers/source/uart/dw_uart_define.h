#ifndef _DW_UART_DEFINE_H_
#define _DW_UART_DEFINE_H_


typedef volatile struct  _T_DW_UART_REG{
	union _RBR_THR_DLL_
	{
		REG32 RBR; 	// 0x00 32 bits R Receive Buffer Register Reset Value: 0x0 Dependencies: LCR[7] bit = 0
		REG32 THR; 	// 32 bits W Transmit Holding Register Reset Value: 0x0 Dependencies: LCR[7] bit = 0
		REG32 DLL; 	// 32 bits R/W Divisor Latch (Low) Reset Value: 0x0 Dependencies: LCR[7] bit = 1
	}DATA;
	union _DLH_IER_
	{
		REG32 DLH; 	// 0x04 32 bits R/W Divisor Latch (High) Reset Value: 0x0 Dependencies: LCR[7] bit = 1
		REG32 IER; 	// 32 bits R/W Interrupt Enable Register Reset Value: 0x0 Dependencies: LCR[7] bit = 0
	}IER_DLH;

	union _IIR_FCR_
	{
	REG32 IIR; 	// 0x08 32 bits R Interrupt Identification Register Reset Value: 0x01
	REG32 FCR; 	// 32 bits W FIFO Control Register Reset Value: 0x0
	}IIR_FCR;

	REG32 LCR;		// 0x0C 32 bits R/W Line Control Register Reset Value: 0x0
	//0x10
	REG32 MCR;		// 0x10 32 bits R/W Modem Control Register Reset Value: 0x0
	REG32 LSR;		// 0x14 32 bits R Line Status Register Reset Value: 0x60
	REG32 MSR;		// 0x18 32 bits R Modem Status Register Reset Value: 0x0
	REG32 SCR;		// 0x1C 32 bits R/W Scratchpad Register Reset Value: 0x0
	//0x20
	REG32 LPDLL;	// 0x20 32 bits R/W Low Power Divisor Latch (Low) Register Reset Value: 0x0
	REG32 LPDLH;	// 0x24 32 bits R/W Low Power Divisor Latch (High) Register Reset Value: 0x0
	REG32 Reserved_28_2C[2];	// 0x28 - 0x2C
	//0x30
	REG32 SRBR_STHR[(0x70-0x30)/4];// 0x30 - 0x6C 32 bits R Shadow Receive Buffer Register Reset Value: 0x0 Dependencies: LCR[7] bit = 0
	                        // 32 bits W Shadow Transmit Holding Register Reset Value: 0x0 Dependencies: LCR[7] bit = 0
	//0x70
	REG32 FIFOAR;	// 0x70 32 bits R/W FIFO Access Register Reset Value: 0x0
	REG32 TFR;		// 0x74 32 bits R Transmit FIFO Read Reset Value: 0x0
	REG32 RFW;		// 0x78 32 bits W Receive FIFO Write Reset Value: 0x0
	REG32 USR;		// 0x7C 32 bits R UART Status Register Reset Value: 0x6
	//0x80
	REG32 TFL;		// 0x80 See R Transmit FIFO Level Width: FIFO_ADDR_WIDTH + 1 Reset Value: 0x0
	REG32 RFL;		// 0x84 See  R Receive FIFO Level Width: FIFO_ADDR_WIDTH + 1 Reset Value: 0x0
	REG32 SRR;		// 0x88 32 bits W Software Reset Register Reset Value: 0x0
	REG32 SRTS;	// 0x8C 32 bits R/W Shadow Request to Send Reset Value: 0x0
	//0x90
	REG32 SBCR;	// 0x90 32 bits R/W Shadow Break Control Register Reset Value: 0x0
	REG32 SDMAM;	// 0x94 32 bits R/W Shadow DMA Mode Reset Value: 0x0
	REG32 SFE;		// 0x98 32 bits R/W Shadow FIFO Enable Reset Value: 0x0
	REG32 SRT;		// 0x9C 32 bits R/W Shadow RCVR Trigger Reset Value: 0x0
	//0xA0
	REG32 STET; 	//0xA0 32 bits R/W Shadow TX Empty Trigger Reset Value: 0x0
	REG32 HTX;		// 0xA4 32 bits R/W Halt TX Reset Value: 0x0
	REG32 DMASA; 	//0xA8 1 bit W DMA Software Acknowledge Reset Value: 0x0 �C
	REG32 Reserved_F0_AC[(0xF4-0xAC)/4];//0xAC - 0xF0 �C �C �C
	REG32 CPR;		// 0xF4 32 bits R Component Parameter Register Reset Value: Configuration-dependent
	REG32 UCV;		// 0xF8 32 bits R UART Component Version Reset Value: See the Releases table in the AMBA 2 release notes.
	REG32 CTR; 	// 0xFC 32 bits R Component Type Register  Reset Value: 0x44570110
} DW_UART_REG , * PDW_UART_REG;

#define DW_UART_LCR_DLAB     (1<<7)
#define DW_UART_LCR_BC	     (1<<6)

#define DW_UART_LCR_EPS		 (1<<4)
#define DW_UART_LCR_PEN      (1<<3)
#define DW_UART_LCR_STOP_1   0
#define DW_UART_LCR_STOP_1D5 (1<<2)
#define DW_UART_LCR_DLS(x) (((x-1)&0x3)>>1)

#define DW_UART_LCR_8N1		0x03
#define DW_UART_LCR_7E1		0x1A
#define DW_UART_LCR_7O1		0x0A


#define DW_UART_LSR_RFE			(1<<7)
#define DW_UART_LSR_TEMT		(1<<6)
#define DW_UART_LSR_THRE		(1<<5)
#define DW_UART_LSR_BI			(1<<4)
#define DW_UART_LSR_FE			(1<<3)
#define DW_UART_LSR_PE			(1<<2)
#define DW_UART_LSR_OE			(1<<1)
#define DW_UART_LSR_DR			(1<<0)


#define DW_UART_FCR_RCVR_2LESS		(3<<6)
#define DW_UART_FCR_RCVR_1P2	(2<<6)
#define DW_UART_FCR_RCVR_1P4	(1<<6)
#define DW_UART_FCR_RCVR_1		(0<<6)
#define DW_UART_FCR_TET_1P2		(3<<4)
#define DW_UART_FCR_TET_1P4		(2<<4)
#define DW_UART_FCR_TET_2		(1<<4)
#define DW_UART_FCR_TET_0		(0<<4)
#define DW_UART_FCR_DMAM1		(1<<3)
#define DW_UART_FCR_DMAM0		(0)
#define DW_UART_FCR_XFIFOR		(1<<2)
#define DW_UART_FCR_RFIFOR		(1<<1)
#define DW_UART_FCR_FIFOE		(1<<0)


#define DW_UART_UCR_RFF			(1<<4)
#define DW_UART_UCR_RFNE		(1<<3)
#define DW_UART_UCR_TFE			(1<<2)
#define DW_UART_UCR_TFNF		(1<<1)
#define DW_UART_UCR_BUSY		(1<<0)

#define DW_UART_MCR_LB      (1<<4)

#endif

#ifndef _MG_MCAN2_DEFINE_H_
#define _MG_MCAN2_DEFINE_H_

#include "ks_datatypes.h"

typedef struct
{
	REG32 MOD;     	     //	0x00, Mode Register
	REG32 CMR;     	     // 0x01, Command Register
	REG32 SR;     	     // 0x02, Status Register
	REG32 IR;             // 0x03, Interrupt Register
	REG32 IER;            // 0x04, Interrupt enable Register
	REG32 RESERVED0;      // 0x05, Reserved
	REG32 BTR0;           // 0x06, Bus Timing 0 Register
	REG32 BTR1;           // 0x07, Bus Timing 1 Register
	REG32 OCR;            // 0x08, Output Control Register
	REG32 RESERVED1;      // 0x09, Reserved
	REG32 RESERVED2;      // 0x0A, Reserved
	REG32 ALC;            // 0x0B, Arbitration Lost Capture Register
	REG32 ECC;            // 0x0C, Error Code Capture Register
	REG32 EWLR;           // 0x0D, Error Warning Limit Register
	REG32 RXERR;          // 0x0E, Receive Error Counter Register
	REG32 TXERR;          // 0x0F, Transmit Error Counter Register
	REG32 RXWINDOW[13];   // 0x10~0x1C, Transmit Buffer: Transmit Frame Information (0x10) + Transmit Data Information(0x11~0x1C), Read back from 0x60~0x6C
	 	 	 	 	     // 0x10~0x1C, Receive Window: Receive Frame Information (0x10) + Receive Data Information(0x11~0x1C)
	           	   	   	 // 0x10~0x13, Acceptance Code Register0~3, 0x14~0x17, Acceptance Mask Register0~3
	REG32 RMC;    		 // 0x1D, Receive Message Counter Register
	REG32 RBSA;      	 // 0x1E, Receive Buffer Start Address Register
	REG32 CDR;    		 // 0x1F, Clock Divider Register
	REG32 RXFIFO[64];     // 0x20~0x5F, Receive FIFO
	REG32 TXBUFFER[13];   // 0x60~0x6C, Transmit Buffer
	REG32 RESERVED[19];   // 0x6D~0x7F, Reserved
} MCAN2_REG , * P_MCAN2_REG;

#endif

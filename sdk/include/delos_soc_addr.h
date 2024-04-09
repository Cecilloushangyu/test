#ifndef DELOS_SOC_ADDR_H_
#define DELOS_SOC_ADDR_H_



// irq
#define IRQ_VEC_SPI_BASE		32

#define IRQ_VEC_BUS_TMO			(IRQ_VEC_SPI_BASE + 23 ) // 55
#define IRQ_VEC_GPIO_NO_SECURE	(IRQ_VEC_SPI_BASE + 22 ) // 54
#define IRQ_VEC_EMMC_WAKEUP		(IRQ_VEC_SPI_BASE + 21 ) // 53
#define IRQ_VEC_GMAC			(IRQ_VEC_SPI_BASE + 20 ) // 52
#define IRQ_VEC_CAN1			(IRQ_VEC_SPI_BASE + 19 ) // 51
#define IRQ_VEC_EMMC			(IRQ_VEC_SPI_BASE + 18 ) // 50
#define IRQ_VEC_BB0				(IRQ_VEC_SPI_BASE + 15 ) // 47
#define IRQ_VEC_UART2			(IRQ_VEC_SPI_BASE + 14 ) // 46
#define IRQ_VEC_UART3			(IRQ_VEC_SPI_BASE + 13 ) // 45
#define IRQ_VEC_DMA			    (IRQ_VEC_SPI_BASE + 12)	 // 44
#define IRQ_VEC_SQI			    (IRQ_VEC_SPI_BASE + 11)	 // 43
#define IRQ_VEC_I2C		    	(IRQ_VEC_SPI_BASE + 10 ) // 42
#define IRQ_VEC_SPIM			(IRQ_VEC_SPI_BASE + 9 ) // 41
#define IRQ_VEC_UART0			(IRQ_VEC_SPI_BASE + 8 ) // 40
#define IRQ_VEC_UART1			(IRQ_VEC_SPI_BASE + 7 ) // 39
#define IRQ_VEC_GPIO			(IRQ_VEC_SPI_BASE + 6)	// 38
#define IRQ_VEC_WDT				(IRQ_VEC_SPI_BASE + 5 ) // 37
#define IRQ_VEC_SPIS			(IRQ_VEC_SPI_BASE + 4 ) // 36
#define IRQ_VEC_CAN0			(IRQ_VEC_SPI_BASE + 3 ) // 35
#define IRQ_SRAM_ECC			(IRQ_VEC_SPI_BASE + 1 ) // 33
#define IRQ_HYPERRAM_ECC		(IRQ_VEC_SPI_BASE + 0 ) // 32

// AHB address map
#define	ROM_BASE			0x00000000
#define ROM_BASE2			0x00008000
#define	SRAM_BASE			0x00100000
#define DMAC_BASE			0x00600000
#define EMMC_BASE			0x00640000
#define APB_BASE			0x00800000
#define	GNSS_BB_BASE		0x01000000
#define	ICTL_BASE			0x02310000
#define HYP_RAM_ADDR		0x10000000

#define ROM_SIZE			0x00010000		//	128KB
#define SRAM_SIZE			0x00100000		//	1024KB
#define GNSS_BB_SIZE		0x01000000		//	16MB
#define APB_SIZE			0x00100000		//	1024KB
#define ICTL_SIZE			0x00010000		//	128KB
#define DMA_SIZE			0x00020000		//	256KB
#define EMMC_SIZE			0x00020000		//	256KB


// APB address map
//SYSCTL、GPIO和PWM为特殊外设　访问地址为原地址加0x8000
#define	SYSCTL_BASE			(APB_BASE + 0x000000+0x8000)
#define	GPIO_BASE			(APB_BASE + 0x010000+0x8000)
#define	PWM_BASE			(APB_BASE + 0x020000+0x8000)
#define SPIS_BASE			(APB_BASE + 0x030000)
#define	I2C_BASE			(APB_BASE + 0x040000)
#define	WDT0_BASE			(APB_BASE + 0x050000)
#define	WDT1_BASE			(APB_BASE + 0x060000)
#define	UART0_BASE			(APB_BASE + 0x080000)
#define	UART1_BASE			(APB_BASE + 0x090000)
#define	UART2_BASE			(APB_BASE + 0x0A0000)
#define	UART3_BASE			(APB_BASE + 0x0B0000)
#define	SQI_BASE			(APB_BASE + 0x0C0000)
#define	HYP_RAM_CTRL_BASE	(APB_BASE + 0x0D0000)
#define SPIM_BASE			(APB_BASE + 0x0E0000)
#define GMAC_BASE			(APB_BASE + 0x0F0000)
#define CAN0_BASE			UART1_BASE 
#define CAN1_BASE			UART2_BASE




#endif /* DELOS_SOC_ADDR_H_ */
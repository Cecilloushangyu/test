#ifndef KS_SYSCTRL_H_
#define KS_SYSCTRL_H_

#include "ks_driver.h"

// SYSCTL
#define	SYSCTL_PLL_CTRL			0x00
#define SYSCTL_GCLK_CTRL		0x04
#define SYSCTL_DEVCLK_EN		0x08
#define SYSCTL_SYS_RST			0x0C
#define SYSCTL_CPU_RST			0x10
#define	SYSCTL_DEV_RST			0x14
#define	SYSCTL_JTAG_EN			0x18
#define	SYSCTL_IO_ASSIGN		0x20
#define	SYSCTL_DEV_ASSIGN		0x24
#define	SYSCTL_FUNC_SEL			0x28
#define	SYSCTL_IO_MUX0			0x30
#define	SYSCTL_IO_MUX1			0x34
#define	SYSCTL_GPIO_PULL_EN		0x38
#define	SYSCTL_GPIO_PULL_DIR	0x3C
#define	SYSCTL_AP_OCMEM			0x40
#define	SYSCTL_AP_DRAM0			0x50
#define	SYSCTL_AP_DRAM1			0x54
#define	SYSCTL_AP_DRAM2			0x58
#define	SYSCTL_AP_DRAM3			0x5C
#define	SYSCTL_AP_VIO_STAT		0x60
#define	SYSCTL_AP_VIO_ADDR		0x64
#define	SYSCTL_BUS_TMO_CTL		0x70
#define	SYSCTL_BUS_TMO_ADDR		0x74
#define	SYSCTL_BUS_TMO_LADDR	0x78
#define	SYSCTL_SPUHD_EMA		0x80
#define	SYSCTL_SPRF_EMA			0x84
#define	SYSCTL_2PRF_EMA			0x88
#define	SYSCTL_DPHD_EMA			0x8C
#define SYSCTL_EFUSE_VALUE_0	0x100
#define SYSCTL_EFUSE_VALUE_1	0x104


#define	SYS_CTRL(x)				HW_REG_WR(SYSCTL_BASE + x )
#define	SYS_READ(x, value)		HW_REG_RD(SYSCTL_BASE + x , value)




#define  MUX_V_GPIO			0x00
#define  MUX_V_FUN1			0x01
#define  MUX_V_FUN2			0x02
#define  MUX_V_FUN3			0x03


#define WDT1_CLK_EN		(1 << 22)
#define WDT0_CLK_EN		(1 << 21)

#define CAN1_CLK_EN		(1 << 19)
#define PWM_CLK_EN		(1 << 18)
#define CAN0_CLK_EN		(1 << 17)

#define SPIS_CLK_EN		(1 << 16)	

#define GPIO_CLK_EN		(1 << 15)
#define I2C_CLK_EN		(1 << 14)
#define SQI_CLK_EN		(1 << 13)
#define SPIM_CLK_EN		(1 << 12)
#define UART3_CLK_EN	(1 << 11)
#define UART2_CLK_EN	(1 << 10)
#define UART1_CLK_EN	(1 << 9)
#define UART0_CLK_EN	(1 << 8)

#define EMMC_CLK_EN		(1 << 7)
#define GMAC_CLK_EN		(1 << 6)
#define DMAC_CLK_EN		(1 << 2)
#define GNSS_CLK_EN		(1 << 0)


#define CAN_OEN_INV  			(1 << 31)
#define SQI_SPIS_FUNC_SEL  		(1 << 30)
#define BOOTSTRAP  				(1 << 28)
#define RXD0_OUT_DATA  			(1 << 26)
#define RXD0_OUT_FUNC_SEL  		(1 << 25)
#define WDT1_FUNC_SEL  			(1 << 24)
#define GNSS_EVENT_SEL  		(1 << 17)
#define RF_SPI_PAGE  			(1 << 16)
#define BB_BUS_MODE  			(1 << 15)
#define CAN1_FUNC_SEL  			(1 << 14)
#define CAN0_FUNC_SEL  			(1 << 13)
#define HYPM_DQ_PULL_SEL(n)  	(n << 8)
#define SDMMC_DETECT_N  		(1 << 7)
#define SDMMC_WRITE_PROC  		(1 << 6)
#define RF_SPI_FUNC_SEL(n)		(n << 4)
#define SQI_FUNC_SEL 			(1 << 3)
#define PWM_FUNC_SEL(n) 		(n << 0)


#define WDT1_RST	(1 << 22)
#define WDT0_RST	(1 << 21)
#define CAN1_RST	(1 << 19)
#define PWM_RST		(1 << 18)
#define CAN0_RST	(1 << 17)


#define SPIS_RST	(1 << 16)	
#define GPIO_RST	(1 << 15)
#define I2C_RST		(1 << 14)

#define SQI_RST		(1 << 13)
#define SPIM_RST	(1 << 12)
#define UART3_RST	(1 << 11)
#define UART2_RST	(1 << 10)
#define UART1_RST	(1 << 9)
#define UART0_RST	(1 << 8)

#define EMMC_RST	(1 << 7)
#define GMAC_RST	(1 << 6)
#define RF_SPI_RST	(1 << 5)
#define RF_RST		(1 << 4)
#define HYPM_RST	(1 << 3)
#define DMAC_RST	(1 << 2)

#define GNSS_RST 	(1 << 0)


void ks_driver_sysctrl_set_mux_sel(U8 gpio_id, U8 fuction);
void ks_driver_sysctrl_set_clock_enable(U32 clk_id,U32 enable);
void ks_driver_sysctrl_set_fuction_sel(U32 fuction_id,U8 fuction);
void ks_driver_sysctrl_set_dev_reset(U32 rst_id,U32 enable);



#endif /* INTERFACE_INCLUDE_AP_SPI_H_ */

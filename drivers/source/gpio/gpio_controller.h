#ifndef PLATFORM_DRIVER_GPIO_GPIO_CONTROLLER_H_
#define PLATFORM_DRIVER_GPIO_GPIO_CONTROLLER_H_


#include "ks_driver.h"

// GPIO
#define	GPIO_SWPORTA_DR			0x00
#define GPIO_SWPORTA_DDR		0x04
#define	GPIO_SWPORTA_CTL		0x08
#define	GPIO_INTEN				0x30
#define GPIO_INTMASK			0x34
#define GPIO_INTTYPE_LEVEL		0x38
#define	GPIO_INT_POLARITY		0x3C
#define	GPIO_INTSTATUS			0x40
#define GPIO_RAW_INTSTATUS		0x44
#define GPIO_DEBOUNCE			0x48
#define GPIO_PORTA_EOI			0x4C
#define GPIO_EXT_PORTA			0x50
#define GPIO_INT_BOTHEDGE		0x68
#define	GPIO_CTRL(x)			HW_REG_WR(GPIO_BASE + x)
#define	GPIO_READ(x, value)		HW_REG_RD(GPIO_BASE + x, value)


void _gpio_init(U8 gpio_id);
void _set_gpio_data_sr(U8 gpio_id, U8 software_mode);
void _set_gpio_dir(U8 gpio_id, U8 dir);
void _set_gpio_dr(U8 gpio_id, U8 low);
void _set_gpio_pull_enable(U8 gpio_id, U8 pull_enable);
void _set_gpio_pull_dir(U8 gpio_id, U8 pull_dir);
void _set_gpio_irq_type_level(U8 gpio_id, U8 level);
void _set_gpio_irq_polarity(U8 gpio_id, U8 low);
void _set_gpio_irq_en(U8 gpio_id, U8 enable);
void _set_gpio_irq_mask(U8 gpio_id, U8 mask);
void _set_gpio_irq_clear(U8 gpio_id);
void _set_gpio_all_irq_clear(U32 int_status);
U32 _get_gpio_irq_status(void);
U32 _get_gpio_status(void);
U32 _get_gpio_raw_irq_status(void);
void _set_gpio_irq_bothedge(U8 gpio_id, U8 bothedge);
void _set_gpio_irq_debounce(U8 gpio_id, U8 debounce);
void _set_gpio_irq_mask_set(U32 mask);


#endif /* PLATFORM_DRIVER_GPIO_GPIO_CONTROLLER_H_ */

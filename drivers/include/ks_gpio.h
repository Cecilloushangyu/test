#ifndef INTERFACE_INCLUDE_AP_GPIO_H_
#define INTERFACE_INCLUDE_AP_GPIO_H_
#include "delos_soc_addr.h"
#include "ks_os.h"

#define	GPIOA_0					0
#define	GPIOA_1					1
#define	GPIOA_2					2
#define	GPIOA_3					3
#define	GPIOA_4					4
#define	GPIOA_5					5
#define	GPIOA_6					6
#define	GPIOA_7					7
#define	GPIOA_8					8
#define	GPIOA_9					9
#define	GPIOA_10				10
#define	GPIOA_11				11
#define	GPIOA_12				12
#define	GPIOA_13				13
#define	GPIOA_14				14
#define	GPIOA_15				15
#define	GPIOA_16				16
#define	GPIOA_17				17
#define	GPIOA_18				18
#define	GPIOA_19				19
#define	GPIOA_20				20
#define	GPIOA_21				21
#define	GPIOA_22				22
#define	GPIOA_23				23
#define	GPIOA_24				24
#define	GPIOA_25				25
#define	GPIOA_26				26

#define	GPIOA_27				27
#define	GPIOA_28				28
#define	GPIOA_29				29
#define	GPIOA_30				30
#define	GPIOA_31				31

#define	GPIOA_MAX				31

#define  GPIO_INPUT				0x00
#define  GPIO_OUTPUT      		0x01

#define  GPIO_LOW_LEVEL			0x00
#define  GPIO_HIGH_LEVEL      	0x01

#define  GPIO_PULL_DOWN      	0x00
#define  GPIO_PULL_UP			0x01


#define	TRIGGER_LOW_LEVEL 		0x00 
#define TRIGGER_HIGH_LEVEL 		0x01

#define	TRIGGER_FALLING_EDGE 	0x00  
#define TRIGGER_RISING_EDGE 	0x01

#define TRIGGER_TYPE_LEVEL  	0x00
#define TRIGGER_TYPE_EDGE   	0x01

void ks_driver_gpio_init(U8 gpio_id);
void ks_driver_gpio_set_dir(U8 gpio_id, U8 is_output);					//	1: output, 0: input
U32 ks_driver_gpio_status(void);
void ks_driver_gpio_set_output_level(U8 gpio_id, U8 output_level);		//	1: high level, 0: low level

void ks_driver_gpio_set_pull_enable(U8 gpio_id, U8 pull_enable);//	1: enable, 0: disabale
void ks_driver_gpio_set_pull_dir(U8 gpio_id, U8 pull_dir);//1: pull up, 0: pull down


//	int_type_level: 1-edge sensitive, 0-level sensitive
//  int_polarity:   1-active high,    0-active low
void ks_driver_gpio_interrupt_config(U8 gpio_id, U8 sensitive_type, U8 polarity);
void ks_driver_gpio_interrupt_bothedge(U8 gpio_id, U8 bothedge);
void ks_driver_gpio_interrupt_mask(U8 gpio_id, U8 mask);		//	1: mask, 0: unmask
void ks_driver_gpio_interrupt_mask_set( U32 mask);

void ks_driver_gpio_interrupt_enable(U8 gpio_id, U8 enable);	//	1: enable, 0: disable

//清除边沿触发有效，电平无效
void ks_driver_gpio_interrupt_clear(U8 gpio_id);
void ks_driver_gpio_interrupt_clear_all(U32 status);
U32 ks_driver_gpio_interrupt_status(void);

void ks_driver_gpio_irq_create(IRQEntry handler, void* arg);
void ks_driver_gpio_irq_enable(void);
void ks_driver_gpio_irq_disable(void);
void ks_driver_gpio_irq_vfp_enable();

#endif /* INTERFACE_INCLUDE_AP_GPIO_H_ */

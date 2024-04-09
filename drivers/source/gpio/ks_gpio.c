#include "ks_gpio.h"
#include "gpio_controller.h"



void ks_driver_gpio_init(U8 gpio_id)
{
	_gpio_init(gpio_id);
}


void ks_driver_gpio_set_dir(U8 gpio_id, U8 is_output)
{
	_set_gpio_dir(gpio_id,  ((is_output>0)?GPIO_OUTPUT:GPIO_INPUT));
}

U32 ks_driver_gpio_status(void)
{
	return _get_gpio_status();
}

void ks_driver_gpio_set_pull_enable(U8 gpio_id, U8 pull_enable)
{
	_set_gpio_pull_enable(gpio_id, pull_enable );
}

void ks_driver_gpio_set_pull_dir(U8 gpio_id, U8 pull_dir)
{
	_set_gpio_pull_dir(gpio_id, pull_dir );
}


void ks_driver_gpio_set_output_level(U8 gpio_id, U8 output_level)
{
	_set_gpio_dr(gpio_id, ((output_level>0)?GPIO_HIGH_LEVEL:GPIO_LOW_LEVEL));
}


void ks_driver_gpio_interrupt_config(U8 gpio_id, U8 sensitive_type, U8 polarity)
{
	U8 type = (sensitive_type>0?TRIGGER_TYPE_EDGE:TRIGGER_TYPE_LEVEL);
	_set_gpio_irq_type_level(gpio_id, type);
	if(type == TRIGGER_TYPE_EDGE){
		_set_gpio_irq_polarity(gpio_id, (polarity>0?TRIGGER_RISING_EDGE:TRIGGER_FALLING_EDGE));
	}else{
		_set_gpio_irq_polarity(gpio_id, (polarity>0?TRIGGER_HIGH_LEVEL:TRIGGER_LOW_LEVEL));
	}

}
void ks_driver_gpio_interrupt_bothedge(U8 gpio_id, U8 bothedge)
{
	_set_gpio_irq_bothedge(gpio_id,bothedge);

}



void ks_driver_gpio_interrupt_mask_set( U32 mask)
{
	_set_gpio_irq_mask_set(mask);
}

void ks_driver_gpio_interrupt_mask(U8 gpio_id, U8 mask)
{
	_set_gpio_irq_mask(gpio_id, mask);
}

void ks_driver_gpio_interrupt_enable(U8 gpio_id, U8 enable)
{
	_set_gpio_irq_en(gpio_id, enable);
}

void ks_driver_gpio_interrupt_clear(U8 gpio_id)
{
	_set_gpio_irq_clear(gpio_id);
}

void ks_driver_gpio_interrupt_clear_all(U32 status)
{
	_set_gpio_all_irq_clear(status);
}

U32 ks_driver_gpio_interrupt_status(void)
{
	return _get_gpio_irq_status();
}

U32 ks_driver_gpio_raw_interrupt_status(void)
{
	return _get_gpio_raw_irq_status();
}


void ks_driver_gpio_irq_create(IRQEntry handler, void* arg)
{
	ks_os_irq_map_target(IRQ_VEC_GPIO_NO_SECURE, 1);
	ks_os_irq_create(IRQ_VEC_GPIO_NO_SECURE, handler, arg);
}

void ks_driver_gpio_irq_enable(void)
{
	ks_os_irq_enable(IRQ_VEC_GPIO_NO_SECURE);
}

void ks_driver_gpio_irq_disable(void)
{
	ks_os_irq_disable(IRQ_VEC_GPIO_NO_SECURE);
}

void ks_driver_gpio_irq_vfp_enable()
{
	ks_os_irq_vfp_enable(IRQ_VEC_GPIO_NO_SECURE);
}

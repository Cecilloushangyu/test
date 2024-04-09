
#include "gpio_controller.h"
#include "ks_gpio.h"
#include "ks_sysctrl.h"


void _gpio_init(U8 gpio_id)
{

	ks_driver_sysctrl_set_mux_sel(gpio_id,MUX_V_GPIO);

}

void _set_gpio_data_sr(U8 gpio_id, U8 software_mode)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_SWPORTA_CTL, reg_value);
	if (software_mode)
		reg_value &= (~(1 << gpio_id));
	else
		reg_value |= (1 << gpio_id);
	GPIO_CTRL(GPIO_SWPORTA_CTL) = reg_value;
}

void _set_gpio_dir(U8 gpio_id, U8 dir)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_SWPORTA_DDR, reg_value);
	if (dir == GPIO_OUTPUT)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));

	GPIO_CTRL(GPIO_SWPORTA_DDR) = reg_value;
	
}

void _set_gpio_pull_enable(U8 gpio_id, U8 pull_enable)
{
	U32 reg_value = 0;
    SYS_READ(SYSCTL_GPIO_PULL_EN, reg_value);
	if (pull_enable)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));

    SYS_CTRL(SYSCTL_GPIO_PULL_EN) = reg_value;
}

void _set_gpio_pull_dir(U8 gpio_id, U8 pull_dir)
{
	U32 reg_value = 0;
    SYS_READ(SYSCTL_GPIO_PULL_DIR, reg_value);
	if (pull_dir)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));

    SYS_CTRL(SYSCTL_GPIO_PULL_DIR) = reg_value;
}

void _set_gpio_dr(U8 gpio_id, U8 level)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_SWPORTA_DR, reg_value);
	if (level)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));

	GPIO_CTRL(GPIO_SWPORTA_DR) = reg_value;
}

void _set_gpio_irq_type_level(U8 gpio_id, U8 level)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_INTTYPE_LEVEL, reg_value);
	if (level)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));
	GPIO_CTRL(GPIO_INTTYPE_LEVEL) = reg_value;
}

void _set_gpio_irq_polarity(U8 gpio_id, U8 polarity)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_INT_POLARITY, reg_value);
	if (polarity)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));

	GPIO_CTRL(GPIO_INT_POLARITY) = reg_value;
}


void _set_gpio_irq_bothedge(U8 gpio_id, U8 bothedge)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_INT_BOTHEDGE, reg_value);
	if (bothedge)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));

	GPIO_CTRL(GPIO_INT_BOTHEDGE) = reg_value;
}




void _set_gpio_irq_debounce(U8 gpio_id, U8 debounce)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_DEBOUNCE, reg_value);
	if (debounce)
		reg_value |= (1 << gpio_id);
	else
		reg_value &= (~(1 << gpio_id));

	GPIO_CTRL(GPIO_DEBOUNCE) = reg_value;
}

void _set_gpio_irq_en(U8 gpio_id, U8 enable)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_INTEN, reg_value);
	if (enable == 0)
		reg_value &= (~(1 << gpio_id));
	else
		reg_value |= (1 << gpio_id);
	GPIO_CTRL(GPIO_INTEN) = reg_value;
}

void _set_gpio_irq_mask(U8 gpio_id, U8 mask)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_INTMASK, reg_value);
	if (mask == 0)
		reg_value &= (~(1 << gpio_id));
	else
		reg_value |= (1 << gpio_id);
	GPIO_CTRL(GPIO_INTMASK) = reg_value;
}
void _set_gpio_irq_mask_set(U32 mask)
{
	GPIO_CTRL(GPIO_INTMASK) = mask;
}

void _set_gpio_irq_clear(U8 gpio_id)
{
	U32 reg_value = 0;
	GPIO_READ(GPIO_PORTA_EOI, reg_value);
	reg_value |= (1 << (gpio_id));
	GPIO_CTRL(GPIO_PORTA_EOI) = reg_value;
}

void _set_gpio_all_irq_clear(U32 int_status)
{
	GPIO_CTRL(GPIO_PORTA_EOI) = int_status;
}

U32 _get_gpio_irq_status(void)
{
	U32 int_status = 0;
	GPIO_READ(GPIO_INTSTATUS, int_status);
	return int_status;
}

U32 _get_gpio_raw_irq_status(void)
{
	U32 int_status = 0;
	GPIO_READ(GPIO_RAW_INTSTATUS, int_status);
	return int_status;
}




U32 _get_gpio_status(void)
{
	U32 int_status = 0;
	GPIO_READ(GPIO_EXT_PORTA, int_status);
	return int_status;
}

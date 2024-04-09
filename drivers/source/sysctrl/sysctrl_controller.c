
#include "ks_sysctrl.h"
#include "sysctrl_controller.h"
#include "ks_gpio.h"



void _set_dev_reset(U32 rst_id,U32 enable)
{
	U32 sysctrl_reg_value;
	SYS_READ(SYSCTL_DEV_RST, sysctrl_reg_value);
	if(enable)
	sysctrl_reg_value &= (~rst_id);
	else
	sysctrl_reg_value |= rst_id;
	SYS_CTRL(SYSCTL_DEV_RST) = sysctrl_reg_value;
}

void _set_clock_enable(U32 clk_id,U32 enable)
{
	U32 sysctrl_reg_value;
	SYS_READ(SYSCTL_DEVCLK_EN, sysctrl_reg_value);
	if(enable)
	sysctrl_reg_value |= clk_id;
	else
	sysctrl_reg_value &= (~clk_id);
	SYS_CTRL(SYSCTL_DEVCLK_EN) = sysctrl_reg_value;
}

void _set_fuction_sel(U32 fuction_id,U8 fuction)
{
	U32 temp_value;
	U32 func_mux=SYSCTL_FUNC_SEL;
	SYS_READ(func_mux, temp_value);
	if(fuction){
		temp_value |= fuction_id;
	}else{
		temp_value &= (~fuction_id);
	}

	SYS_CTRL(func_mux) = temp_value;
	
}

void _set_mux_sel(U8 gpio_id, U8 fuction)
{
	U32 temp_value;
	U32 ctrl_mux;
	U32 offset;

	if(gpio_id > GPIOA_15){
		ctrl_mux = SYSCTL_IO_MUX1;
		offset = (gpio_id - GPIOA_16)*2;
	}else{
		ctrl_mux = SYSCTL_IO_MUX0;
		offset = gpio_id*2;
	}

	
	SYS_READ(ctrl_mux, temp_value);
	
	//clean bit & set bit 
	temp_value &= (~(0x3 << offset));
	temp_value |= fuction << offset;
	SYS_CTRL(ctrl_mux) = temp_value;
	

}






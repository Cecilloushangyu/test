#include "ks_sysctrl.h"
#include "sysctrl_controller.h"


void ks_driver_sysctrl_set_mux_sel(U8 gpio_id, U8 fuction)
{

	_set_mux_sel(gpio_id,fuction);

}

void ks_driver_sysctrl_set_clock_enable(U32 clk_id,U32 enable)
{

	_set_clock_enable(clk_id,enable);

}

void ks_driver_sysctrl_set_fuction_sel(U32 fuction_id,U8 fuction)
{
	_set_fuction_sel( fuction_id, fuction);
}


void ks_driver_sysctrl_set_dev_reset(U32 rst_id,U32 enable){
	_set_dev_reset(rst_id,enable);
}


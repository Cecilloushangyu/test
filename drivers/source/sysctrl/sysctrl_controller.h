#ifndef SYS_CTRL_CONTROLLER_H_
#define SYS_CTRL_CONTROLLER_H_

#include "ks_driver.h"



void _set_mux_sel(U8 gpio_id, U8 fuction);
void _set_clock_enable(U32 clk_id,U32 enable);
void _set_fuction_sel(U32 fuction_id,U8 fuction);
void _set_dev_reset(U32 rst_id,U32 enable);


#endif /* PLATFORM_DRIVER_GPIO_MUX_CONTROLLER_H_ */

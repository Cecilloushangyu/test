

#ifndef SOURCE_DRIVERS_I2C_CONTROLLER_H_
#define SOURCE_DRIVERS_I2C_CONTROLLER_H_

#include "ks_datatypes.h"
#include "ks_i2c.h"
#include "ks_driver.h"
#include "ks_os.h"

int32_t _i2c_init(i2c_dev_t *i2c,uint32_t sel);

int32_t _i2c_config(i2c_dev_t *i2c,i2c_config_t* config);

int32_t _i2c_master_send(i2c_dev_t *i2c, uint16_t dev_addr, const void *data,
		   uint16_t size, uint32_t timeout);


int32_t _i2c_master_recv(i2c_dev_t *i2c, uint16_t dev_addr, void *data,
						   uint16_t size, uint32_t timeout);
		   
int32_t _i2c_slave_send(i2c_dev_t *i2c, const void *data, uint16_t size, uint32_t timeout);

int32_t _i2c_slave_recv(i2c_dev_t *i2c, void *data, uint16_t size, uint32_t timeout);

int32_t _i2c_finalize(i2c_dev_t *i2c);


#endif /* SOURCE_DRIVERS_I2C_CONTROLLER_H_ */

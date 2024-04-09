#include "ks_i2c.h"
#include "i2c_controller.h"
#include "ks_sysctrl.h"
#include "ks_driver.h"

static i2c_dev_t i2c_dev[CONFIG_IIC_NUM];


S32 ks_driver_i2c_init(U32 i2c_id,U32 sel)
{
	if(i2c_id!=0) return DRV_ERROR_PARAMETER;
	i2c_dev_t *i2c = &i2c_dev[0];
	i2c->port = 0;
	return _i2c_init(i2c,sel);
}


S32 ks_driver_i2c_config(U32 i2c_id,
                       i2c_config_t* config)
{
	if(i2c_id!=0) return DRV_ERROR_PARAMETER;

	i2c_dev_t *i2c = &i2c_dev[i2c_id];
		
	if(i2c->inited == 0 ) return DRV_ERROR_STATUS;
	
	memcpy(&i2c->config, config, sizeof(i2c_config_t));

	return _i2c_config(i2c,config);
}


S32 ks_driver_i2c_master_send(U32 i2c_id, U32 devaddr, const void *data, U32 num,U32 timeout)
{
	if(i2c_id!=0) return DRV_ERROR_PARAMETER;
			
	i2c_dev_t *i2c = &i2c_dev[i2c_id];
	if(i2c->inited == 0 ) return DRV_ERROR_STATUS;
	if(i2c->config.mode != IIC_MODE_MASTER ) return DRV_ERROR_STATUS;

	return _i2c_master_send(i2c,devaddr,data,num,timeout);

}


S32 ks_driver_i2c_master_receive(U32 i2c_id, U32 devaddr, void *data, U32 num,U32 timeout)
{
	if(i2c_id!=0) return DRV_ERROR_PARAMETER;
	i2c_dev_t *i2c = &i2c_dev[i2c_id];
			
	if(i2c->inited == 0 ) return DRV_ERROR_STATUS;
	if(i2c->config.mode != IIC_MODE_MASTER ) return DRV_ERROR_STATUS;

	return _i2c_master_recv(i2c,devaddr,data,num,timeout);

}

S32 ks_driver_i2c_slave_send(U32 i2c_id, const void *data, U32 num,U32 timeout)
{


	if(i2c_id!=0) return DRV_ERROR_PARAMETER;
		
	i2c_dev_t *i2c = &i2c_dev[i2c_id];
			
	if(i2c->inited == 0 ) return DRV_ERROR_STATUS;
	if(i2c->config.mode != IIC_MODE_SLAVE ) return DRV_ERROR_STATUS;

	return _i2c_slave_send(i2c,data,num,timeout);


}


S32 ks_driver_i2c_slave_receive(U32 i2c_id,  void *data, U32 num,U32 timeout)
{

	if(i2c_id!=0) return DRV_ERROR_PARAMETER;

	i2c_dev_t *i2c = &i2c_dev[i2c_id];
		
	if(i2c->inited == 0 ) return DRV_ERROR_STATUS;
	if(i2c->config.mode != IIC_MODE_SLAVE ) return DRV_ERROR_STATUS;

	return _i2c_slave_recv(i2c,data,num,timeout);

}



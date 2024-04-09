#include <stdint.h>
#include "ks_mem.h"
#include "ks_mmcsd.h"
#include "mmcsd_controller.h"
#include "ks_sysctrl.h"

void ks_driver_mmcsd_init(U32 card_type)
{
	mmcsd_init(card_type);
}


S32 ks_driver_mmcsd_get_info(mmcsd_info* pinfo){

	return mmcsd_get_info(pinfo);


}

S32 ks_driver_mmcsd_add_status_change_callback(MMCSDStatusCbk cb_func){

	return mmcsd_add_change_cb(cb_func);

}

U32  ks_driver_mmcsd_read(U32 card_type,U32 pos,void *buffer,U32 block)
{
	return mmcsd_read_recover(card_type, pos, buffer, block);


}

U32  ks_driver_mmcsd_write(U32 card_type,U32 pos,const void *buffer,U32 block)
{

	return mmcsd_write_recover(card_type, pos, buffer, block);

}



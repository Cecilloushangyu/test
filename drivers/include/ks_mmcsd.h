#ifndef KS_MMCSD_H_
#define KS_MMCSD_H_
#include <stdint.h>
#include "ks_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MMCSD_HOST_UNPLUGED     0
#define MMCSD_HOST_PLUGED       1


#define CARD_TYPE_MMC                   0 /* MMC card */
#define CARD_TYPE_SD                    1 /* SD card */

typedef void (*MMCSDStatusCbk)(U32 card_type, U32 status);

typedef struct mmcsd_info
{
	U32 card_type;
	U32 card_status;
	U32 max_data_rate;
	U32 sector_count;							/**< count of sectors */
	U32 bytes_per_sector;						/**< number of bytes per sector */
	U32 erase_size; 							/**< number of bytes to erase one block */
}mmcsd_info;

/**
 * @brief  mmcsd 初始化函数
 * @param  card_type　卡类型，目前板子只有一个sdio 接口，需要配置硬件连接，指定当前是sd还是emmc
 * @retval 无
 */

void ks_driver_mmcsd_init(U32 card_type);
/**
 * @brief  mmcsd 获取当前mmcsd卡信息
 * @param  pinfo 参数输出
 * @retval 0 OK, 非零失败
 */

S32 ks_driver_mmcsd_get_info(mmcsd_info* pinfo);

/**
 * @brief  mmcsd 状态变化回调函数
 * @param  cb_func 回调接口，卡插入还是拔出
 * @retval 无
 */
S32 ks_driver_mmcsd_add_status_change_callback(MMCSDStatusCbk cb_func);
/**
 * @brief  mmcsd 读取指定的扇区数据
 * @param  card_type　卡类型
 * @param pos　扇区号
 * @param buffer　数据buffer
 * @param block  读取长度，单位为扇区
 * @retval 实际读取的扇区个数
 */

U32 ks_driver_mmcsd_read(U32 card_type,U32 pos,void* buffer,U32 block);
/**
 * @brief  mmcsd 写入指定的扇区数据
 * @param  card_type　卡类型
 * @param pos　扇区号
 * @param buffer　数据buffer
 * @param block  写入长度，单位为扇区
 * @retval 实际写入的扇区个数
 */
U32  ks_driver_mmcsd_write(U32 card_type,U32 pos,const void *buffer,U32    block);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_INCLUDE_AP_UAH_ */

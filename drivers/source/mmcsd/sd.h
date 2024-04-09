
#ifndef __SD_H__
#define __SD_H__


#include "mmcsd_host.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t mmcsd_send_if_cond(struct mmcsd_host *host, uint32_t ocr);
int32_t mmcsd_send_app_op_cond(struct mmcsd_host *host, uint32_t ocr, uint32_t *rocr);

int32_t mmcsd_get_card_addr(struct mmcsd_host *host, uint32_t *rca);
int32_t mmcsd_get_scr(struct mmcsd_card *card, uint32_t *scr);

int32_t init_sd(struct mmcsd_host *host, uint32_t ocr);

#ifdef __cplusplus
}
#endif

#endif

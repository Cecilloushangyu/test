
#pragma  once

#include <stdint.h>
#include <stddef.h>

/* single list */
typedef struct {
	pbuf_t * head;
	pbuf_t * tail;
	uint32_t  count;
} pbuf_list_t;



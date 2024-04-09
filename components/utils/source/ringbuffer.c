
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ks_exception.h"

#include "ringbuffer.h"

#define MIN(a, b) (a)<(b)? (a) : (b)

#define _check_param(cond)   if(!(cond)) {ks_exception_assert(0);}

int ringbuffer_create(ringbuf_t *ringbuffer, char *buffer, int length)
{
    _check_param(ringbuffer && buffer);

    memset(buffer, 0, length);

    ringbuffer->length = length - 1;
    ringbuffer->ridx = 0;
    ringbuffer->widx = 0;
    ringbuffer->buffer = (uint8_t *)buffer;

    return 0;
}

void ringbuffer_destroy(ringbuf_t *ringbuffer)
{
    _check_param(ringbuffer);
    ringbuffer->buffer = NULL;
    ringbuffer->length = ringbuffer->ridx = ringbuffer->widx = 0;
}

int ringbuffer_available_read_space(ringbuf_t *ringbuffer)
{
    _check_param(ringbuffer);

    uint32_t ridx = ringbuffer->ridx;
    uint32_t widx = ringbuffer->widx;

    if (ridx == widx) {
        return 0;
    } else if (ridx < widx) {
        return widx - ridx;
    } else {
        return ringbuffer->length - (ridx - widx - 1);
    }
}

int ringbuffer_write(ringbuf_t *ringbuffer, uint8_t *data, uint32_t length)
{
    _check_param(ringbuffer && data);

    uint32_t i = 0;

    if (ringbuffer_available_write_space(ringbuffer) < length) {
        length = ringbuffer_available_write_space(ringbuffer);
    }

    for (i = 0; i < length; i++) {

        ringbuffer->buffer[ringbuffer->widx] = data[i];

        ringbuffer->widx++;

        if (ringbuffer->widx >= ringbuffer->length + 1) {
            ringbuffer->widx = 0;
        }
    }

    /* return real write len */
    return i;
}

int ringbuffer_read(ringbuf_t *ringbuffer, uint8_t *target, uint32_t amount)
{
    _check_param(ringbuffer && target);

    int copy_sz = 0;
    int i;

    if (amount == 0) {
        return -1;
    }

    if (ringbuffer_empty(ringbuffer)) {
        return 0;
    }

    /* get real read size */
    uint32_t buffer_size = ringbuffer_available_read_space(ringbuffer);
    copy_sz = MIN(amount, buffer_size);

    /* cp data to user buffer */
    for (i = 0; i < copy_sz; i++) {
        target[i] = ringbuffer->buffer[ringbuffer->ridx];

        ringbuffer->ridx++;

        if (ringbuffer->ridx >= ringbuffer->length + 1) {
            ringbuffer->ridx = 0;
        }
    }

    return copy_sz;
}


int ringbuffer_get_buffer_ridx(ringbuf_t *ringbuffer, uint8_t** p_bufferout, uint32_t length){

	_check_param(ringbuffer && p_bufferout);
	

    if (ringbuffer_empty(ringbuffer)) {
        return 0;
    }
	
    /* get real read size */
    uint32_t buffer_size = ringbuffer_available_read_space(ringbuffer);

	*p_bufferout = &ringbuffer->buffer[ringbuffer->ridx];

    return     MIN(length, buffer_size);;
}

/*dma 操作ring buffer 地址只能往上增，所以读到ring 末尾长度不能超界*/
int ringbuffer_get_dma_buffer_ridx(ringbuf_t *ringbuffer, uint8_t** p_bufferout, uint32_t length){

	_check_param(ringbuffer && p_bufferout);


	*p_bufferout = &ringbuffer->buffer[ringbuffer->ridx];

	if (ringbuffer->ridx + length>= ringbuffer->length + 1) {
		return (ringbuffer->length + 1- ringbuffer->ridx) ;
	}

    return length;
}

int ringbuffer_update_ridx(ringbuf_t *ringbuffer, uint32_t length){

	_check_param(ringbuffer );

	uint32_t i = 0;
	if (ringbuffer_available_read_space(ringbuffer) < length) {
		length = ringbuffer_available_read_space(ringbuffer);
	}

	for (i = 0; i < length; i++) {

	       ringbuffer->ridx++;

        if (ringbuffer->ridx >= ringbuffer->length + 1) {
            ringbuffer->ridx = 0;
        }
	}
    return length;
}


int ringbuffer_get_buffer_widx(ringbuf_t *ringbuffer, uint8_t** p_bufferout, uint32_t length){

	_check_param(ringbuffer && p_bufferout);
	
    if (ringbuffer_available_write_space(ringbuffer) < length) {
        length = ringbuffer_available_write_space(ringbuffer);
    }


	*p_bufferout = &ringbuffer->buffer[ringbuffer->widx];

    return length;
}




/*dma 操作ring buffer 地址只能往上增，所以写到ring 末尾，长度不能超界*/
int ringbuffer_get_dma_buffer_widx(ringbuf_t *ringbuffer, uint8_t** p_bufferout, uint32_t length){

	_check_param(ringbuffer && p_bufferout);

    if (ringbuffer_available_write_space(ringbuffer) < length) {
        length = ringbuffer_available_write_space(ringbuffer);
    }

	*p_bufferout = &ringbuffer->buffer[ringbuffer->widx];

	if (ringbuffer->widx + length>= ringbuffer->length + 1) {
		return (ringbuffer->length + 1- ringbuffer->widx) ;
	}



    return length;
}


int ringbuffer_update_widx(ringbuf_t *ringbuffer, uint32_t length){

	_check_param(ringbuffer );

	uint32_t i = 0;
	if (ringbuffer_available_write_space(ringbuffer) < length) {
		length = ringbuffer_available_write_space(ringbuffer);
	}

	for (i = 0; i < length; i++) {

		ringbuffer->widx++;

		if (ringbuffer->widx >= ringbuffer->length + 1) {
			ringbuffer->widx = 0;
		}
	}
    return length;
}



int ringbuffer_check_inrange(ringbuf_t *ringbuffer, uint8_t* p_buffer){

	if( (p_buffer>=(uint8_t*)&ringbuffer->buffer[0])
		&& (p_buffer <=(uint8_t*)&ringbuffer->buffer[ringbuffer->length])){
		return 1;
	}
	return 0;
}

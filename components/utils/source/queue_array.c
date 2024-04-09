#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ks_printf.h"
#include "queue_array.h"

int queue_array_create(queue_array *this ,char* name,unsigned int data_offset,unsigned int queue_size,unsigned int count)
{

    if(this==NULL)  return -1;
    memset(this,0, sizeof(queue_array)+ queue_size* count );
    memcpy(this->queue_name,name,strlen(name)>16?16:strlen(name));
    this->readpos = 0;
    this->writepos = 0;
	this->data_offset =data_offset;
    this->queue_size =queue_size;
    this->count = count;

    return 0;
}

int queue_array_is_full(queue_array* p_queue)
{
    if ((p_queue->writepos+1)%p_queue->count == p_queue->readpos) {
        //kprintf("buffer is full ++++++++%d  %d  %d\r\n", p_queue->writepos,p_queue->readpos,p_queue->count);
	    return   1;
    }

   return 0 ;

}

int queue_array_push(queue_array* p_queue, void*pmsg)
{

	void*src;
	void*dst;

    if(NULL == pmsg ||p_queue==NULL)
    {
        kprintf("[%s] param is error\r\n", __FUNCTION__);
        return -1;
    }

	/* Wait until buffer is not full */
    if ((p_queue->writepos+1)%p_queue->count == p_queue->readpos) {
        //kprintf("buffer is full ++++++++%d  %d  %d\r\n", p_queue->writepos,p_queue->readpos,p_queue->count);
	    return   -2;
    }

    dst = (void*)&p_queue->data[p_queue->writepos*p_queue->queue_size] ;

    src  = pmsg;

    memcpy(dst, src,p_queue->queue_size);

    p_queue->writepos++;
    if (p_queue->writepos >= p_queue->count) p_queue->writepos = 0;


    return 0;

}

int queue_array_pop(queue_array* p_queue, void* pmsg_out)
{
    void *dst;
    void *src;

	if(NULL == p_queue ||NULL==pmsg_out)
	{
    	kprintf("[%s] param is error \r\n", __FUNCTION__);
    	return -1;
	}

	// kprintf("queue_array_pop %d %d  \r\n", p_queue->writepos,p_queue->readpos);
    if(p_queue->writepos ==p_queue->readpos) {

		// TRACE_DEBUG("[%s] queue  is empty\n", __FUNCTION__);
		return -2;
    }

	dst = pmsg_out;
	if(p_queue->readpos>p_queue->count) return -3;
	
    src  = &p_queue->data[p_queue->readpos*p_queue->queue_size]  ;
	
    memcpy(dst, src,p_queue->queue_size);
	
    p_queue->readpos++;
    
    if (p_queue->readpos >=  p_queue->count) p_queue->readpos = 0;

    
	return 0;

}

int queue_array_push_buffer(queue_array* p_queue, void*pbuffer,unsigned int len,void**pbuffer_queue_out)
{
    //uint8_t *dst;
	uint8_t *src;
    uint8_t *dst;
    if(p_queue==NULL)
    {
        kprintf("[%s] param is error\r\n", __FUNCTION__);
        return -1;
    }

	/* Wait until buffer is not full */
    if ((p_queue->writepos+1)%p_queue->count == p_queue->readpos) {
        //kprintf("buffer is full ++++++++%d  %d  %d\r\n", p_queue->writepos,p_queue->readpos,p_queue->count);
	    return   -2;
    }

    dst = &p_queue->data[p_queue->writepos*p_queue->queue_size] ;
	
	if(pbuffer_queue_out!=NULL){
		*pbuffer_queue_out = dst;
	}


    src  = pbuffer;
	
	if(src!=NULL&&len>0)
    memcpy(dst, src,len<p_queue->queue_size?len:p_queue->queue_size);

    p_queue->writepos++;
    if (p_queue->writepos >= p_queue->count) p_queue->writepos = 0;


    return 0;

}

int queue_array_pop_buffer(queue_array* p_queue, void* pbuffer,unsigned int len,void**pbuffer_queue_out)
{
    void *dst;
    void *src;
	if(NULL == p_queue )
	{
    	kprintf("[%s] param is error \r\n", __FUNCTION__);
    	return -1;
	}

	// kprintf("queue_array_pop %d %d  \r\n", p_queue->writepos,p_queue->readpos);
    if(p_queue->writepos ==p_queue->readpos) {

		// TRACE_DEBUG("[%s] queue  is empty\n", __FUNCTION__);
		return -2;
    }

	dst = pbuffer;
	if(p_queue->readpos>p_queue->count) return -3;
	
    src  = &p_queue->data[p_queue->readpos*p_queue->queue_size]  ;
	if(pbuffer_queue_out != NULL)
	*pbuffer_queue_out = src;
	
	if(dst!=NULL&&len>0)
    memcpy(dst, src,len<p_queue->queue_size?len:p_queue->queue_size);
	
    p_queue->readpos++;
    
    if (p_queue->readpos >=  p_queue->count) p_queue->readpos = 0;

    
	return 0;

}




int queue_array_push_msg(queue_array* p_queue, void*pmsg,unsigned int msg_len,void* pdata,unsigned int data_len)
{
    //uint8_t *dst;
    void *src;
    void *dst;
    uint32_t len ;
    if(NULL == pmsg ||p_queue==NULL)
    {
        kprintf("[%s] param is error\r\n", __FUNCTION__);
        return -1;
    }

	/* Wait until buffer is not full */
    if ((p_queue->writepos+1)%p_queue->count == p_queue->readpos) {
        //kprintf("buffer is full ++++++++%d  %d  %d\r\n", p_queue->writepos,p_queue->readpos,p_queue->count);
	    return   -2;
    }
	len = msg_len;
	dst = &p_queue->data[p_queue->writepos*p_queue->queue_size] ;
	src  = pmsg;
	if(len > p_queue->data_offset){
		memcpy(dst, src,p_queue->data_offset);
	}
	else{
		memcpy(dst, src,len);
	}
	
	len = data_len;

    dst = &p_queue->data[p_queue->writepos*p_queue->queue_size + p_queue->data_offset] ;
	src  = pdata;

	if(len > p_queue->queue_size - p_queue->data_offset){
		memcpy(dst, src,p_queue->queue_size-p_queue->data_offset);
	}
	else{
		memcpy(dst, src,len);
	}
	
    //ks_os_printf_hex(0,dst,len);
	
    //*dst =pmsg;

    p_queue->writepos++;
    if (p_queue->writepos >= p_queue->count) p_queue->writepos = 0;


    return 0;

}


int queue_array_pop_msg(queue_array* p_queue, void* pmsg_out,unsigned int msg_len,void* pdata_out,unsigned int data_len)
{
    void *dst;
    void *src;
    uint32_t len ;
	if(NULL == p_queue ||NULL==pmsg_out)
	{
    	kprintf("[%s] param is error \r\n", __FUNCTION__);
    	return -1;
	}

	// kprintf("queue_array_pop %d %d  \r\n", p_queue->writepos,p_queue->readpos);
    if(p_queue->writepos ==p_queue->readpos) {

		// TRACE_DEBUG("[%s] queue  is empty\n", __FUNCTION__);
		return -2;
    }



	if(p_queue->readpos>p_queue->count) return -3;
	
	len = msg_len;
	dst = pmsg_out;
    src  = &p_queue->data[p_queue->readpos*p_queue->queue_size]  ;
	if(len > p_queue->data_offset){
		memcpy(dst, src,p_queue->data_offset);
	}
	else{
		memcpy(dst, src,len);
	}

	len = data_len;

	dst = pdata_out;
    src  = &p_queue->data[p_queue->readpos*p_queue->queue_size + p_queue->data_offset]  ;

	if(len > p_queue->queue_size - p_queue->data_offset){
		memcpy(dst, src,p_queue->queue_size-p_queue->data_offset);
	}
	else{
		memcpy(dst, src,len);
	}
	
    //ks_os_printf_hex(0,src,100);

    p_queue->readpos++;
    
    if (p_queue->readpos >=  p_queue->count) p_queue->readpos = 0;
    

	return 0;

}




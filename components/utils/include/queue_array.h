

#ifndef _QUEUE_ARRAY_H_
#define _QUEUE_ARRAY_H_



typedef struct queue_array
{
   char queue_name[16];
   uint32_t data_offset;
   uint32_t readpos;
   uint32_t writepos;
   uint32_t queue_size;
   uint32_t count;
   uint8_t data[0];
}queue_array;


int queue_array_create(queue_array *this ,char* name,unsigned int data_offset,unsigned int queue_size,unsigned int count);
int queue_array_push(queue_array* p_queue, void*pmsg);
int queue_array_pop(queue_array* p_queue, void* pmsg_out);
int queue_array_push_buffer(queue_array* p_queue, void*pbuffer,unsigned int len,void**pbuffer_queue_out);
int queue_array_pop_buffer(queue_array* p_queue, void* pbuffer,unsigned int len,void**pbuffer_queue_out);

int queue_array_push_msg(queue_array* p_queue, void*pmsg,unsigned int msg_len,void* pdata,unsigned int data_len);
int queue_array_pop_msg(queue_array* p_queue, void* pmsg_out,unsigned int msg_len,void* pdata_out,unsigned int data_len);

#endif 



#ifndef __PACKET_UTILS_H__
#define __PACKET_UTILS_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>





// 短整型大小端互换
#define BigLittleSwap16(A)  ((((uint16_t)(A) & 0xff00) >> 8) | \
                            (((uint16_t)(A) & 0x00ff) << 8))
 // 长整型大小端互换
 
#define BigLittleSwap32(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
                            (((uint32_t)(A) & 0x00ff0000) >> 8) | \
                            (((uint32_t)(A) & 0x0000ff00) << 8) | \
                            (((uint32_t)(A) & 0x000000ff) << 24))

unsigned long int t_htonl(unsigned long int h);
unsigned long int t_ntohl(unsigned long int n);
unsigned short int t_htons(unsigned short int h);
unsigned short int t_ntohs(unsigned short int n);

void FillNet16(uint8_t *p,uint16_t data);
void FillNet32(uint8_t *p,uint32_t data);
uint16_t GetNet16(uint8_t *pAddr);
uint32_t GetNet32(uint8_t *pAddr);


 
#endif

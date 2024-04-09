
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "dlist.h"
#include "packet_utils.h"


 // 本机大端返回1，小端返回0
int checkCPUendian()
{
   union{
          unsigned long int i;
          unsigned char s[4];
   }c;

   c.i = 0x12345678;
   return (0x12 == c.s[0]);
}
 
// 模拟htonl函数，本机字节序转网络字节序
unsigned long int t_htonl(unsigned long int h)
{
   // 若本机为大端，与网络字节序同，直接返回
   // 若本机为小端，转换成大端再返回
   return checkCPUendian() ? h : BigLittleSwap32(h);
}
 
// 模拟ntohl函数，网络字节序转本机字节序
unsigned long int t_ntohl(unsigned long int n)
{
   // 若本机为大端，与网络字节序同，直接返回
   // 若本机为小端，网络数据转换成小端再返回
   return checkCPUendian() ? n : BigLittleSwap32(n);
}
 
// 模拟htons函数，本机字节序转网络字节序
unsigned short int t_htons(unsigned short int h)
{
   // 若本机为大端，与网络字节序同，直接返回
   // 若本机为小端，转换成大端再返回
   return checkCPUendian() ? h : BigLittleSwap16(h);
}
 
// 模拟ntohs函数，网络字节序转本机字节序
unsigned short int t_ntohs(unsigned short int n)
{
   // 若本机为大端，与网络字节序同，直接返回
   // 若本机为小端，网络数据转换成小端再返回
   return checkCPUendian() ? n : BigLittleSwap16(n);
}



void FillNet32(uint8_t *p,uint32_t data)
{
	p[0] = data >> 24;
	p[1] = data >> 16;
	p[2] = data >> 8;
	p[3] = data & 0xFF;
}

void FillNet16(uint8_t *p,uint16_t data)
{

	p[0] = data >> 8;
	p[1] = data & 0xFF;
}

uint16_t GetNet16(uint8_t *pAddr)
{
    return (pAddr[0]<<8 | pAddr[1]);
}

uint32_t GetNet32(uint8_t *pAddr)
{
    return (pAddr[0]<<24 | pAddr[1]<<16
        | pAddr[2]<<8 | pAddr[3]);
}





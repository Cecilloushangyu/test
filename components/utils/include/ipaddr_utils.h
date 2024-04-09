
#ifndef __IPADDR_H__
#define  __IPADDR_H__

#include "ks_datatypes.h"


int ip_isvalued(const char *ip) ;


#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((u32_t)((d) & 0xff) << 24) | \
                         ((u32_t)((c) & 0xff) << 16) | \
                         ((u32_t)((b) & 0xff) << 8)  | \
                          (u32_t)((a) & 0xff)
                          


#define IPV4(a,b,c,d) ((((a)&0xFF)<<24)|(((b)&0xFF)<<16)|(((c)&0xFF)<<8)|((d)&0xFF))


#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

/* Here for now until needed in other places in lwIP */
#ifndef isprint
#define in_range(c, lo, up)  ((uint8_t)c >= lo && (uint8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#endif


// char -> uint 
uint32_t iputils_atou(const char *cp);
//uint  ->  char
char *iputils_utoa(const uint32_t ipaddr);
//uint  ->  char
char *iputils_ultoa(const uint32_t ipaddr);

//char  ->  hex 
int mac_atoh( char *macin, uint8_t *macout);
//hex  ->   char
char * mac_htoa(unsigned char * pMac) ;

#endif /* __LWIP_IP_ADDR_H__ */

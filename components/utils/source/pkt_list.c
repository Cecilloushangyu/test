
#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#include "dlist.h"
#include "pkt_pool.h"
#include "ks_os.h"
#include "ks_shell.h"
#include "pkt_list.h"

 void  pkt_list_initialize( pbuf_list_t * plist )
{
	plist->head = NULL;
	plist->tail = NULL;
	plist->count = 0;
	return;
}



/*
single list..
*/


void  pkt_list_add_tail( pbuf_list_t * plist, pbuf_t * pkt )
{

	/**/
	pkt->next = NULL;

	/**/
	if ( plist->head == NULL )
	{
		plist->head = pkt;
		plist->tail = pkt;
		plist->count = 1;
	}
	else
	{
		plist->tail->next = pkt;
		plist->tail = pkt;
		plist->count += 1;
	}

	return;
}

void  pkt_list_add_head( pbuf_list_t * plist, pbuf_t * pkt )
{

	/**/
	if ( plist->head == NULL )
	{
		pkt->next = NULL;
		plist->head = pkt;
		plist->tail = pkt;
		plist->count = 1;
	}
	else
	{   
	    pkt->next = plist->head;
		plist->head = pkt;
		plist->count += 1;
	}

	return;
}

 pbuf_t * pkt_list_remove_head( pbuf_list_t * plist )
{
	pbuf_t * p;

	if ( plist->head == NULL )
	{
		return NULL;
	}

	/**/
	p = plist->head;
	plist->head = p->next;


	plist->count -= 1;

	return p;
}





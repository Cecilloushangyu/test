#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "memheap.h"
#include "ks_datatypes.h"

#ifdef _cplusplus
extern "C" {
#endif


#define AARCHPTR ubase_t
#define IS_ALIGNED(value, alignSize)  (0 == (((AARCHPTR)(value)) & ((AARCHPTR)(alignSize - 1))))



typedef struct MemHeader {
	struct MemHeader* next;
	struct MemHeader* pre;
	unsigned short magic;
	unsigned short tag; //0:free, 1:alloced
    unsigned int sizereq; // 真实申请大小
	unsigned int size; //记录此块大小
	void* lptag;
}  MemHeader;

typedef struct MemTail {
	struct MemHeader* uplink; //记录此块头的地置
	unsigned short magic;
	unsigned short tag; /* must == uplink->tag */
}   MemTail;


typedef struct MemHeapCtx {
	MemHeader* head_av; //记录首块可分配的内存地址
	MemHeader* tail_av;
    char* lbound; //记录内存的下边界
	char* ubound; //记录内存的上边界
	HeapMemStaticsInfo statistics_info;
	int count; //num of nodes in list，记录内存池双向链表中可用内存的块数
	//char pad[4]; //make sizeof(this)%8==0
} MemHeapCtx;


//#define MINBLKSZ (sizeof(MemHeader)+sizeof(MemTail)+32) /* the minium mem blk to alloc */

#define MINBLKSZ (sizeof(MemHeader)+sizeof(MemTail)+64) /*可分配的最小单位为16+8+64 = 88，设置为64是因为网络包最小为64 */


ubase_t MemAddressAlignDown(ubase_t uwData, uint32_t uwAlign)
{    //alignCheck(uwAlign);
    return uwData - (uwData & (uwAlign - 1));
} 

ubase_t MemAddressAlignUp(ubase_t uwData, uint32_t uwAlign)
{   // alignCheck(uwAlign);
    return (uwData + (AARCHPTR)(uwAlign - 1)) & ~((ubase_t)(uwAlign - 1));
}



static void* g_MemHeapAddress; 



/**
 * @fn int align(int)
 * @brief 4字节对齐
 *
 * @param p
 * @return
 */
static inline int align4(int p) {
	int s = p & (int)0x03;
	if (s == 0)
		return p;
	return p + (4 - s);
}

/**
 * @fn int align(int)
 * @brief 8字节对齐
 *
 * @param p
 * @return
 */

static inline int align8(int p) {
	int s = p & (int)0x07;
	if (s == 0)
		return p;
	return p + (8 - s);
}

/*memory pool manage*/
/**
 * @fn int MemHeapinbound(MemHeapCtx*, void*)
 * @brief p是否落在PM的上限与下限之间
 *
 * @param pm
 * @param p
 * @return
 */
static int MemHeapinbound(MemHeapCtx* pm, void* p) {
	if ((ubase_t)pm->lbound <= (ubase_t)p && (ubase_t)p < (ubase_t)pm->ubound)
		return 1;
	return 0;
}

/**
 * @fn int szof_mm2hdr()
 * @brief 头大小
 *
 * @return
 */
static int szof_mm2hdr() {
	return sizeof(MemHeader);
}

/**
 * @fn int szof_mm2tal()
 * @brief 尾大小
 *
 * @return
 */
static int szof_mm2tal() {
	return sizeof(MemTail);
}

/**
 * @fn int szof_mm2ht()
 * @brief 头尾相加大小
 *
 * @return
 */
static int szof_mm2ht() {
	return szof_mm2hdr() + szof_mm2tal(); //error when use in fhcspl
}

/**
 * @fn MemTail MemHeapgettal*(MemHeader*, int)
 * @brief 返回起始地址为p，长度为sz的尾数组
 *
 * @param p
 * @param sz
 * @return
 */
static MemTail* MemHeapgettal(MemHeader* p, int sz) {
	return (MemTail*)((char*)p + sz - szof_mm2tal());
}

/**
 * @fn int MemHeapMakeNode(MemHeader*, int, int)
 * @brief 创建起始为p,长度为sz,标志位tag，填充头跟尾信息
 *
 * @param p
 * @param sz
 * @param tag
 * @return
 */
static int MemHeapMakeNode(MemHeader* p, int sz, int tag) {
	MemTail* q = NULL;

	//make head
	p->magic = ((ubase_t)p & 0xffff);
	p->tag = tag;
	p->size = sz;

	//make tail
	q = MemHeapgettal(p, p->size);
	q->uplink = p;
	q->tag = p->tag;
	q->magic = ((ubase_t)q & 0xffff);
	return 0;
}

/* get the resedue blk */
/* sz is the whole blk size, e.g (szof_mm2ht()+usr mem size) */
/**
 * @fn MemHeader MemHeapResetBlock*(MemHeader*, int)
 * @brief 改变该内存块的大小，并重新放入双向链表中
 *
 * @param p
 * @param sz
 * @return
 */
static MemHeader* MemHeapResetBlock(MemHeader* p, int sz) {
	MemHeader* p1;

	if (p->size - sz < MINBLKSZ) {
		return 0;
	}

	p1 = (MemHeader*)((char*)p + sz);
	MemHeapMakeNode(p1, p->size - sz, 0);

	return p1;
}

/**
 * @fn void MemHeapInsertAv(MemHeapCtx*, MemHeader*)
 * @brief 将p所指向内存块加入尾部
 *
 * @param pheapctx
 * @param p
 */
static void MemHeapInsertAv(MemHeapCtx* pheapctx, MemHeader* p) {
	p->next = 0;
	p->pre = 0;
	//
	if (pheapctx->head_av == 0) {
		pheapctx->head_av = p;
		pheapctx->tail_av = p;
	}
	else {
		pheapctx->tail_av->next = p;
		p->pre = pheapctx->tail_av;
		pheapctx->tail_av = p;
	}

	pheapctx->count++;
}

/**
 * @fn void MemHeapDelAv(MemHeapCtx*, MemHeader*)
 * @brief 将p所指向的内存从链表中删除
 *
 * @param pheapctx
 * @param p
 */
static void MemHeapDelAv(MemHeapCtx* pheapctx, MemHeader* p) {
	if (p->next == 0) {
		pheapctx->tail_av = p->pre;
	}
	else {
		p->next->pre = p->pre;
	}
	if (p->pre == 0) {
		pheapctx->head_av = p->next;
	}
	else {
		p->pre->next = p->next;
	}
	pheapctx->count--;
}

int MemHeapCreate(void* address, int size ) {

    void* pm;
    int pool_sz;
	// 8 对齐
    if(!IS_ALIGNED(address,8)){
    	pm= (void*)MemAddressAlignUp((ubase_t)address,8);
    }else{
    	pm = address;
    }

    if(!IS_ALIGNED(size,8)){
    	pool_sz=   MemAddressAlignDown(size,8);
    }else{
    	pool_sz = size;
     }

	g_MemHeapAddress = pm;

	MemHeapCtx* pheapctx = (MemHeapCtx*)pm;
	MemHeader* p;
	int sz;

	pheapctx->lbound = (char*)(pheapctx + 1);
	pheapctx->ubound = (char*)pm + pool_sz;

	sz = pool_sz - sizeof(MemHeapCtx);

	p = (MemHeader*)pheapctx->lbound;
	MemHeapMakeNode(p, sz, 0);

	pheapctx->head_av = 0; //init to empty state
	pheapctx->count = 0;
	MemHeapInsertAv(pheapctx, p);

	pheapctx->statistics_info.totalSize = pool_sz;
	pheapctx->statistics_info.usedSize  = 0;
    pheapctx->statistics_info.maxusedsize = 0;
	pheapctx->statistics_info.freeSize  = pool_sz;
	pheapctx->statistics_info.allocCount  = 0;
    pheapctx->statistics_info.freeCount  = 0;
    pheapctx->statistics_info.failureCount  = 0;

	
	return pool_sz;
}

/**
 * @fn void MemHeapmalloc*(void*, int)
 * @brief 分配内存
 *
 * @param pm
 * @param sz
 */
void* MemHeapMalloc(int sz,const char* callfuction ) {

	MemHeapCtx* pheapctx;
	MemHeader* p, * p1;
	int F, t;
    int sizereq = sz;
    void* pm = g_MemHeapAddress;
	if (pm == NULL ) {
		return NULL;
	}
	pheapctx = (MemHeapCtx*)pm;


	pheapctx->statistics_info.allocCount ++;

    
	p = pheapctx->head_av; //p指向首块可分配的内存地址
	if (!p){
        pheapctx->statistics_info.failureCount++;
		return NULL;

    }

    
	sz = align8(sz) + szof_mm2ht(); //需要分配的内存大小对齐并且加上首尾的大小

	F = 0;
	t = 0;

	for (p = pheapctx->head_av; p; p = p->next) { //从链表中找到第一块大于要分配的大小的内存
		if (p->size >= (unsigned int)sz) {
			F = 1;
			break;
		}
		t++;
		if (t >= 1000000) {
			//TRACE_DEBUG("malloc fail\n");
		}
	} //for

	if (!F) {
        pheapctx->statistics_info.failureCount++;
		return NULL;
	}
	MemHeapDelAv(pheapctx, p); //将p从双向链表中删除
	p1 = MemHeapResetBlock(p, sz); //重置p的大小，重置后指针为p1
	if (p1) { //如果p1的大小还够分配一次
		MemHeapInsertAv(pheapctx, p1); //将p1加入双向链表
		MemHeapMakeNode(p, sz, 1); //填充p的头尾信息
	}
	else {
		MemHeapMakeNode(p, p->size, 1); //如果p1不够分配一次大小，将p1的整个大小都分配给p，同时填充p的头尾信息
	}
	p->lptag = p;
    p->sizereq = sizereq;
   // TRACE_DEBUG("MemHeapMalloc p->size %d  sz  %d \r\n",p->size,sz);
    
	char* lp = (char*)((char*)p + szof_mm2hdr()); //将p指针偏过头的位置
    //memset(lp,0,sizereq);
	
	pheapctx->statistics_info.usedSize += sz ;
	pheapctx->statistics_info.freeSize -= sz;
    if(pheapctx->statistics_info.usedSize>pheapctx->statistics_info.maxusedsize){
        pheapctx->statistics_info.maxusedsize = pheapctx->statistics_info.usedSize;
    }
	return lp; //返回p的指针
}

/**
 * @fn void MemHeapfree(void*, void*)
 * @brief 释放内存
 *
 * @param pm
 * @param p
 */
int MemHeapFree( void* p,const char* callfuction) {

    void* pm = g_MemHeapAddress;
	if (pm == NULL || p == NULL) {
		return -1;
	}
	MemHeapCtx* pheapctx = (MemHeapCtx*)pm;
	MemHeader* p0, * pl, * pr;
	MemTail* q;
    
	if (!MemHeapinbound(pm, p))
		return -1 ;

	p0 = (MemHeader*)((char*)p - szof_mm2hdr()); //将p0指向要删除内存块的头部分

    //TRACE_DEBUG("MemHeapFree p0->size %d \r\n",p0->size);
	pheapctx->statistics_info.usedSize -= p0->size ;
	pheapctx->statistics_info.freeSize += p0->size;
	pheapctx->statistics_info.freeCount ++;
    
	/* check left block */
	if ((char*)p0 <= pheapctx->lbound)
		pl = 0; //如果p0比内存池指针的最小端还小，将P1置为0
	else {
		q = (MemTail*)((char*)p0 - szof_mm2tal()); //将q指向要释放内存块的前一块内存的尾部地址
		if (q->tag == 0) { //如果该块内存没有被使用
			pl = q->uplink; //将p1指向要释放内存前一块内存的头地址
		}
		else
			pl = 0; //如果该块内存已经被使用，将p1置为0
	}
	/* check right block */
	pr = (MemHeader*)((char*)p0 + p0->size);
	if ((char*)pr >= pheapctx->ubound)
		pr = 0;
	if (pr)
		if (pr->tag == 1)
			pr = 0;
	if (pl && pr) { //左边右边都没有被占用，都合并
		MemHeapDelAv(pheapctx, pl);
		MemHeapDelAv(pheapctx, pr);
		pl->size = pl->size + p0->size + pr->size;
		MemHeapMakeNode(pl, pl->size, 0);
		MemHeapInsertAv(pheapctx, pl);
	}
	else if (pl) { //左边没有被占用，合并左边
		MemHeapDelAv(pheapctx, pl);
		pl->size = pl->size + p0->size;
		MemHeapMakeNode(pl, pl->size, 0);
		MemHeapInsertAv(pheapctx, pl);
	}
	else if (pr) { //右边没有被占用，合并右边
		MemHeapDelAv(pheapctx, pr);
		p0->size = p0->size + pr->size;
		MemHeapMakeNode(p0, p0->size, 0);
		MemHeapInsertAv(pheapctx, p0);
	}
	else { //左右都被占用，只是把需要释放的内存放入双向链表
		MemHeapMakeNode(p0, p0->size, 0);
		MemHeapInsertAv(pheapctx, p0);
	}


    
	return 0;
}

void MemHeapStatistics(HeapMemStaticsInfo* pinfo) {
    void* pm = g_MemHeapAddress;
    
    if (pm == NULL || pinfo == NULL) {
        return;
    }
    MemHeapCtx* pheapctx = (MemHeapCtx*)pm;
    memcpy(pinfo,&pheapctx->statistics_info,sizeof(HeapMemStaticsInfo));

 } 


#ifdef _cplusplus
}
#endif


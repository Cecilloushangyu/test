
#ifndef SOURCE_PROTOCOL_EXCEPTION_PROTOCOL_H_
#define SOURCE_PROTOCOL_EXCEPTION_PROTOCOL_H_
#include "stdio.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXCEPTION_UART_ID	2

#define EXCEPTION_MSG_LEN	1024
extern char g_exception_msg[EXCEPTION_MSG_LEN];

typedef struct
{
	unsigned int sp;
	unsigned int spsr;
	unsigned int r[13];
	unsigned int lr;
	unsigned int pc;
} REG_CONTEXT, *pREG_CONTEXT;

extern REG_CONTEXT g_abt_saved_registers;

static inline void SaveRegister(void) __attribute__((always_inline));

void SaveRegister(void)
{
	register int sp asm ("r13");
	register int lr asm ("r14");
	register int r0 asm ("r0");

	asm ("push {r0-r12, r14, pc}");
	asm ("mrs r0, spsr");
	asm ("push {r0}");
	asm ("push {r13}");

	memcpy(&g_abt_saved_registers, (pREG_CONTEXT) sp, sizeof(REG_CONTEXT));
	g_abt_saved_registers.pc -= 0x8;
}

void FaultProcess();

#define FaultException(msg) \
do \
{ \
	SaveRegister(); \
	sprintf(g_exception_msg, "\r\n\r\nFaultException at %s\r\nMSG: %s\r\n", __FUNCTION__, msg); \
	FaultProcess(); \
} while (0);


void AssertProcess();

#define ks_exception_assert(EXPR)   \
if(!(EXPR))                                                                   \
{   																			\
	sprintf(g_exception_msg,"assert failed at callfunction %s  %s %d \r\n", __FUNCTION__,__FILE__,__LINE__);         \
	AssertProcess(); \
	while (1){};      \
}


void ks_exception_output_uart_set(int uart_id);
void ks_exception_project_name_set(char* project_name);

#ifdef __cplusplus
}
#endif
#endif /* SOURCE_PROTOCOL_EXCEPTION_PROTOCOL_H_ */

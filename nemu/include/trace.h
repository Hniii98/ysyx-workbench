#ifndef __TRACE_H__
#define __TRACE_H__

#include <common.h>

#ifndef CONFIG_TARGET_AM
/* Instruction trace */
#ifdef CONFIG_IRINGTRACE
#define RBUFSIZE 16

typedef struct 
{
  char ringbuf[RBUFSIZE][256]; // 256 greater than size of logbuf 
  size_t wptr;
}IRingBuf;

extern IRingBuf iringbuf;


void iringbuf_write_once(uint32_t inst, vaddr_t pc);
void iringbuf_display();
#endif

#ifdef CONFIG_MTRACE
  void mwrite_trace(vaddr_t addr, int len, word_t data);
  void mread_trace(vaddr_t addr, int len);
#endif


#endif

#endif


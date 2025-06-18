#ifndef __TRACE_H__
#define __TRACE_H__

#include <common.h>
#include <macro.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>



/* Instruction trace */
#ifdef CONFIG_IRINGTRACE

#define RBUFSIZE 16

typedef struct 
{
  char ringbuf[RBUFSIZE][256]; // 256 greater than size of logbuf 
  size_t wptr;
}IRingBuf;

void init_iringbuf();
void iringbuf_write_once(uint32_t inst, vaddr_t pc);
void iringbuf_display();

#endif

#ifdef CONFIG_MTRACE
  bool mtrace_enable(paddr_t addr);
  void mwrite_trace(paddr_t addr, int len, word_t data);
  void mread_trace(paddr_t addr, int len);
#endif

#ifdef CONFIG_FTRACE
  
  #define MAX_CALLDEPTH 10000
  
  
  typedef  MUXDEF(CONFIG_ISA64, Elf64_Ehdr, Elf32_Ehdr) Ehdr;
  typedef  MUXDEF(CONFIG_ISA64, Elf64_Shdr, Elf32_Shdr) Shdr;
  typedef  MUXDEF(CONFIG_ISA64, Elf64_Sym,  Elf32_Sym ) Sym;
  
  #define ST_TYPE(X) MUXDEF(CONFIG_ISA64, ELF64_ST_TYPE(X), ELF32_ST_TYPE(X))

  typedef struct{
    Sym *symtab;
    char *strtab;
    word_t symcnt;
  }FTraceTab;
  
  enum {ft_call = 0, ft_ret, };

  typedef struct{ 
    word_t wptr;
    char ft_logs[MAX_CALLDEPTH][128];
  }FTraceData;

  void init_ftrace(const char *elf_file);
  void free_ftracetab();
  void ftracedata_write_once(vaddr_t pc, int ft_type, vaddr_t target);
  void ftracedata_display();
  
#endif


#endif



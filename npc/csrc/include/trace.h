#ifndef __TRACE_H__
#define __TRACE_H__

#include <macro.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>


#ifdef CONFIG_FTRACE
  
  #define MAX_CALLDEPTH 100
  typedef  MUXDEF(CONFIG_ISA64, Elf64_Ehdr, Elf32_Ehdr) Ehdr;
  typedef  MUXDEF(CONFIG_ISA64, Elf64_Shdr, Elf32_Shdr) Shdr;
  typedef  MUXDEF(CONFIG_ISA64, Elf64_Sym,  Elf32_Sym ) Sym;
  
  #define ST_TYPE(X) MUXDEF(CONFIG_ISA64, ELF64_ST_TYPE(X), ELF32_ST_TYPE(X))

  typedef struct{
    Sym *symtab;
    char *strtab;
    uint32_t symcnt;
  }FTraceTab;
  
  enum {ft_call = 0, ft_ret, };

  typedef struct{ 
    uint32_t wptr;
    uint32_t rptr;
    char ftrace_logs[MAX_CALLDEPTH][128];
  }FTraceData;

  void init_ftrace(const char *elf_file);
  void free_ftracetab();
  void ftracedata_write_once(uint32_t pc, int ft_type, uint32_t target);
  void ftracedata_display_once();
  void write_uncondjump_trace(uint32_t pc, uint8_t rd, uint32_t target);
  
#endif
#endif


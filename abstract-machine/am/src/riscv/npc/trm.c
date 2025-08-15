#include <am.h>
#include <klib-macros.h>

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
  volatile uint32_t *serial = (uint32_t *)0xa00003F8;
  *serial = (uint8_t)ch;  // 写入字符到串口
}

void halt(int code) {
  asm volatile(
    "mv a0, %0\n\t"
    "ebreak"
    :
    : "r"(code)
  );
  __builtin_unreachable();
}


void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}

#ifndef __PADDR_H__
#define __PADDR_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <common.h>

/* marco defination */
#define RESET_VECTOR 0x80000000
#define CONFIG_MSIZE 0x80000000
#define CONFIG_MBASE 0x80000000


/* variable defination */

/* default img loaded to memory when img_file was not given */
extern uint32_t builtin_img[3];

/* function defination */
static inline bool in_pmem(uint32_t addr) {
	return addr - CONFIG_MBASE < CONFIG_MSIZE;
}


/* decalaration */
uint8_t* guest_to_host(uint32_t paddr);
void init_mem();
void out_of_bound(uint32_t paddr);

uint32_t paddr_read(uint32_t addr, int len);
void paddr_write(uint32_t addr, int len, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif
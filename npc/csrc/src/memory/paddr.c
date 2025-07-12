#include <common.h>
#include <paddr.h>
#include <host.h>
#include <sim.h>


uint32_t builtin_img [] = {
  0x00500093, // addi x1, x0, 5   (x1 = x0 + 5)
  0x07F10193, // addi x3, x2, 0x7F (x3 = x2 + 127)
  0x00100073, // ebreak
};

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};


/* memory address translation */
uint8_t* guest_to_host(uint32_t paddr) {return pmem + paddr - CONFIG_MBASE;}
uint32_t host_to_guest(uint8_t *haddr) {return haddr - pmem + CONFIG_MBASE;}

void out_of_bound(uint32_t addr){
	printf("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR " ] at pc = " FMT_WORD "\n",
      addr, CONFIG_MBASE, CONFIG_MBASE+CONFIG_MSIZE-1, g_current_pc);
}

/* memory access */
static uint32_t pmem_read(uint32_t addr, int len){
	uint32_t ret = host_read(guest_to_host(addr), len); 
	return ret;
}

static void pmem_write(uint32_t addr, int len,  uint32_t data){
	host_write(guest_to_host(addr), len, data);
}

/* memory initialization */
void init_mem(){
	memset(pmem, rand(), CONFIG_MSIZE);
	printf("[npc] phsical memory initialization finished\n");
}

uint32_t paddr_read(uint32_t addr, int len) {
	
	
	if(likely(in_pmem(addr))) return pmem_read(addr, len);
	out_of_bound(addr);
	return 0;
}

void paddr_write(uint32_t addr, int len, uint32_t data){
	

	if(likely(in_pmem(addr))) { pmem_write(addr, len, data); return;}
	out_of_bound(addr);
	return ;
}

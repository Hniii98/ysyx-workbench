#include <dpic_impl.h>
#include <sim.hh>
#include <paddr.h>



uint32_t  npc_pmem_read(uint32_t raddr){
	printf(ANSI_FMT("\tnpc: read four bytes of memory at " FMT_PADDR, ANSI_FG_WHITE) "\n",
        raddr);
	return paddr_read(raddr, 4);
}


void npc_pmem_write(uint32_t waddr, uint32_t wdata, uint8_t wmask){
	printf(ANSI_FMT("\tnpc: write data {" FMT_WORD "} to memory at address " FMT_PADDR ".", ANSI_FG_WHITE),
        wdata,
        waddr); 
	if(wmask == 0x3u) {
		paddr_write(waddr, 1, wdata & 0xFF); // write lowwest one byte
		printf("Data lenth is 1 bytes.\n");
	}
	else if(wmask == 0xFu) {
		paddr_write(waddr, 2, wdata & 0xFFFF); // write lowwest two bytes
		printf("Data lenth is 2 bytes.\n");
	}
	else if(wmask == 0xFFu) { 
		paddr_write(waddr, 4, wdata); // write all four bytes
		printf("Data lenth is 4 bytes.\n");
	}

	printf("Invalid write data  mask '" FMT_BYTE "', should be one of '0x3' or '0xF'"  
			"or '0xFF' \n", wmask);
	
}

#include <dpic_impl.h>
#include <sim.hh>
#include <paddr.h>



uint32_t  npc_pmem_read(uint32_t raddr){
	/* 
	   To simplify, always read four bytes data in raddr to verilator .
	   corresponding data slice and extend process done in memory.v.
	   Memory read in npc in combinational logic, it will execute many
	   times before being stable, which makes log confused. So we remove
	   memory read trace. 
	*/
	uint32_t rdata = paddr_read(raddr, 4);
	return rdata;
}


void npc_pmem_write(uint32_t waddr, uint32_t wdata, uint8_t wmask){

	if(waddr >= SERIAL_ADDR && waddr <= SERIAL_END) { 
		putchar(wdata);
		return;
	} 

	switch (wmask) {
		case 0x3u:  // one bytes
			paddr_write(waddr, 1, wdata );
			IFDEF(CONFIG_MTRACE, 
					printf(ANSI_FMT("\tnpc: write data { " FMT_BYTE " } to memory at address " FMT_PADDR ".\n", ANSI_FG_GREEN),
						(uint8_t)wdata, waddr););
			break;
		case 0xfu:  // two bytes
			paddr_write(waddr, 2, wdata );
			IFDEF(CONFIG_MTRACE, 
					printf(ANSI_FMT("\tnpc: write data { " FMT_HALFWORD " } to memory at address " FMT_PADDR ".\n", ANSI_FG_GREEN),
						(uint16_t)wdata, waddr););
			break;
		case 0xffu: // four bytes
			paddr_write(waddr, 4, wdata); 
			IFDEF(CONFIG_MTRACE, 
					printf(ANSI_FMT("\tnpc: write data { " FMT_WORD " } to memory at address " FMT_PADDR ".\n", ANSI_FG_GREEN),
						wdata, waddr););
			break;
		default:
			printf("Invalid write mask " FMT_BYTE "\n", wmask);
}

}

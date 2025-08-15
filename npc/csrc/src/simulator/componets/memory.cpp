#include <dpic_impl.h>
#include <sim.hh>
#include <paddr.h>
#include <utils.h>

#define SERIAL_ADDR 0xa00003F8
#define SERIAL_END  (SERIAL_ADDR + 8)
#define TIMER_UPTIME_ADDR  0xa0000048  
#define TIMER_UPTIME_HIGH  0xa000004C  



uint32_t  npc_pmem_read(uint32_t raddr){
	/* 
	   To simplify, always read four bytes data in raddr to verilator .
	   corresponding data slice and extend process done in memory.v.
	   Memory read in npc in combinational logic, it will execute many
	   times before being stable, which makes log confused. So we remove
	   memory read trace. 
	*/
	if (raddr >= TIMER_UPTIME_ADDR && raddr < TIMER_UPTIME_ADDR + 8) {
        uint64_t time_us = get_time();
        if (raddr == TIMER_UPTIME_ADDR) {
            return (uint32_t)(time_us & 0xFFFFFFFF); 
        } else if (raddr == TIMER_UPTIME_HIGH) {
            return (uint32_t)(time_us >> 32);       
        }
    } 
    
    // 
    return paddr_read(raddr, 4);
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

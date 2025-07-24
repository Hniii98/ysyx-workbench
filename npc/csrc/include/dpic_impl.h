#ifndef __DIPC_IMPL_H__
#define __DIPC_IMPL_H__


#ifdef __cplusplus
extern "C" {
#endif
	#include <stdbool.h>
	#include <inttypes.h>
	/* verilator to  C */
	/* do not forget to set scope when using */
	uint32_t npc_send_gprval(uint32_t index);
	uint32_t npc_send_nextpc();
	uint8_t  npc_send_rd();
	uint8_t  npc_send_is_uncondjump();

	/* C to verilator */
	void npc_reach_ret(int code);
	uint32_t  npc_pmem_read(uint32_t raddr);
	void npc_pmem_write(uint32_t waddr, uint32_t wdata, uint8_t wmask);
	
	
	

#ifdef __cplusplus
}
#endif



#endif
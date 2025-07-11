#ifndef __DIPC_IMPL_H__
#define __DIPC_IMPL_H__


#ifdef __cplusplus
extern "C" {
#endif
	#include <inttypes.h>
	/* verilator to  C*/
	uint32_t npc_send_gprval(uint32_t index);
	uint32_t npc_send_inst();

	/* C to verilator */
	void npc_reach_ret(int code);
	
	

#ifdef __cplusplus
}
#endif



#endif
#ifndef __DIPC_IMPL_H__
#define __DIPC_IMPL_H__


#ifdef __cplusplus
extern "C" {
#endif
	#include <inttypes.h>
	//  verilator export function
	uint32_t npc_send_gprval(uint32_t index);

	//  verilator dpi-c import
	void npc_reach_ret(int code);
	

#ifdef __cplusplus
}
#endif



#endif
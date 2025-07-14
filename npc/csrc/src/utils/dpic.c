#include <dpic_impl.h>
#include <utils.h>
#include <sim.h>

void npc_reach_ret(int code) {
	g_npc_state.state = NPC_END;
	g_npc_state.halt_ret = code;
	g_npc_state.halt_pc = g_frozen_pc;	
}



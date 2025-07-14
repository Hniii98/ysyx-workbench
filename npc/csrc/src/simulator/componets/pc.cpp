#include <dpic_impl.h>
#include <sim.hh>


uint32_t sim_get_nextpc(){
	npc_set_scope("pc");
	return npc_send_nextpc();
}
#include <dpic_impl.h>
#include <sim.hh>


uint32_t sim_get_dnpc(){
	npc_set_scope("pc");
	return npc_send_dnpc();
}
#include <dpic_impl.h>
#include <sim.hh>


uint8_t sim_get_rd(){
	npc_set_scope("control");
	return npc_send_rd();
}

bool  sim_get_is_uncondjump(){
	npc_set_scope("control");
	return npc_send_is_uncondjump() == 1; // convert uint8_t to bool	
}  
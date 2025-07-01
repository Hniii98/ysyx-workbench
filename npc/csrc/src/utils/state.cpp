#include <utils.h>


NPC_STATE npc_sate = {.state = NPC_STOP};

int is_exit_status_bad(){
	/* manual quit or program excute end*/
	int good = (npc_state.state == NPC_END && npc_state.halt_ret == 0) || npc_state.state == NPC_QUIT; 
	return !good;
}
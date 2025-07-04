#include <utils.h>


NPC_STATE npc_state = {.state = NPC_STOP};

int is_exit_status_bad(){
	/* 
		if exit from user mannual quit command or program reach ret 
		with exit code 0, exit status is good! Otherwise, npc meet 
		unexpected behavior, like npc implementation wrong.
	*/
	int good = (npc_state.state == NPC_END && npc_state.halt_ret == 0) 
		|| npc_state.state == NPC_QUIT; 
	
	return !good;
}
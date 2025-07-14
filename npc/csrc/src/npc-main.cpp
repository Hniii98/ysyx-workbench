#include <sim.h>
#include <macro.h>

extern "C"{
	void init_monitor(int argc, char *argv[]);
	void sdb_mainloop();
	int is_exit_status_bad();
	void free_ftracetab();
}

int main(int argc, char *argv[]){

	/* init verilator related parameter*/
	sim_init();

	/* init global parameter */
	init_monitor(argc, argv);

	/* start sdb, waiting command from user */
	sdb_mainloop();

	/* receive quit command or program exit*/
	sim_exit();

	/* free resource for ftracedata*/
	IFDEF(CONFIG_FTRACE, free_ftracetab());

	/* detect exit status */
	return is_exit_status_bad();
}
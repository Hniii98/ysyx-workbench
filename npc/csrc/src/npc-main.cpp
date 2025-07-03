#include <common.h>

void init_monitor(int argc, char *argv[]);
void sim_init();
void sim_exit();
void sdb_mainloop();
int is_exit_status_bad();



int main(int argc, char *argv[]){
	/* init global parameter */
	init_monitor(argc, argv);

	/* init verilator related parameter*/
	sim_init();

	/* start sdb, waiting command from user */
	sdb_mainloop();

	/* receive quit command or program exit*/
	sim_exit();

	/* detect exit status */
	return is_exit_status_bad();
}
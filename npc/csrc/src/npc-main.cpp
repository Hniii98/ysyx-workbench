#include <common.h>

void init_monitor(int argc, char *argv[]);

int main(int argc, char *argv[]){

	init_monitor(argc, argv);

	sim_init();

	simulator_clean();


	return is_exit_status_bad();

}
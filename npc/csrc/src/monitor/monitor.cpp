#include <getopt.h>
#include <common.h>
#include <paddr.h>

static char *img_file = NULL;

void sdb_set_batch_mode();

static int parse_argument(int argc, char *argv[]){
	const struct option table[] = {
		{"batch"	, no_argument		, 	NULL, 'b'},
		{"help"		, no_argument		, 	NULL, 'h'},
		{0			, 0					,	NULL,  0 },	
	};
	int o;
	while( (o = getopt_long(argc, argv, "-bh", table, NULL)) != -1){
		switch (o)
		{
		case 'b': sdb_set_batch_mode(); 	break;
		case   1: img_file = optarg;		return 0;
		default:
			printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
			printf("\t-b,--batch              run with batch mode\n");
			printf("\n");
			exit(0);
		}
	}
	return 0;
}


static long load_img(){
	if(img_file == NULL){
		printf("[NPC] loading built in img to npc \n");
		memcpy(guest_to_host(RESET_VECTOR), builtin_img, sizeof(builtin_img));
		return 4096;  // force to align to 4KB
	}

	FILE *fp = fopen(img_file, "rb");
	assert(fp);

	/* measure size */
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);

	/* load img */
	fseek(fp, 0, SEEK_SET);
	int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
	assert(ret == 1);

	fclose(fp);
	return size;
}



void init_monitor(int argc, char *argv[]){
	
	//
	parse_argument(argc, argv);

	//
	init_rand();

	//
	init_mem();

	//
	long img_size = load_img();

}
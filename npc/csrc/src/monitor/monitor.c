#include <getopt.h>
#include <paddr.h>
#include <trace.h>
#include <macro.h>

static char *img_file = NULL;
void init_disasm(const char *triple);
void sdb_set_batch_mode();
void init_disasm(const char *triple);
void init_ftrace(const char* elf_file);


static char *elf_file = NULL;

static int parse_argument(int argc, char *argv[]){
	const struct option table[] = {
		{"batch"	, no_argument		, 	NULL, 'b'},
		{"elf"      , required_argument , 	NULL, 'e'},
		{"help"		, no_argument		, 	NULL, 'h'},
		{0			, 0					,	NULL,  0 },	
	};
	int o;
	while( (o = getopt_long(argc, argv, "-bhe:", table, NULL)) != -1){
		switch (o)
		{
		case 'b': sdb_set_batch_mode(); 	break;
		case 'e': elf_file = optarg;		break;
		case   1: img_file = optarg;		return 0;
		default:
			printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
			 printf("\t-e,--elf=FILE           input elf file to npc\n");
			printf("\t-b,--batch              run with batch mode\n");
			printf("\n");
			exit(0);
		}
	}
	return 0;
}


static long load_img(){
	if(img_file == NULL){
		printf("[npc] loading built in img to npc \n");
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
	
	// parse argument from options
	parse_argument(argc, argv);

	// init rand seed
	init_rand();

	// init memory by random value
	init_mem();

	// load image to memory
	long img_size = load_img();

	// init disassemble
	init_disasm("riscv32" "-pc-linux-gnu");

	IFDEF(CONFIG_FTRACE, init_ftrace(elf_file));

}
#include <getopt.h>
#include <paddr.h>
#include <trace.h>
#include <macro.h>


void init_disasm(const char *triple);
void sdb_set_batch_mode();
void init_disasm(const char *triple);
void init_ftrace(const char* elf_file);
void init_difftest(const char* ref_so_file, long img_size, int port);

static char *img_file = NULL;
static char *elf_file = NULL;
static char *diff_so_file = NULL;
static int difftest_port = 1234;


static int parse_argument(int argc, char *argv[]){
	const struct option table[] = {
		{"batch"	, no_argument		, 	NULL, 'b'},
		{"elf"      , required_argument , 	NULL, 'e'},
		{"diff"		, required_argument	, 	NULL, 'd'},
		{"help"		, no_argument		, 	NULL, 'h'},
		{0			, 0					,	NULL,  0 },	
	};
	int o;
	while( (o = getopt_long(argc, argv, "-bhe:d:", table, NULL)) != -1){
		switch (o)
		{
		case 'b': sdb_set_batch_mode(); 	break;
		case 'e': elf_file		= optarg;	break;
		case 'd': diff_so_file	= optarg;	break;
		case   1: img_file 		= optarg;	return 0;
		default:
			printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
			printf("\t-e,--elf=FILE           input elf file to npc\n");
			printf("\t-d,--diff=FILE          input diff so file to npc\n");		
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

	printf("[npc] loading '%s' to npc, size: %lu KB \n", img_file, size/1024);

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
	IFDEF(CONFIG_FTRACE, init_disasm("riscv32" "-pc-linux-gnu"););
	// init function trace
	IFDEF(CONFIG_FTRACE, init_ftrace(elf_file));

	// init .so file for differential testing
	IFDEF(CONFIG_DIFFTEST, init_difftest(diff_so_file, img_size, difftest_port ));

}
#include <common.h>
#include <sdb.h>
#include <sim.h>
#include <paddr.h>
#include <reg.h>
#include <readline/history.h>
#include <readline/readline.h>

static bool is_batch_mode = false;

void sdb_set_batch_mode(){
	is_batch_mode = true;
}

/* read from stdin */
static char *rl_gets() {
	static char *line_read = NULL;

	if (line_read) { 
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(npc) ");

	if(line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_help(char *args); // show help information
static int cmd_si(char *args); // step instruction
static int cmd_x(char *args); // scan memory
static int cmd_info(char *args); // display information about given args
static int cmd_q(char *args); // exit npc
static int cmd_c(char *args); // continue the execution of program
 
static struct {
	const char *name;
	const char *description;
	int (*handler) (char *);
} cmd_table[] = {
	{"help", "Display information about all supported commands.", cmd_help},
	{"q", "Exit.", cmd_q},
	{"info", "Display information about given args, only support 'info r' now.", cmd_info},
	{"c", "Continue excuting the program.", cmd_c},
	{"x", "Display N times four bytes memory from given position.", cmd_info},
	{"si", "Move n  steps and pause, n default set to 1.", cmd_si},
	
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args){
	/* extract argument */
	char *arg = strtok(NULL, " ");

	/* show all supported commands */
	if(arg == NULL){
		for(int i = 0; i < NR_CMD; i++){
			printf("%-10s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
		return 0;
	}
	/* only show about given arg */
	else{
		for(int i = 0; i < NR_CMD; i++){
			if(strcmp(arg, cmd_table[i].name) == 0){
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
	}
	/* unknown command */
	printf("Unknown command '%s' \n", arg);
	return 0;
}


static bool is_pure_number(char *str){
	if(!str) return false;

	while(str){
		if(*str < '0' || *str > '9') return false;
		str++;
	}

	return true;
}

static int cmd_si(char *args){
	char *arg = strtok(NULL, " ");
	int t;

	/* set t to defualt 1. */	
	if(arg == NULL) { t = 1;}
	/* set t to arg */
	else if(is_pure_number(arg)) { t = atoi(arg);}
	/* arg is not a pure number */
	else {
		printf("Illegal argument '%s', shoule be a number \n", arg);
		return 0;	
	}

	/* excute t times */
	npc_exec(t);
	return 0;
}

static int cmd_x(char *args){
	char *times, *addr;

	times = strtok(NULL, " ");
	addr  = strtok(NULL, " ");

	if(times == NULL || addr == NULL){
		printf("Missing argument, should give both times and address \n");
		printf("example: x 10 0x80000000\n");
	}

	uint32_t pmem_addr = strtoul(addr, NULL, 0);
	/* invali phsical memory address */
	if(!in_pmem(pmem_addr)){
		printf("Invalid memory address, should in [" FMT_PADDR ", " FMT_PADDR " ]", 
			CONFIG_MBASE, CONFIG_MBASE+CONFIG_MSIZE-1);
		return 0;
	}

	uint8_t *start = guest_to_host(pmem_addr);
	uint32_t n = strtoul(times, NULL, 0);

	/* read size over memory bound */
	if(pmem_addr - CONFIG_MBASE + 4*n >= CONFIG_MSIZE) { 
		out_of_bound(pmem_addr); 
		return 0;
	}

	/* display bytes */
	printf("{");
	for(int i = 0; i < n; i++){
		uint32_t value;
		memcpy(&value, start, 4); // read 4bytes
		printf(FMT_WORD, value);
		if(i != n-1) {printf(", ");}
		start += 4; // move forward 4 bytes
	}
	printf("}\n");

	return 0;
}

static int cmd_info(char *args){
	char *arg = strtok(NULL, " ");

	if(strcmp(arg, "r") == 0) {
		npc_reg_display();
	}
	/* TODO: support more infomation like watchpoint ..*/
	else{
		printf("Unknown arg '%s' \n", arg);
		printf("Only support 'info r'\n");
		return 0;
	}
	return 0;
}


static int cmd_q(char *args){
	npc_state.state = NPC_QUIT;
	return -1;

}

static int cmd_c(char *args){
	npc_exec(-1);
	return 0;
}

void sdb_mainloop(){
	if(is_batch_mode){
		cmd_c(NULL);
		return;
	}

	for(char *str; (str = rl_gets()) != NULL; ){
		char *str_end = str + strlen(str);

		char *cmd = strtok(str, " ");
		/* empty command line input */
		if(cmd == NULL) { continue; }

		char *args = cmd + strlen(cmd) + 1;
		/* arguments empty */
		if(args >= str_end) { args = NULL;}

		int i;
		for(i = 0; i < NR_CMD; i++){
			if(strcmp(cmd, cmd_table[i].name) == 0){
				if(cmd_table[i].handler(args) < 0){
					/* received cmd_q */
					return;
				}
				break; // only match once
			}
		}

		/* no match command */
		if(i == NR_CMD){ printf("Unknown command '%s' \n", cmd); }
	}
}


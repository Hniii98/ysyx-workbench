/***************************************************************************************
 ** Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "sdb.h"
#include <cpu/cpu.h>
#include <isa.h>
#include <readline/history.h>
#include <readline/readline.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_d(char *args);
static int cmd_w(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Pause after N times execution of the instructions", cmd_si},
    {"info", "Display information about all registers", cmd_info},
    {"x", "Display N times 4 bytes start from memory position EXP", cmd_x},
    {"p", "Evaluate a given expression", cmd_p},
    {"d", "Delete watch point no in using", cmd_d},
	{"w", "Insert a watchpoint", cmd_w},
    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int is_purenum(char *str){
	if(!str) return -1;
	
	while(str){
		if(*str < '0' || *str > '9') return -1;
		str++;
	}
	return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  int t; // execute times, maybe overflow if CL input large number

  if (arg == NULL) {
    /* no argument given, t default value equals 1 */
    t = 1;
    cpu_exec(1);
  } else {
    /* excute t times if args are pure number */
    if (is_purenum(args)) {
      t = atoi(args);
      cpu_exec(t);
    } else {
      printf("Unknown arg '%s'\n", arg);
      return -1;
    }
  }
  return 0;
}

static int cmd_x(char *args) {
  char *arg_n, *arg_exp;
  /* extract first argument */
  arg_n = strtok(NULL, " ");
  if (arg_n == NULL) {
    /* no argument N given */
    printf("No argument N given\n");
    return -1;
  } else {
    /* extract second argument */
    arg_exp = strtok(NULL, " ");
    if (arg_exp == NULL) {
      printf("No argument EXPR given\n");
      return -1;
    }
  }

  /* parse argument */
  paddr_t pmem_addr;                     // paddr_t = uint32_t
  pmem_addr = strtoul(arg_exp, NULL, 0); // str to ul(32 bit)
  if (pmem_addr < CONFIG_MBASE) {
    printf("Invalid pmem address");
    return -1;
  }
  uint8_t *start = guest_to_host(pmem_addr); // return a memory ptr
  paddr_t n;
  n = strtoul(arg_n, NULL, 0);
  word_t step = sizeof(word_t);

  /* make sure don't out of bound */
  if (pmem_addr - CONFIG_MBASE + n * step >
      CONFIG_MSIZE) { // display 4byte at each time
    printf("out of pmem bound");
    return -1;
  }

  /* display byte information */
  printf("{");
  for (int i = 0; i < n; i++) {
    word_t value;
    memcpy(&value, start, sizeof(value));
    printf(FMT_WORD, value);
    if (i != n - 1)
      printf(", ");

    pmem_addr += step;
    start += step;
  }
  printf("}\n");

  return 0;
}

static int cmd_p(char *args) {
  /* first argument */
  char *arg = strtok(NULL, "\n");

  /* empty argument */
  if (arg == NULL) {
    printf("No argument given \n");
    return -1;
  }

  bool success = true;
  word_t result = 0;

  result = expr(arg, &success);
  if (success) {
    printf("%" PRIu32 "\n",
           result); // simplified to think all result are uint32_t
  }
  return 0;
}

static int cmd_info(char *args) {
  /* fisrt argument */
  char *arg = strtok(NULL, " ");
  /* parse argument */
  if (strcmp(arg, "r") == 0)
    isa_reg_display();
  /* TODO: parse more argument, like register ..*/
  else if(strcmp(arg, "w") == 0){
	 wp_in_head_display();
  }
  else {
    printf("Unknown arg '%s'\n", arg);
    return -1;
  }
  return 0;
}

  /* /1* first argument behind command *1/ */
  /* char *arg = strtok(NULL, " "); */
  /* if (!arg) { */
  /*   printf("Empty argument\n"); */
  /*   return -1; */
  /* } */
/* } */

static int cmd_d(char *args) {
  char *arg = strtok(NULL, " ");
  if (!arg) {
    printf("Empty argument\n");
    return -1;
  }

  char *endptr = NULL;
  unsigned long no = strtoul(arg, &endptr, 0);
  if (*endptr != '\0') {
    printf("Invalid argument: %s, need a number.\n", arg);
    return -1;
  }
  /* return the WP of NO=no */
  WP *to_free = detach_wp(no);
  if (to_free) {
    free_wp(to_free);
  }
  return 0;
}

static int cmd_w(char *args){
	char *arg = strtok(NULL, " ");
	if(!arg){
		printf("Empty argument \n");
		return -1;
	}

	bool success = true;
	word_t val = expr(arg, &success);
	if(success){
		WP *wp = new_wp();
		use_wp(wp);
		if(wp){
			memcpy(wp->expr, args, strlen(args)+1);
			wp->recorded = val;
		}	
		else { 
			printf("wp allocate failed, equal to NULL"); 
			return -1;
		}
	}
	
	return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

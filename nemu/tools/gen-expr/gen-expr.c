/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
/* TODO:compose these to a struct state */
int buf_idx = 0;

uint32_t choose(uint32_t n){
	return rand() % n;
}

static void gen(char c){
	/* buf array overflow, do nothing */
	if(buf_idx >= 65535) return;

	buf[buf_idx++] = c;
}

static void gen_num(){
	/*
	char num = '0' + choose(10); // generate '0' to '9'
	if(num == '0' && buf_idx > 0 &&  buf[buf_idx-1] == '/') num = '1'; // avoid / followed with 0 
	*/
	switch(choose(3)){
		case 0: 
			/* gen single digits */
			gen('0'+choose(10));
			break;
		case 1:
			/* gen tens digits */
			gen('1'+choose(9)); // avoid considerd as otcal num
			gen('0'+choose(10));
			break;
		case 2:
			/* gen hundreds digits */
			gen('1'+choose(9));
			gen('0'+choose(10));
			gen('0'+choose(10));
			break;
	}			
	gen('u'); // make sure expr computes in unsigned way.
}

static void gen_rand_op(){
	char op_array[] = "+-*/";

	gen(op_array[choose(4)]);
}

static void gen_rand_expr(){
	if(buf_idx >= 65535) return; 

	switch(choose(3)){
		case 0: gen_num(); break;
		case 1: gen('('); gen_rand_expr(); gen(')'); break;
		default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
	}
	/* make sure expr legal, only gen space in the border */
	if(choose(10) < 1) gen(' '); // p = 0.1
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
	buf_idx = 0;
    gen_rand_expr();
	buf[buf_idx] = '\0'; // make buf end

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

	/* if there is warning, return is not 0 */
    int ret = system("gcc -Wall -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}

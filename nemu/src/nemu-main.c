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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
void test_expr_gen();
word_t expr(char *e, bool *success);
void test_isa_reg_str2val();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
#ifdef CONFIG_LONGEXPR
  test_expr_gen();
#endif
  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}

void test_expr_gen(){
	uint32_t line_number = 0, given_val = 0, expr_result = 0;
	uint32_t illegal_num = 0, wa_num = 0;
	char given_expr[MAX_TOKENS] = {};

	FILE *fp = fopen("/home/hniii98/Project/ysyx/ysyx-workbench/nemu/tools/gen-expr/input", "r");
	assert(fp != NULL);

	while(fscanf(fp, "%u %[^\n]", &given_val, given_expr) == 2){
		bool success = true;
		line_number++;

		expr_result = expr(given_expr, &success);
		if(!success){
			printf("ILLEGAL EXPRESSION: Line: %u, expression: %s\n",
					line_number,
					given_expr);
			illegal_num++;
			continue;
		}
		
		if(expr_result != given_val){
			printf("WRONG ANSWER: Line: %u, expression: %s, given_val: %u, expr return: %u\n",
					line_number,
					given_expr,
					given_val,
					expr_result);
			wa_num++;
		}
		memset(given_expr, 0, sizeof(given_expr));
	}

	if(illegal_num == 0 && wa_num == 0){
		printf("exprssion function testing finished, everything go well so far\n");
	}
	else{
		printf("something wrong with function expr, illegal expression num: %u, wrong answer expression num: %u.",
				illegal_num,
				wa_num);
	}
}	


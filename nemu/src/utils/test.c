#include <common.h>
#include <test.h>
#include "../monitor/sdb/sdb.h"

//word_t expr(char *e, bool *success);


void test_expr_gen(){
	uint32_t line_number = 0, given_val = 0, expr_result = 0;
	uint32_t illegal_num = 0, wa_num = 0;
	char given_expr[MAX_TOKENS] = {};

	FILE *fp = fopen("tools/gen-expr/input", "r");
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
		printf("Exprssion function testing finished, everything go well so far :)\n");
	}
	else{
		printf("Something wrong with function expr(), illegal expression num: %u, wrong answer expression num: %u.",
				illegal_num,
				wa_num);
	}

	fclose(fp);
}	



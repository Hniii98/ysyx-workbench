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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>


// max op_type number, for making ranks_map and aritys_map
#define MAX_OP_TYPE 255 + 100
typedef struct watchpoint{
	int NO;
	struct watchpoint *next;
	char expr[MAX_TOKENS];
	word_t recorded;
}WP;
word_t expr(char *e, bool *success);
uint8_t* guest_to_host(paddr_t paddr);
void use_wp(WP *wp);
void free_wp(WP *wp);
WP *new_wp();
WP *detach_wp(unsigned int no);
void wp_in_head_display();

#endif

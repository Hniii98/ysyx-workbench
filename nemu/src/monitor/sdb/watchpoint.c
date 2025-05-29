/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

#define NR_WP 32


static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    memset(wp_pool[i].expr, 0, sizeof(wp_pool[i].expr));
    wp_pool[i].recorded = 0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

/* Return a available node to use */
WP *new_wp() {
  if (!free_)
    assert(0); // no free node to use

  // free_ point to first available node 
  WP *tmp = free_;
  free_ = free_->next;

  // detach node */
  tmp->next = NULL;

  return tmp;
}

/* Free wp, and join it to free_ list */
void free_wp(WP *wp) {
  //free_ list empty, wp become the first node 
  if (!free_) {
    free_ = wp;
    return;
  }
  //insert from the head 
  if(wp){
	  wp->next = free_->next;
	  free_->next = wp;
  }
}

/* Use given wp to watch, join it to head list  */
void use_wp(WP *wp) {
  // empty list, wp become head 
  if (!head) {
    head = wp;
    return;
  }

  /* insert from the head */
  wp->next = head->next;
  head->next = wp;
}

/* Detach NO watchpoint  from head list  */
WP *detach_wp(unsigned int no) {
  /* invalid NO value, must in [0, NR_WP-1] */
  if (no >= NR_WP) {
    printf("Invalid watchpoint NO, out of max watchpoint number\n");
    return NULL;
  }

  WP dummy_head = {0};
  dummy_head.next = head;
  WP *itr = &dummy_head;
  
  while(itr->next){
	  if(itr->next->NO == no){
		  WP *tmp = itr->next;
		  itr->next = itr->next->next;
		  head = dummy_head.next;
		  tmp->next = NULL;
		  return tmp;
	  }
	  itr = itr->next;
  }
  /* itrate head list over, not found responsicv NO watchpoint*/
  printf("Watchpoint no is not in head list, can't remove\n");
  return NULL;
}

int load_expr_and_cal(WP *wp, char *arg) {
  if (!wp || (wp->NO < 0) || (wp->NO >= NR_WP)) {
    printf("Invalid watchpoint wp, can't load expression to it\n");
    return -1;
  }

  memcpy(wp->expr, arg, strlen(arg) + 1);

  bool success = true;
  word_t value = expr(arg, &success);
  if (!success) {
    printf("Invalid expression: %s", arg);
    return -1;
  }
  /*evaluate expression after saving in wp , record it*/
  wp->recorded = value;
  return 0;
}

void wp_in_head_display(){
	if(!head){
		printf("Exist none watchpoint\n");
		return;
	}
	
	WP *itr = head;
	printf("%-4s %-10s %-s\n", "Num", "What", "Value");
	while(itr){
		printf("%-4d %-s %-u\n", itr->NO, itr->expr, itr->recorded);
		itr = itr->next;
	}
	return;
}
		

void watchpoint_difftest() {
  WP *itr = head;

  while (itr) {
    bool success = true;
    word_t new_value = expr(itr->expr, &success);
    if (success && itr->recorded != new_value) {
      printf("watchpoint %d : %s\n"
             "Old value = %d    \n"
             "New value = %d    \n",
             itr->NO, itr->expr, itr->recorded, new_value);
      itr->recorded = new_value;
      nemu_state.state = NEMU_STOP;
      return;
    }
    //itr++;
	itr = itr->next;
  }
}

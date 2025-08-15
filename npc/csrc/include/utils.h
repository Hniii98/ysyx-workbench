#ifndef __UTILS_H__
#define __UTILS_H__



#ifdef __cplusplus
extern "C" {
#endif

#include "trace.h"
// state
enum { NPC_RUNNING, NPC_STOP, NPC_END, NPC_ABORT, NPC_QUIT };

typedef struct {
	int state;
	uint32_t halt_pc;
	uint32_t halt_ret;
#ifdef CONFIG_ITRACE
	char logbuf[128];
#endif
} NPC_STATE;

extern NPC_STATE g_npc_state;
// ANSI color

#define ANSI_FG_BLACK   "\33[1;30m"
#define ANSI_FG_RED     "\33[1;31m"
#define ANSI_FG_GREEN   "\33[1;32m"
#define ANSI_FG_WHITE   "\33[1;37m"
#define ANSI_FG_BLUE    "\33[1;34m"

#define ANSI_BG_BLACK   "\33[1;40m"
#define ANSI_BG_RED     "\33[1;41m"
#define ANSI_BG_GREEN   "\33[1;42m"
#define ANSI_BG_BLUE    "\33[1;44m"
#define ANSI_BG_WHITE   "\33[1;47m"

#define ANSI_NONE       "\33[0m"

/* ANSI_NONE 确保每次fmt生效完后流处理恢复默认设置 */
#define ANSI_FMT(str, fmt) fmt str ANSI_NONE


/* timer */
void init_rand();
uint64_t get_time();



#ifdef __cplusplus
}
#endif


#endif
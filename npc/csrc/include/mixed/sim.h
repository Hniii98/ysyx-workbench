#ifndef __SIM_H__
#define __SIM_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

void npc_exec(uint64_t n);
void sim_init();
void sim_exit();

#ifdef __cplusplus
}
#endif


#endif
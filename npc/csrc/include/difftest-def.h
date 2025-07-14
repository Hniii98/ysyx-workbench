#ifndef __DIFFTEST_DEF_H__
#define __DIFFTEST_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

typedef struct {
	uint32_t gpr[32];
	uint32_t pc;
} CPU_state;


extern void (*ref_difftest_memcpy)(uint32_t addr, void *buf, size_t n, bool direction);
extern void (*ref_difftest_regcpy)(void *dut, bool direction);
extern void (*ref_difftest_exec)(uint64_t n);


void difftest_step();
void init_difftest(const char* ref_so_file, long img_size, int port);



#ifdef __cplusplus
}
#endif

#endif
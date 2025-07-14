#pragma once

#include <cinttypes>

void npc_set_scope(const char* name);
bool sim_get_is_uncondjump();
uint8_t sim_get_rd();
uint32_t sim_get_nextpc();
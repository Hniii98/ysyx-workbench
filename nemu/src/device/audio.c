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
#include <device/map.h>
#include <SDL2/SDL.h>


enum {
  reg_freq = 0,
  reg_channels,
  reg_samples,
  reg_sbuf_size, // read only for am, 64KB.
  reg_init,
  reg_count, // read only for am
  nr_reg = 6 
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static void sdl_audio_callback(void *userdata, Uint8* stream, int len) {
  int used = audio_base[reg_count];
  
 // if(used < 0 || used > CONFIG_SB_SIZE) used = 0; // illegal situation, simply mute it.

  int copy = (len < used) ? len : used; // safe copy size
  memcpy(stream, sbuf, copy);

  if (len >= used) {
    memset(stream + used, 0, len - used);
    audio_base[reg_count] = 0;
  } else {
    memmove(sbuf, sbuf + len, used - len);
    audio_base[reg_count] -= len;
  }
}

static void init_sdl(){
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS;
  s.userdata = NULL;
  s.freq = audio_base[reg_freq];
  s.channels = audio_base[reg_channels];
  s.samples = audio_base[reg_samples];
  s.callback = sdl_audio_callback;
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  
  int success = SDL_OpenAudio(&s, NULL);
  if (success < 0) {
      printf("SDL_OpenAudio failed: %s\n", SDL_GetError());
      return;
  }
  SDL_PauseAudio(0);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if(!is_write) return; // read option don't need any optio
  if(is_write && offset == 0x10 && audio_base[reg_init]) {
      init_sdl();
      audio_base[reg_init] = 0;
  }
}

static void audio_consumer_handler(uint32_t offset, int len, bool is_write) {
  assert(audio_base[reg_count] + len <= audio_base[reg_sbuf_size]);
  if (is_write) {
    audio_base[reg_count] += len;
  } 
  
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
  audio_base[reg_freq]     = 8000;
  audio_base[reg_channels] = 1;
  audio_base[reg_samples]  = 1024;
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  audio_base[reg_init] = 0;
  audio_base[reg_count] = 0;
  

#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, audio_consumer_handler);
  memset((uint8_t *)sbuf, 0, (size_t)CONFIG_SB_SIZE-1);
}

#include <am.h>
#include <nemu.h>
#include <klib-macros.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

void __am_audio_init() {
  // this may slowdown system, we rely on reg_count for initialization
  // uint8_t *dst = (uint8_t *)(uintptr_t)AUDIO_SBUF_ADDR;
  // memset(dst, 0, io_read(AM_AUDIO_CONFIG).bufsize);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR); // read only.
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {

  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1); // inform hardware update parameters.                        
  
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR); // read only.
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  // spin lock
  size_t len = (uint8_t *)ctl->buf.end - (uint8_t *)ctl->buf.start;
  while((io_read(AM_AUDIO_CONFIG).bufsize - io_read(AM_AUDIO_STATUS).count)
          < len);

  // may data race here
  uint8_t *dst = (uint8_t *)(uintptr_t)(AUDIO_SBUF_ADDR + io_read(AM_AUDIO_STATUS).count);
  uint8_t *src = (uint8_t *)(uintptr_t)ctl->buf.start;
  memcpy(dst, src, len);
}

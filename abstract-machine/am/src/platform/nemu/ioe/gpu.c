#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)


void __am_gpu_init() {
  
  AM_GPU_CONFIG_T cfg;
  ioe_read(AM_GPU_CONFIG, &cfg);

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (int i = 0; i < cfg.height * cfg.width; i++) {
    fb[i] = ~AM_GPU_NULL; 
  }

  outl(SYNC_ADDR, 0);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {

  uint32_t whbound = inl(VGACTL_ADDR);
  int wbound = (int)(whbound >> 16);  
  int hbound = (int)(whbound & 0xffff);

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = wbound, .height = hbound,
    .vmemsz = wbound * hbound * sizeof(uint32_t),
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  AM_GPU_CONFIG_T cfg;
  ioe_read(AM_GPU_CONFIG, &cfg);
  uint32_t wbound = cfg.width;
  uint32_t hbound = cfg.height;

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *px = (uint32_t *)(uintptr_t)ctl->pixels;

  int x_end = ctl->x + ctl->w;
  int y_end = ctl->y + ctl->h;
  if (x_end > wbound) x_end = wbound;
  if (y_end > hbound) y_end = hbound;

  for (int y = ctl->y; y < y_end; y++) {
    for (int x = ctl->x; x < x_end; x++) {
      int fb_pos = y * wbound + x;
      int px_pos = (y - ctl->y) * ctl->w + (x - ctl->x);
      fb[fb_pos] = px[px_pos];
    }
  }


  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}



void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}

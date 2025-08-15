#include <am.h>
#include <time.h>

#define TIMER_LO_ADDR  0xa0000048  // 计时器低32位
#define TIMER_HI_ADDR  0xa000004C  // 计时器高32位


void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
    uint32_t lo = *(volatile uint32_t *)TIMER_LO_ADDR;  
    uint32_t hi = *(volatile uint32_t *)TIMER_HI_ADDR;  
    uptime->us = ((uint64_t)hi << 32) | lo;             
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  /* Simulate rtc time via uptime */
  uint32_t lo = *(volatile uint32_t *)TIMER_LO_ADDR;  
  uint32_t hi = *(volatile uint32_t *)TIMER_HI_ADDR;  
  uint64_t us = ((uint64_t)hi << 32) | lo; 
  uint64_t sec = us / 1000000;

  // start from 2025-08-015 00:00:00 to simulate
  rtc->second = sec % 60;
  rtc->minute = (sec / 60) % 60;
  rtc->hour   = (sec / 3600) % 24;
  rtc->day    = ((sec / 86400)  + 15) % 31;   // start from 15.
  rtc->month  = ((sec / 2592000) + 8) % 12; // start form 12.
  rtc->year   = 2025 + sec / 31536000;    // start from 2025.
}

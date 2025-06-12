#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void *mempcpy(void *out, const void *in, size_t n);
void *memcpy(void *out, const void *in, size_t n);
void *memset(void *s, int c, size_t n);

size_t strlen(const char *s) {
  
  if (s == NULL) return 0; 
  size_t len =  0; 
  const char *ptr = s;
  while(*ptr != '\0'){
    len++;
    ptr++;
  }
  return len;
}

char *stpcpy(char *dst, const char *src){
  char *p;
  p = mempcpy(dst, src, strlen(src));
  *p = '\0';

  return p;
}

char *strcpy(char *dst, const char *src) {
  /* user is responsible for allocating large enough buffer for dst*/
  stpcpy(dst, src);
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  /* User should make sure they do not overlap*/
  size_t dst_len = strlen(dst);
  size_t src_len = strlen(src);

  if(dst_len > src_len){
    memset(mempcpy(dst, src, src_len), 0, dst_len-src_len);
  }
  else if(dst_len < src_len){
    memcpy(dst, src, dst_len); // truncate
  }
  else{
    memcpy(dst, src, src_len);
  }

  return dst;
}

char *strcat(char *dst, const char *src) {
  stpcpy(dst + strlen(dst), src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  const char *ptr_s1 = s1, *ptr_s2 = s2;

  /* Compare each char of two string*/
  while(*ptr_s1 && *ptr_s2){
    if(*ptr_s1 == *ptr_s2){
      ptr_s1++;
      ptr_s2++;
    }
    else{
      return *ptr_s1 - *ptr_s2;
    }      
  }

  /*one or both string have reach end, return comparsion value*/
  return *ptr_s1 - *ptr_s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  const char *ptr_s1 = s1, *ptr_s2 = s2;

  for(size_t i = 0; i  < n; i++){
    if(*ptr_s1 == '\0' || *ptr_s2 == '\0') break;
    else{
      if(*ptr_s1 == *ptr_s2){
        ptr_s1++;
        ptr_s2++;
      }
      else{
        return *ptr_s1 - *ptr_s2;
      }     
    }    
  }

  /* earl break because of reaching end */
  if(*ptr_s1 == '\0' || *ptr_s2 == '\0') return *ptr_s1 - *ptr_s2;

  /* fisrt n bytes of s1 and s2 are equal*/
  return 0;
}

void *memset(void *s, int c, size_t n) {
  char byte = (char)c; //truncate if overflow
  char *ptr = s;
  for(size_t i = 0; i < n; i++){
    *ptr = byte;
    ptr++;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  /* The area may overlap*/
  if(dst < src){
    /* dst is on left, copy from head to tail*/
    const uint8_t *from = src;
    uint8_t *to   = dst;

    for(size_t i = 0; i < n; i++){
      *to = *from;
      to++, from++;
    }
  }
  else if(dst > src){
   /* dst is on left, copy from tail to head*/ 
    uint8_t *from = (uint8_t *)src + n - 1;
    uint8_t *to   = (uint8_t *)dst + n - 1; // point to rear

    for(size_t i = 0; i < n; i++){
      *to = *from;
      to--, from--;
    }
  }
  /* if dst == src, return directly*/
  return dst;  
  
}

void *memcpy(void *out, const void *in, size_t n) {
  /* The area must not overlap*/
  const uint8_t *from = in;
  uint8_t *to = out;
  for(size_t i = 0; i < n; i++){
    *to = *from;
    from++;
    to++;
  }
  return out;
}

void *mempcpy(void *out, const void *in, size_t n){
    /* The area must not overlap*/

  uint8_t *from = (uint8_t*)in;
  uint8_t *to = (uint8_t*)out;
  for(size_t i = 0; i < n; i++){
    *to = *from;
    from++;
    to++;
  }
  /* tp point to the byte after n byte of out */
  return to;  
}

int memcmp(const void *s1, const void *s2, size_t n) {
  if(n == 0) return 0;

  const char *ptr_s1 = s1, *ptr_s2 = s2;
  for(size_t i = 0; i < n; i++){
    if(*ptr_s1 == *ptr_s2){
      ptr_s1++;
      ptr_s2++;
    }
    else{
      return *ptr_s1-*ptr_s2;
    }
  }

  /*all byte of s1 and s2 are same (intepreter as unsigned char) in for loop*/
  return 0;
}

#endif

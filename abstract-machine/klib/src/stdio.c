#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)



int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

static void int_to_str(int num, char *str);
int sprintf(char *out, const char *fmt, ...) {
  char *ptr = out;  
  va_list ap;
  va_start(ap, fmt);


  char *s;
  int d;
  
  while(*fmt != '\0'){

    switch (*fmt){
      case '%' : /* format character  */ 
        fmt++; // we need the next flag

        if(*fmt == 's'){ /* string */
          s = NULL;
          s = va_arg(ap, char *);
          ptr = mempcpy(ptr, s, strlen(s));  
        } 
        else if(*fmt == 'd'){ /* int */
          char buf[100] = {0}; // large enough to store int character 
          d = va_arg(ap, int);
          int_to_str(d, buf);
          ptr = mempcpy(ptr, buf, strlen(buf));
        }
        else{
          /* error occur, invalid flag, return a negative number*/
          return -1;
        }

        break;

      default : /* normal character */
        *ptr++ = *fmt;  // copy directly
        break;
    }

    fmt++; // process next character
  }
 

  /* append terminated character*/
  *ptr = '\0';

  va_end(ap);

  return strlen(out);
  
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}


/* Transfer a int number to corresponding string including sign */
static void int_to_str(int num, char *str){
  int i = 0, sign = num;
  if(num < 0) num = -num; /* get the absolute value*/

  /* transfer number to char form LSB to MSB */
  do{
    str[i++] = num % 10 + '0';
  }while((num/=10) > 0);

  if(sign < 0) str[i++] = '-';
  str[i] = '\0';

  /* reverse str */
  for(int j = 0; j < i/2; j++){
    char tmp = str[j];
    str[j] = str[i-j-1];
    str[i-j-1] = tmp;
  }
}

#endif

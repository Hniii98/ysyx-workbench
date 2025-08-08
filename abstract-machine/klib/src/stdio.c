#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

typedef struct{  
  char *buf; 
  size_t pos;
  size_t size;
} BufContext;
// TODO: compose all emitter context to BufContext

typedef void (*EmitFunc) (char ch, void *context); // callback


/* Decide where a character emit to */
static void stream_emitter(char ch, void *context){
  putch(ch); // only support stdout for now.
}

static void buf_emitter(char ch, void *context){
  BufContext *c = (BufContext *)context;

  if(c->pos < c->size-1) c->buf[c->pos++] = ch; // safely writing
}

static void null_emitter(char ch, void *context){
  /* Do nothing about emit, only countng */
  (*(size_t *)context)++;
}


/* A wrapper of emitter */
static int emit_number(EmitFunc emit, void *context, int num, int base){
  char buf[64];
  int len = 0;
  if(num < 0){
    emit('-', context);
    num = -num;
    len++;
  }

  do {
    buf[len++] = "0123456789abcdef"[num % base];  
  } while( num /= base );

  for(int i = len-1; i >=0; i--) emit(buf[i], context);
  return len;
}

static int emit_string(EmitFunc emit, void *context, const char* str){
  int len = 0;
  while(*str){
    emit(*str++, context); 
    len++;
  }

  return len;
}

/* Core format process function */
static int format_core(EmitFunc emit, void *context, const char *fmt, va_list ap){
  int total = 0;

  while(*fmt){
    /* Fast path */
    if(*fmt != '%'){
      emit(*fmt++, context); // directly emit a chararcter.
      total++;
      continue;
    }

    /* Slow path */
    fmt++; // skip '%' for now

    if (*fmt == '\0') { // boundary check
      emit('%', context);
      total++;
      break;
    }

    char spec = *fmt++; // character after '%'
    switch (spec) {
      case '%': // "%%" should output '%'
        emit('%', context);
        total++;
        break;

      case 'd':
        total += emit_number(emit, context, va_arg(ap, int), 10);
        break;

      case 's':
        total += emit_string(emit, context, va_arg(ap, char*));
        break;

      default: // invalid format 
        emit('%', context); // emit the skipped '%'
        emit(spec, context); 
        total += 2;
    }
  }

  return total;
}


int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int ret = format_core(stream_emitter, NULL, fmt, ap);

  va_end(ap);
  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  BufContext ctx = {.buf = out, .pos = 0, .size = (size_t)-1 }; // unsafe size

  int ret = format_core(buf_emitter, &ctx, fmt, ap);
  
  out[ctx.pos] = '\0'; // unsafe termination

  return ret;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int ret = vsprintf(out, fmt, ap);

  va_end(ap);
  return ret;
  
}


int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  // counting the needed length
  va_list ap_copy;
  va_copy(ap_copy, ap);
  int needed = format_core(null_emitter, &(size_t){0} , fmt, ap_copy);
  va_end(ap_copy);

  BufContext ctx = {.buf = out, .pos = 0, .size = n}; // safe size
  format_core(buf_emitter, &ctx, fmt, ap);
  out[ctx.pos] = '\0'; // safe termination

  return needed;
}


int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int ret = vsnprintf(out, n, fmt, ap);

  va_end(ap);
  return ret;
}

#endif

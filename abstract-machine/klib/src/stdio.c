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

/* Add padding for width specification */
static void emit_padding(EmitFunc emit, void *context, int width, int len, char pad_char) {
  for (int i = len; i < width; i++) {
    emit(pad_char, context);
  }
}

static void null_emitter(char ch, void *context){
  /* Do nothing about emit, only countng */
  (*(size_t *)context)++;
}


/* A wrapper of emitter */
static int emit_number(EmitFunc emit, void *context, int num, int base, int width, char pad_char, int left_align){
  char buf[64];
  int len = 0;
  int negative = 0;
  
  if(num < 0){
    negative = 1;
    num = -num;
  }

  do {
    buf[len++] = "0123456789abcdef"[num % base];
  } while( num /= base );

  int content_len = len + (negative ? 1 : 0);
  int padding_len = (width > content_len) ? width - content_len : 0;

  // For zero padding with negative numbers, we need special handling
  if(pad_char == '0' && negative && !left_align) {
    emit('-', context);  // Print sign first
    emit_padding(emit, context, width - 1, content_len - 1, pad_char);
    for(int i = len-1; i >=0; i--) emit(buf[i], context);
  }
  // Left align: content first, then padding
  else if(left_align) {
    if(negative) emit('-', context);
    for(int i = len-1; i >=0; i--) emit(buf[i], context);
    emit_padding(emit, context, width, content_len, pad_char);
  }
  // Right align with space padding (default): padding first, then content
  else {
    emit_padding(emit, context, width, content_len, pad_char);
    if(negative) emit('-', context);
    for(int i = len-1; i >=0; i--) emit(buf[i], context);
  }
  
  return content_len + padding_len;
}

static int emit_string(EmitFunc emit, void *context, const char* str, int width, char pad_char, int left_align){
  int len = 0;
  const char* p = str;
  while(*p){
    len++;
    p++;
  }

  int padding_len = (width > len) ? width - len : 0;

  // Left align: content first, then padding
  if(left_align) {
    while(*str){
      emit(*str++, context);
    }
    emit_padding(emit, context, width, len, pad_char);
  }
  // Right align (default): padding first, then content
  else {
    emit_padding(emit, context, width, len, pad_char);
    while(*str){
      emit(*str++, context);
    }
  }

  return len + padding_len;
}

/* Parse flags, width specification, return flags/width and advance fmt pointer */
static int parse_flags(const char **fmt, int *left_align, char *pad_char) {
  *left_align = 0;
  *pad_char = ' ';
  
  while (**fmt) {
    if (**fmt == '-') {
      *left_align = 1;
      (*fmt)++;
    } else if (**fmt == '0') {
      *pad_char = '0';
      (*fmt)++;
    } else {
      break;
    }
  }
  return 0;
}

static int parse_width(const char **fmt, va_list ap) {
  int width = 0;
  
  if (**fmt == '*') {
    (*fmt)++;
    width = va_arg(ap, int);
  } else {
    while (**fmt >= '0' && **fmt <= '9') {
      width = width * 10 + (**fmt - '0');
      (*fmt)++;
    }
  }
  
  return width > 0 ? width : 0; // 0 means no width specified
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

    /* Parse flags and width specification */
    int left_align;
    char pad_char;
    parse_flags(&fmt, &left_align, &pad_char);
    int width = parse_width(&fmt, ap);

    char spec = *fmt++; // character after width
    switch (spec) {
      case '%': // "%%" should output '%'
        emit('%', context);
        total++;
        break;

      case 'd':
        total += emit_number(emit, context, va_arg(ap, int), 10, width, pad_char, left_align);
        break;

      case 's':
        total += emit_string(emit, context, va_arg(ap, char*), width, pad_char, left_align);
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

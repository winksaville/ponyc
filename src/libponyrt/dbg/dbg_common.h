#ifndef DBG_COMMON_H
#define DBG_COMMON_H

#include <platform.h>
#include <stdio.h>
#include <stdint.h>

PONY_EXTERN_C_BEGIN

#define DBG_CTX_GEN_ID(a, b, c, d, e, f, g, h) \
  ((uint64_t)(a & 0xFF) << 56 | \
   (uint64_t)(b & 0xFF) << 48 | \
   (uint64_t)(c & 0xFF) << 40 | \
   (uint64_t)(d & 0xFF) << 32 | \
   (uint64_t)(e & 0xFF) << 24 | \
   (uint64_t)(f & 0xFF) << 16 | \
   (uint64_t)(g & 0xFF) <<  8 | \
   (uint64_t)(h & 0xFF) <<  0) 

#define DBG_CTX_ID_COMMON DBG_CTX_GEN_ID('C', 'O', 'M', 'M', 'O', 'N', 'I', 'D')

typedef struct dbg_ctx_t dbg_ctx_t;

typedef struct dbg_ctx_common_t {
  uint64_t id;
  size_t* bits;
  size_t number_of_bits;
  char* fun_name;
  int line_num;

  void* dbg_ctx_impl;
  void (*dbg_ctx_impl_destroy)(dbg_ctx_t* dc);
  size_t (*dbg_ctx_impl_vprintf)(dbg_ctx_t* dc, const char* format,
      va_list args);
} dbg_ctx_common_t;

void  dbg_ctx_common_init(dbg_ctx_common_t* dc, uint64_t id,
    size_t number_of_bits);

PONY_EXTERN_C_END

#endif

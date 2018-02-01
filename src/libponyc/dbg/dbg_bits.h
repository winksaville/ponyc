#ifndef DBG_BITS_H
#define DBG_BITS_H

#include <platform.h>
#include <stdio.h>
#include <stdint.h>

PONY_EXTERN_C_BEGIN

#define _DC_BIT_IDX_NAME(base_name) (base_name ## _bit_idx)
#define _DC_BIT_CNT_NAME(base_name) (base_name ## _bit_cnt)

#define _DC_NEXT_IDX(bit_idx) (_DC_BIT_IDX_NAME(bit_idx) + \
                                    _DC_BIT_CNT_NAME(bit_idx))

#define _DC_BITS_ARRAY_IDX(ctx, bit_idx) (bit_idx / (sizeof(ctx->bits[0]) * 8))
#define _DC_BIT_MSK(ctx, bit_idx) (1 << (bit_idx % (sizeof(ctx->bits[0]) * 8)))

#define _DC_BIT_IDX(base_name, bit_offset) \
  (_DC_BIT_IDX_NAME(base_name) + bit_offset)

enum {
  first_bit_idx = 0,
  first_bit_cnt = 32,
  dummy_bit_idx = _DC_NEXT_IDX(first),
  dummy_bit_cnt = 32,
};

#define size_bits 1024
typedef struct {
  FILE* file;
  uint32_t bits[size_bits];
} dbg_ctx_t;

extern dbg_ctx_t _dbg_ctx;

#if !defined(DC)
#define DC ((&_dbg_ctx))
#endif

#define dc_sb(ctx, base_name, bit_offset, bit_value) \
  do \
  { \
    uint32_t bit_idx = _DC_BIT_IDX(base_name, bit_offset); \
    uint32_t bits_array_idx = _DC_BITS_ARRAY_IDX(ctx, bit_idx); \
    if(bit_value != 0) \
      ctx->bits[bits_array_idx] |= _DC_BIT_MSK(ctx, bits_array_idx); \
    else \
      ctx->bits[bits_array_idx] &= ~_DC_BIT_MSK(ctx, bits_array_idx); \
  } \
  while(0)

#define dc_gb(ctx, base_name, bit_offset) \
  ({ \
    uint32_t bit_idx = _DC_BIT_IDX(base_name, bit_offset); \
    uint32_t bits_array_idx = _DC_BITS_ARRAY_IDX(ctx, bit_idx); \
    uint32_t b = ctx->bits[bits_array_idx] & _DC_BIT_MSK(ctx, bits_array_idx); \
    b; \
  })

void dc_init(dbg_ctx_t* dbg_ctx, FILE* file);

PONY_EXTERN_C_END

#endif

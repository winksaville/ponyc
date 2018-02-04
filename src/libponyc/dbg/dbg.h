#ifndef DBG_H
#define DBG_H

#include <platform.h>
#include <stdio.h>
#include <stdint.h>

PONY_EXTERN_C_BEGIN

#define _DBG_DO(...) switch(0) case 0:do { __VA_ARGS__ } while(0)

#define _DBG_BIT_IDX_NAME(base_name) (base_name ## _bit_idx)
#define _DBG_BIT_CNT_NAME(base_name) (base_name ## _bit_cnt)

#define _DBG_NEXT_IDX(base_name) (_DBG_BIT_IDX_NAME(base_name) + \
                                    _DBG_BIT_CNT_NAME(base_name))

#define _DBG_BITS_ARRAY_IDX(bit_idx) ((bit_idx) / 32)

#define _DBG_BIT_MASK(bit_idx) ((uint32_t)1 << (uint32_t)((bit_idx) & 0x1F))

enum {
  first_bit_idx = 0,
  first_bit_cnt = 32,
  dummy_bit_idx = _DBG_NEXT_IDX(first),
  dummy_bit_cnt = 32,
};

typedef struct {
  FILE* file;
  uint32_t* bits;
} dbg_ctx_t;

/**
 * Create a dbg_ctx
 */
dbg_ctx_t* dbg_ctx_create(FILE* file, uint32_t number_of_bits);

/**
 * Destroy a previously created dbg_ctx
 */
void dbg_ctx_destroy(dbg_ctx_t* dbg_ctx);

/**
 * Compute bit index from base_name and bit offsert
 * Convert base_name to base_name_bit_idx add bit_offset
 */
#define dbg_bni(base_name, bit_offset) \
  dbg_bi(_DBG_BIT_IDX_NAME(base_name), bit_offset)

/**
 * Compute bit index and bit offsert
 * Simply adds the two values.
 */
static inline uint32_t dbg_bi(uint32_t bit_base, uint32_t bit_offset)
{
  return bit_base + bit_offset;
}

/**
 * Set bit at bit_idx to bit_value
 */
static inline void dbg_sb(dbg_ctx_t* ctx, uint32_t bit_idx, bool bit_value)
{
  uint32_t bits_array_idx = _DBG_BITS_ARRAY_IDX(bit_idx);
  uint32_t bit_mask = _DBG_BIT_MASK(bit_idx);
  if(bit_value)
  {
    ctx->bits[bits_array_idx] |= bit_mask;
  } else {
    ctx->bits[bits_array_idx] &= ~bit_mask;
  }
}

/**
 * Get bit at bit_idx
 */
static inline bool dbg_gb(dbg_ctx_t* ctx, uint32_t bit_idx)
{
  uint32_t bits_array_idx = _DBG_BITS_ARRAY_IDX(bit_idx);
  return (ctx->bits[bits_array_idx] & _DBG_BIT_MASK(bit_idx)) != 0;
}

/**
 * Create a dbg context and initialize. In particular the
 * bits array will be zeroed.
 */
dbg_ctx_t* dbg_create(FILE* file, uint32_t number_of_bits);

/**
 * Free memory, the FILE is NOT closed.
 */
void dbg_destroy(dbg_ctx_t* dbg_ctx);

/**
 * Never a nop via conditional compilation and ignores ctx->bits
 */
#define DBG_PFU(ctx, format, ...) \
  _DBG_DO( fprintf(ctx->file, format, ## __VA_ARGS__); )

/**
 * Never a nop via conditional compilation and ignores ctx->bits
 */
#define DBG_PFNU(ctx, format, ...) \
  _DBG_DO(fprintf(ctx->file, "%s:  " format, __FUNCTION__, ## __VA_ARGS__);)

/**
 * Flush ctx file
 */
#define DBG_FLUSH(ctx) _DBG_DO(fflush(ctx->file);)

/**
 * Printf if bit_idx is set
 */
#define DBG_PF(ctx, bit_idx, format, ...) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->file, format, ## __VA_ARGS__); )

/**
 * Printf with leading "<funcName>:  " if bit_idx is set
 */
#define DBG_PFN(ctx, bit_idx, format, ...) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->file, "%s:  " format, __FUNCTION__, ## __VA_ARGS__); )

/**
 * "Enter routine" prints "<funcName>:+\n" if bit_idx is set
 */
#define DBG_E(ctx, bit_idx) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->file, "%s:+\n", __FUNCTION__); )

/**
 * Enter routine print leading functionName:+ and new line
 * if bit_idx is set
 */
#define DBG_PFE(ctx, bit_idx, format, ...) \
  _DBG_DO( \
     if(DBG_ENABLED) \
       if(dbg_gb(ctx, bit_idx)) \
         fprintf(ctx->file, "%s:+ " format, __FUNCTION__, ## __VA_ARGS__); )

/**
 * "Exit routine" prints "<funcName>:-\n" if bit_idx is set
 */
#define DBG_X(ctx, bit_idx) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->file, "%s:-\n", __FUNCTION__);)

/**
 * Enter routine print leading functionName:+ and new line
 * if bit_idx is set
 */
#define DBG_PFX(ctx, bit_idx, format, ...) \
  _DBG_DO( \
     if(DBG_ENABLED) \
       if(dbg_gb(ctx, bit_idx)) \
         fprintf(ctx->file, "%s:- " format, __FUNCTION__, ## __VA_ARGS__);)

PONY_EXTERN_C_END

#endif
#ifndef DBG_BITS_H
#define DBG_BITS_H

#include <platform.h>
#include <stdio.h>
#include <stdint.h>

PONY_EXTERN_C_BEGIN

#define _DC_BIT_IDX_NAME(base_name) (base_name ## _bit_idx)
#define _DC_BIT_CNT_NAME(base_name) (base_name ## _bit_cnt)

#define _DC_NEXT_IDX(base_name) (_DC_BIT_IDX_NAME(base_name) + \
                                    _DC_BIT_CNT_NAME(base_name))

#define _DC_BITS_ARRAY_IDX(bit_idx) ((bit_idx) / 32)

#define _DC_BIT_MASK(bit_idx) ((uint32_t)1 << (uint32_t)((bit_idx) & 0x1F))

enum {
  first_bit_idx = 0,
  first_bit_cnt = 32,
  dummy_bit_idx = _DC_NEXT_IDX(first),
  dummy_bit_cnt = 32,
};

typedef struct {
  FILE* file;
  uint32_t* bits;
} dbg_ctx_t;

extern dbg_ctx_t _dbg_ctx;

#if !defined(DC)
#define DC ((&_dbg_ctx))
#endif

/**
 * Compute bit index from base_name and bit offsert
 * Convert base_name to base_name_bit_idx add bit_offset
 */
#define dc_bni(base_name, bit_offset) \
  dc_bi(_DC_BIT_IDX_NAME(base_name), bit_offset)

/**
 * Compute bit index and bit offsert
 * Simply adds the two values.
 */
static inline uint32_t dc_bi(uint32_t bit_base, uint32_t bit_offset)
{
  return bit_base + bit_offset;
}

/**
 * Set bit at bit_idx to bit_value
 */
static inline void dc_sb(dbg_ctx_t* ctx, uint32_t bit_idx, bool bit_value)
{
  uint32_t bits_array_idx = _DC_BITS_ARRAY_IDX(bit_idx);
  uint32_t bit_mask = _DC_BIT_MASK(bit_idx);
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
static inline bool dc_gb(dbg_ctx_t* ctx, uint32_t bit_idx)
{
  uint32_t bits_array_idx = _DC_BITS_ARRAY_IDX(bit_idx);
  return (ctx->bits[bits_array_idx] & _DC_BIT_MASK(bit_idx)) != 0;
}

/**
 * Create a dbg context and initialize. In particular the
 * bits array will be zeroed.
 */
dbg_ctx_t* dc_create(FILE* file, uint32_t number_of_bits);

/**
 * Free memory, the FILE is NOT closed.
 */
void dc_destroy(dbg_ctx_t* dbg_ctx);

/**
 * Never a nop via conditional compilation and ignores ctx->bits
 */
#define DCUP(ctx, format, ...) \
  do { fprintf(ctx->file, format, ## __VA_ARGS__); } while(0)

/**
 * Flush ctx file
 */
#define DCFLUSH(ctx) do { fflush(ctx->file); } while(0)

/**
 * Printf if bit_idx is set
 */
#define DCPF(ctx, bit_idx, format, ...) \
  do \
  { \
     if(DBG_ENABLED) \
     { \
       if(dc_gb(ctx, bit_idx)) \
         fprintf(ctx->file, format, ## __VA_ARGS__); \
     } \
  } \
  while(0)

/**
 * Printf with leading "<funcName>:  " if bit_idx is set
 */
#define DCPFN(ctx, bit_idx, format, ...) \
  do \
  { \
     if(DBG_ENABLED) \
     { \
       if(dc_gb(ctx, bit_idx)) \
         fprintf(ctx->file, "%s:  " format, __FUNCTION__, ## __VA_ARGS__); \
     } \
  } \
  while(0)

/**
 * "Enter routine" prints "<funcName>:+\n" if bit_idx is set
 */
#define DCE(ctx, bit_idx) \
  do \
  { \
     if(DBG_ENABLED) \
     { \
       if(dc_gb(ctx, bit_idx)) \
         fprintf(ctx->file, "%s:+\n", __FUNCTION__); \
     } \
  } \
  while(0)

/**
 * Enter routine print leading functionName:+ and new line
 * if bit_idx is set
 */
#define DCPFE(ctx, bit_idx, format, ...) \
  do \
  { \
     if(DBG_ENABLED) \
     { \
       if(dc_gb(ctx, bit_idx)) \
         fprintf(ctx->file, "%s:+ " format, __FUNCTION__, ## __VA_ARGS__); \
     } \
  } \
  while(0)

/**
 * "Exit routine" prints "<funcName>:-\n" if bit_idx is set
 */
#define DCX(ctx, bit_idx) \
  do \
  { \
     if(DBG_ENABLED) \
     { \
       if(dc_gb(ctx, bit_idx)) \
         fprintf(ctx->file, "%s:-\n", __FUNCTION__); \
     } \
  } \
  while(0)

/**
 * Enter routine print leading functionName:+ and new line
 * if bit_idx is set
 */
#define DCPFX(ctx, bit_idx, format, ...) \
  do \
  { \
     if(DBG_ENABLED) \
     { \
       if(dc_gb(ctx, bit_idx)) \
         fprintf(ctx->file, "%s:- " format, __FUNCTION__, ## __VA_ARGS__); \
     } \
  } \
  while(0)

PONY_EXTERN_C_END

#endif

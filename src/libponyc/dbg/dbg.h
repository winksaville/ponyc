#ifndef DBG_H
#define DBG_H

#include <platform.h>
#include <stdio.h>
#include <stdint.h>

PONY_EXTERN_C_BEGIN

#if !defined(DBG_ENABLED)
#define DBG_ENABLED false
#endif

#define _DBG_DO(...) do {__VA_ARGS__;} while(0)

#define _DBG_BITS_ARRAY_IDX(bit_idx) ((bit_idx) / 32)
#define _DBG_BIT_MASK(bit_idx) ((uint32_t)1 << (uint32_t)((bit_idx) & 0x1F))

#if !defined(DBG_TMP_BUF_SIZE) || (DBG_TMP_BUF_SIZE <= 0)
#define DBG_TMP_BUF_SIZE 0x200
#endif

/**
 * Debug context structure.
 *
 * I've decided that the more important information of any single
 * "print/write" operation to the is the beginning information. As
 * such when writing to dst_buf only the first N bytes will be
 * preserved. Where N is the min(dst_buf_size, tmp_buf_size).
 */
typedef struct {
  FILE* dst_file;
  char* dst_buf;
  size_t dst_buf_size;
  size_t dst_buf_begi;
  size_t dst_buf_endi;
  size_t dst_buf_cnt;
  char tmp_buf[DBG_TMP_BUF_SIZE];
  size_t tmp_buf_size;
  size_t max_size;
  uint32_t* bits;
} dbg_ctx_t;

/**
 * Create a dbg_ctx with char buf as destination
 */
dbg_ctx_t* dbg_ctx_create_with_dst_buf(size_t dst_size,
    uint32_t number_of_bits);

/**
 * Create a dbg_ctx with a FILE as as destination
 */
dbg_ctx_t* dbg_ctx_create_with_dst_file(FILE* dst, uint32_t number_of_bits);

/**
 * Destroy a previously created dbg_ctx
 */
void dbg_ctx_destroy(dbg_ctx_t* dbg_ctx);

/**
 * Print a formated string to the current dbg_ctx destination
 */
int dbg_printf(dbg_ctx_t* dbg_ctx, const char* format, ...);

/**
 * Print a formated string to the current dbg_ctx destination with
 * the args being a va_list.
 */
int dbg_vprintf(dbg_ctx_t* dbg_ctx, const char* format, va_list vlist);

/**
 * used by dbg_get_buf to linearize the buffer which
 * guarantees the buffer does not wrap, so begi < endi.
 */
//void dbg_linearize(dbg_ctx_t* ctx);

/**
 * Read data to the dst buf. dst is the destination and
 * size is the maximum size to read.
 *
 * return number of bytes read not including the null
 * terminator written at the end. If size is 0 nothing
 * is written to dst and the return value is the number
 * of bytes that would be needed to hold the buffer not
 * counting the null terminator.
 */
size_t dbg_read(dbg_ctx_t* ctx, char* dst, size_t size);

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

static inline char* dbg_get_buf(dbg_ctx_t* ctx)
{
  //if(ctx->dst_buf_endi < ctx->dst_buf_begi)
  //  dbg_linearize(ctx);
  return &ctx->dst_buf[ctx->dst_buf_begi];
}

/**
 * Unconditionally print, ignores ctx->bits
 */
#define DBG_PFU(ctx, format, ...) \
  _DBG_DO(dbg_printf(ctx, format, __VA_ARGS__))

/**
 * Unconditionally print a string, ignores ctx->bits
 * Needed for use on Windows
 */
#define DBG_PSU(ctx, str) \
  _DBG_DO(fprintf(ctx->dst_file, str))

/**
 * Unconditionally print with function name, ignores ctx->bits
 */
#define DBG_PFNU(ctx, format, ...) \
  _DBG_DO(fprintf(ctx->dst_file, "%s:  " format, __FUNCTION__, __VA_ARGS__))

/**
 * Unconditionally print string with function name, ignores ctx->bits
 */
#define DBG_PSNU(ctx, str) \
  _DBG_DO(fprintf(ctx->dst_file, "%s:  " str, __FUNCTION__))

/**
 * Flush ctx file
 */
#define DBG_FLUSH(ctx) _DBG_DO(fflush(ctx->dst_file))

/**
 * Printf if bit_idx is set
 */
#define DBG_PF(ctx, bit_idx, format, ...) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->dst_file, format, __VA_ARGS__))

/**
 * Print string if bit_idx is set
 */
#define DBG_PS(ctx, bit_idx, str) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->dst_file, str))

/**
 * Printf with leading "<funcName>:  " if bit_idx is set
 */
#define DBG_PFN(ctx, bit_idx, format, ...) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->dst_file, "%s:  " format, __FUNCTION__, __VA_ARGS__))

/**
 * Print string with leading "<funcName>:  " if bit_idx is set
 */
#define DBG_PSN(ctx, bit_idx, str) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->dst_file, "%s:  " str, __FUNCTION__))

/**
 * "Enter routine" prints "<funcName>:+\n" if bit_idx is set
 */
#define DBG_E(ctx, bit_idx) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->dst_file, "%s:+\n", __FUNCTION__))

/**
 * Enter routine print leading functionName:+ and new line
 * if bit_idx is set
 */
#define DBG_PFE(ctx, bit_idx, format, ...) \
  _DBG_DO( \
     if(DBG_ENABLED) \
       if(dbg_gb(ctx, bit_idx)) \
         fprintf(ctx->dst_file, "%s:+ " format, __FUNCTION__, __VA_ARGS__))

/**
 * Enter routine print string with leading functionName:+ and new line
 * if bit_idx is set
 */
#define DBG_PSE(ctx, bit_idx, str) \
  _DBG_DO( \
     if(DBG_ENABLED) \
       if(dbg_gb(ctx, bit_idx)) \
         fprintf(ctx->dst_file, "%s:+ " str, __FUNCTION__))

/**
 * "Exit routine" prints "<funcName>:-\n" if bit_idx is set
 */
#define DBG_X(ctx, bit_idx) \
  _DBG_DO( \
    if(DBG_ENABLED) \
      if(dbg_gb(ctx, bit_idx)) \
        fprintf(ctx->dst_file, "%s:-\n", __FUNCTION__))

/**
 * Exit routine print leading functionName:+ and new line
 * if bit_idx is set
 */
#define DBG_PFX(ctx, bit_idx, format, ...) \
  _DBG_DO( \
     if(DBG_ENABLED) \
       if(dbg_gb(ctx, bit_idx)) \
         fprintf(ctx->dst_file, "%s:- " format, __FUNCTION__, __VA_ARGS__))

/**
 * Exit routine print string with leading functionName:+ and new line
 * if bit_idx is set
 */
#define DBG_PSX(ctx, bit_idx, str) \
  _DBG_DO( \
     if(DBG_ENABLED) \
       if(dbg_gb(ctx, bit_idx)) \
         fprintf(ctx->dst_file, "%s:- " str, __FUNCTION__))

PONY_EXTERN_C_END

#endif

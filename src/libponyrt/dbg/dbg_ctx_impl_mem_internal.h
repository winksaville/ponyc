#ifndef DBG_CTX_IMPL_MEM_InTERNAL_H
#define DBG_CTX_IMPL_MEM_INTERNAL_H

#include "dbg.h"
#include "ponyassert.h"
#include <assert.h>
#include <platform.h>


PONY_EXTERN_C_BEGIN

/**
 * Debug context implemention structure that logs to memory.
 *
 * Assume that the more important information of any single
 * "print/write" operation is at the beginning. As such when
 * writing to dst_buf only the first N bytes will be
 * preserved. Where N is the min(dst_buf_size, tmp_buf_size).
 */
typedef struct dbg_ctx_impl_mem_t {
  dbg_ctx_common_t common;
  uint8_t* dst_buf;
  size_t dst_buf_size;
  size_t dst_buf_begi;
  size_t dst_buf_endi;
  size_t dst_buf_cnt;
  uint8_t* tmp_buf;
  size_t tmp_buf_size;
  size_t max_size;
} dbg_ctx_impl_mem_t;

pony_static_assert(offsetof(dbg_ctx_impl_mem_t, common) == 0, "");

PONY_EXTERN_C_END

#endif

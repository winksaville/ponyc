#ifndef DBG_CTX_IMPL_FILE_INTERNAL_H
#define DBG_CTX_IMPL_FILE_INTERNAL_H

#include "dbg.h"
#include "ponyassert.h"
#include <assert.h>
#include <platform.h>

PONY_EXTERN_C_BEGIN

/**
 * Debug context implemention structure that logs to a file
 */
typedef struct dbg_ctx_impl_file_t {
  dbg_ctx_common_t common;
  FILE* dst_file;
} dbg_ctx_impl_file_t;

pony_static_assert(offsetof(dbg_ctx_impl_file_t, common) == 0, "");

PONY_EXTERN_C_END

#endif

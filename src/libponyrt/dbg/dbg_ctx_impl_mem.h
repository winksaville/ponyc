#ifndef DBG_CTX_IMPL_MEM_H
#define DBG_CTX_IMPL_MEM_H

#include "dbg.h"
#include <platform.h>

PONY_EXTERN_C_BEGIN

#define DBG_CTX_ID_MEM DBG_CTX_GEN_ID('M', 'E', 'M', ' ', ' ', ' ', 'I', 'D')

/**
 * Create a dbg_ctx with char buf as destination
 */
dbg_ctx_t* dbg_ctx_impl_mem_create(size_t number_of_bits, size_t dst_size,
  size_t tmp_buf_size);

/**
 * Read data from the dc to dst. dst_size is size of
 * the dst and must be > size to accommodate the null
 * terminator written at the end. Size is the maximum size
 * to read.
 *
 * @returns number of bytes read not including the null
 * terminator written at the end.
 */
size_t dbg_read(dbg_ctx_t* dc, char* dst, size_t buf_size, size_t size);

PONY_EXTERN_C_END

#endif

#ifndef DBG_CTX_IMPL_FILE_H
#define DBG_CTX_IMPL_FILE_H

#include "dbg.h"
#include <platform.h>

PONY_EXTERN_C_BEGIN

#define DBG_CTX_ID_FILE DBG_CTX_GEN_ID('F', 'I', 'L', 'E', ' ', ' ', 'I', 'D')

/**
 * Create a dbg_ctx with a FILE as as destination
 */
dbg_ctx_t* dbg_ctx_impl_file_create(size_t number_of_bits, FILE* dst);

/**
 * Destroy a previously created dbg_ctx
 */
void dbg_ctx_impl_file_destroy(dbg_ctx_t* dc);

/**
 * Print to a file
 */
size_t dbg_ctx_impl_file_vprintf(dbg_ctx_t* dc, const char* format,
    va_list args);

PONY_EXTERN_C_END

#endif

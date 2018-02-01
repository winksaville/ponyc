#include <string.h>
#include "dbg_bits.h"

dbg_ctx_t _dbg_ctx;

void dc_init(dbg_ctx_t* dbg_ctx, FILE* file)
{
  dbg_ctx->file = file;
  memset(&dbg_ctx->bits, 0, sizeof(dbg_ctx->bits));
}

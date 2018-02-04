#include "dbg.h"

#include "ponyassert.h"
#include <string.h>


dbg_ctx_t* dbg_ctx_create(FILE* file, uint32_t number_of_bits)
{
  dbg_ctx_t* dc = malloc(sizeof(dbg_ctx_t));
  pony_assert(dc != NULL);
  dc->file = file;
  dc->bits = (uint32_t*)calloc(1, ((number_of_bits + 31) / 32) * 32);
  pony_assert(dc->bits != NULL);
  pony_assert(sizeof(dc->bits[0]) == 4);
  return dc;
}

void dbg_ctx_destroy(dbg_ctx_t* dbg_ctx)
{
  free(dbg_ctx->bits);
  free(dbg_ctx);
}

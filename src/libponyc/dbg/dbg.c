#include "dbg.h"

#include "ponyassert.h"
#include <string.h>

static dbg_ctx_t* calloc_ctx_and_bits(uint32_t number_of_bits)
{
  dbg_ctx_t* dc = (dbg_ctx_t*)calloc(1, sizeof(dbg_ctx_t));
  dc->bits = (uint32_t*)calloc(1, ((number_of_bits + 31) / 32) * 32);
  pony_assert(dc->bits != NULL);
  pony_assert(sizeof(dc->bits[0]) == 4);
  return dc;
}

dbg_ctx_t* dbg_ctx_create_with_dst_file(FILE* file, uint32_t number_of_bits)
{
  dbg_ctx_t* dc = calloc_ctx_and_bits(number_of_bits);
  dc->dst_file = file;
  dc->dst_buf = NULL;
  pony_assert(dc->dst_file != NULL);
  pony_assert(dc->dst_buf == NULL);
  pony_assert(dc->dst_buf_size == 0);
  return dc;
}

dbg_ctx_t* dbg_ctx_create_with_dst_buf(size_t dst_buf_size,
    uint32_t number_of_bits)
{
  dbg_ctx_t* dc = calloc_ctx_and_bits(number_of_bits);
  dc->dst_file = NULL;
  dc->dst_buf_size = dst_buf_size;
  dc->dst_buf = (char*)calloc(1, dst_buf_size);

  pony_assert(dc->dst_buf != NULL);
  pony_assert(dc->dst_file == NULL);
  pony_assert(dc->dst_buf_size > 1);
  return dc;
}

void dbg_ctx_destroy(dbg_ctx_t* dbg_ctx)
{
  free(dbg_ctx->dst_buf);
  free(dbg_ctx->bits);
  free(dbg_ctx);
}

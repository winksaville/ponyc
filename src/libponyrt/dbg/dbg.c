#include "dbg.h"

#include "ponyassert.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void dbg_ctx_common_init(dbg_ctx_common_t* dcc, uint64_t id,
    size_t number_of_bits)
{
  pony_assert(dcc != NULL);
  dcc->id = id;
  dcc->number_of_bits = number_of_bits;
  size_t bits_size =
    ((number_of_bits + sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t);
  if(bits_size != 0)
  {
    dcc->bits = (size_t*)calloc(sizeof(size_t), bits_size);
    pony_assert(dcc->bits != NULL);
  }
  pony_assert(sizeof(dcc->bits[0]) == sizeof(size_t));
  pony_assert(dcc->fun_name == NULL);
  pony_assert(dcc->line_num == 0);
  pony_assert(dcc->dbg_ctx_impl_destroy == NULL);
  pony_assert(dcc->dbg_ctx_impl_vprintf == NULL);
}

void  dbg_ctx_common_destroy(dbg_ctx_common_t* dcc)
{
  if(dcc->bits != NULL)
    free(dcc->bits);
}

dbg_ctx_t* dbg_ctx_create(size_t number_of_bits)
{
  dbg_ctx_common_t* dcc =
    (dbg_ctx_common_t*)calloc(1, sizeof(dbg_ctx_common_t));
  dbg_ctx_common_init(dcc, DBG_CTX_ID_COMMON, number_of_bits);
  return (dbg_ctx_t*)dcc;
}

void dbg_ctx_destroy(dbg_ctx_t* dc)
{
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;
  if(dcc != NULL)
  {
    dbg_ctx_common_destroy(dcc);

    if(dcc->dbg_ctx_impl_destroy != NULL)
      dcc->dbg_ctx_impl_destroy(dc);
  }
}

void dbg_set_bit(dbg_ctx_t* dc, size_t bit_idx, bool bit_value)
{
  dbg_sb(dc, bit_idx, bit_value);
}

bool dbg_get_bit(dbg_ctx_t* dc, size_t bit_idx)
{
  return dbg_gb(dc, bit_idx);
}

size_t dbg_printf(dbg_ctx_t* dc, const char* format, ...)
{
  size_t total = 0;
  if((dc != NULL) && (format != NULL))
  {
    va_list args;
    va_start(args, format);
    total = dbg_vprintf(dc, format, args);
    va_end(args);
    return 0;
  }
  return total;
}

size_t dbg_vprintf(dbg_ctx_t* dc, const char* format, va_list args)
{
  int rv;
  size_t total = 0;
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;

  if((dcc != NULL) && (format != NULL))
  {
    char format_full[0x200];
    va_list args_copy;
    va_copy(args_copy, args);

    if(dcc->fun_name != NULL)
    {
      rv = snprintf(format_full, sizeof(format_full), "%s:%-4d %s",
        dcc->fun_name, dcc->line_num, format);
      if(rv < 0)
      {
        va_end(args_copy);
        return 0;
      }
      dcc->fun_name = NULL;
      format = format_full;
    }
    if(dcc->dbg_ctx_impl_vprintf != NULL)
    {
      dcc->dbg_ctx_impl_vprintf(dc, format, args_copy);
    }
    va_end(args_copy);
  }
  return total;
}

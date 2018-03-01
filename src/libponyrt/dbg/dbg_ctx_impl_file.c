#include "dbg_ctx_impl_file.h"
#include "dbg_ctx_impl_file_internal.h"

size_t dbg_ctx_impl_file_vprintf(dbg_ctx_t* dc, const char* format,
    va_list args)
{
  dbg_ctx_impl_file_t* dcif = (dbg_ctx_impl_file_t*)dc;
  size_t total = 0;
  if((dcif->common.id == DBG_CTX_ID_FILE) && (dcif->dst_file != NULL)) {
    int rv = vfprintf(dcif->dst_file, format, args);
    if(rv < 0)
      return 0;
    total += rv;
  }
  return total;
}

dbg_ctx_t* dbg_ctx_impl_file_create(size_t number_of_bits, FILE* file)
{
  dbg_ctx_impl_file_t* dcif =
    (dbg_ctx_impl_file_t*)calloc(1, sizeof(dbg_ctx_impl_file_t));

  dbg_ctx_common_init(&dcif->common, DBG_CTX_ID_FILE, number_of_bits);
  dcif->dst_file = file;
  dcif->common.dbg_ctx_impl_destroy = NULL;
  dcif->common.dbg_ctx_impl_vprintf = dbg_ctx_impl_file_vprintf;
  pony_assert(dcif->common.id == DBG_CTX_ID_FILE);

  return (dbg_ctx_t*)dcif;
}

void dbg_ctx_impl_file_destroy(dbg_ctx_t* dc)
{
  dbg_ctx_impl_file_t* dcif = (dbg_ctx_impl_file_t*)dc;
  free(dcif);
}

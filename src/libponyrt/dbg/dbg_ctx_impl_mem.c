#include "dbg_ctx_impl_mem.h"
#include "dbg_util.h"
#include "ponyassert.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dbg_ctx_impl_mem_internal.h"

static void move(dbg_ctx_t* dc, uint8_t* dst, const uint8_t* src, size_t size)
{
  dbg_ctx_impl_mem_t* dcim = (dbg_ctx_impl_mem_t*)dc;

  memcpy(dst, src, size);
  dcim->dst_buf_cnt += size;
  dcim->dst_buf_endi += size;
  ssize_t overwritten = dcim->dst_buf_cnt - dcim->dst_buf_size;
  if(overwritten > 0)
  {
    dcim->dst_buf_begi += overwritten;
    dcim->dst_buf_cnt -= overwritten;
  }
  if(dcim->dst_buf_endi >= dcim->dst_buf_size)
    dcim->dst_buf_endi -= dcim->dst_buf_size;
  if(dcim->dst_buf_begi >= dcim->dst_buf_size)
    dcim->dst_buf_begi -= dcim->dst_buf_size;
}

static size_t dbg_ctx_impl_mem_vprintf(dbg_ctx_t* dc, const char* format,
    va_list args)
{
  dbg_ctx_impl_mem_t* dcim = (dbg_ctx_impl_mem_t*)dc;
  size_t total = 0;

  if((dcim->common.id == DBG_CTX_ID_MEM) &&(dcim != NULL) && (format != NULL))
  {
    if((dcim->dst_buf != NULL) && (dcim->tmp_buf != NULL))
    {
      uint8_t* src;
      uint8_t*dst = dcim->tmp_buf;
      size_t size = dcim->max_size;
      int rv = vsnprintf((char*)dst, size + 1, format, args);
      if(rv < 0)
      {
        return 0;
      }

      total = (size >= (size_t)rv) ? (size_t)rv : size;
      src = &dcim->tmp_buf[0];
      dst = &dcim->dst_buf[dcim->dst_buf_endi];
      size = dcim->dst_buf_size - dcim->dst_buf_endi;
      if(size >= total)
      {
        move(dc, dst, src, total);
      } else {
        // Read the first part from the end of the dst_buf
        move(dc, dst, src, size);

        // Read the second part from the beginning of dst_buf
        dst = &dcim->dst_buf[0];
        src = &dcim->tmp_buf[size];
        size = total - size;
        move(dc, dst, src, size);
      }
    }
  }
  return total;
}

size_t dbg_read(dbg_ctx_t* dc, char* dst, size_t buf_size, size_t size)
{
  size_t total;
  uint8_t* src;
  dbg_ctx_impl_mem_t* dcim = (dbg_ctx_impl_mem_t*)dc;

  // Normalize size parameters and be sure
  // to leave room for null trailer
  if(size > buf_size)
  {
    if(buf_size > 0)
      size = buf_size - 1;
    else
      size = 0;
  }

  total = 0;
  if((dcim->common.id == DBG_CTX_ID_MEM) && (dcim != NULL) && (dst != NULL))
  {
    // total = min(size, dcim->dst_buf_cnt)
    total = (size > dcim->dst_buf_cnt) ? dcim->dst_buf_cnt : size;
    if(total > 0)
    {
      size_t idx = dcim->dst_buf_begi;
      if(idx >= dcim->dst_buf_endi)
      {
        // Might do one or two memcpy
        size = dcim->dst_buf_size - idx;
      } else {
        // One memcpy
        size = dcim->dst_buf_endi - idx;
      }
      // Adjust size incase its to large
      if(size > total)
        size = total;

      // Do first copy
      size_t cnt = 0;
      src = &dcim->dst_buf[idx];
      memcpy(dst, src, size);

      // Record what we copied
      dst += size;
      cnt += size;

      // Check if we're done
      if(cnt < total)
      {
        // Not done, wrap to the begining of the buffer
        // and size = endi
        pony_assert(dcim->dst_buf_endi <= dcim->dst_buf_begi);
        idx = 0;
        size = dcim->dst_buf_endi;

        src = &dcim->dst_buf[idx];
        memcpy(dst, src, size);
        dst += size;
        cnt += size;
      } else {
        size = 0;
      }
      // Validate we've finished
      pony_assert(cnt == total);

      // Adjust cnt and begi
      dcim->dst_buf_cnt -= total;
      dcim->dst_buf_begi += total;
      if(dcim->dst_buf_begi >= dcim->dst_buf_size)
        dcim->dst_buf_begi -= dcim->dst_buf_size;
    }

    // Add null terminator
    if(buf_size > 0)
      *dst = 0;
  }
  return total;
}

static void dbg_ctx_impl_mem_destroy(dbg_ctx_t* dc)
{
  dbg_ctx_impl_mem_t* dcim = (dbg_ctx_impl_mem_t*)dc;

  if(dcim != NULL)
  {
    free(dcim->dst_buf);
    free(dcim->tmp_buf);
    free(dc);
  }
}

dbg_ctx_t* dbg_ctx_impl_mem_create(size_t number_of_bits,
    size_t dst_buf_size, size_t tmp_buf_size)
{
  dbg_ctx_impl_mem_t* dcim =
    (dbg_ctx_impl_mem_t*)calloc(1, sizeof(dbg_ctx_impl_mem_t));

  dbg_ctx_common_init(&dcim->common, DBG_CTX_ID_MEM, number_of_bits);

  dcim->tmp_buf_size = tmp_buf_size;
  dcim->tmp_buf = (uint8_t*)calloc(1, tmp_buf_size);

  dcim->dst_buf_size = dst_buf_size;
  dcim->dst_buf = (uint8_t*)calloc(1, dst_buf_size);
  dcim->max_size = dcim->dst_buf_size > dcim->tmp_buf_size ?
                      dcim->tmp_buf_size : dcim->dst_buf_size;
  pony_assert(dcim->common.id == DBG_CTX_ID_MEM);
  pony_assert(dcim->dst_buf != NULL);
  pony_assert(dcim->tmp_buf != NULL);
  pony_assert(dcim->dst_buf_size > 0);
  pony_assert(dcim->dst_buf_begi == 0);
  pony_assert(dcim->dst_buf_endi == 0);
  pony_assert(dcim->dst_buf_cnt == 0);

  dcim->common.dbg_ctx_impl_destroy = dbg_ctx_impl_mem_destroy;
  dcim->common.dbg_ctx_impl_vprintf = dbg_ctx_impl_mem_vprintf;

  return (dbg_ctx_t*)dcim;
}

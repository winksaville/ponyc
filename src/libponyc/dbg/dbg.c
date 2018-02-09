#include "dbg.h"

#include "ponyassert.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static dbg_ctx_t* calloc_ctx_and_bits(size_t tmp_buf_size,
    uint32_t number_of_bits)
{
  dbg_ctx_t* dc = (dbg_ctx_t*)calloc(1, sizeof(dbg_ctx_t));
  pony_assert(dc != NULL);
  dc->bits = (uint32_t*)calloc(1, ((number_of_bits + 31) / 32) * 32);
  pony_assert(dc->bits != NULL);
  pony_assert(sizeof(dc->bits[0]) == 4);
  dc->tmp_buf_size = tmp_buf_size;
  if(tmp_buf_size > 0)
  {
    dc->tmp_buf = (char*)calloc(1, tmp_buf_size);
    pony_assert(dc->tmp_buf != NULL);
  }
  else
    dc->tmp_buf = NULL;
  return dc;
}

dbg_ctx_t* dbg_ctx_create_with_dst_file(FILE* file, uint32_t number_of_bits)
{
  dbg_ctx_t* dc = calloc_ctx_and_bits(0, number_of_bits);
  dc->dst_file = file;
  pony_assert(dc->dst_file != NULL);
  pony_assert(dc->tmp_buf == NULL);
  pony_assert(dc->dst_buf == NULL);
  pony_assert(dc->dst_buf_size == 0);
  pony_assert(dc->dst_buf_begi == 0);
  pony_assert(dc->dst_buf_endi == 0);
  return dc;
}

dbg_ctx_t* dbg_ctx_create_with_dst_buf(size_t dst_buf_size, size_t tmp_buf_size,
    uint32_t number_of_bits)
{
  dbg_ctx_t* dc = calloc_ctx_and_bits(tmp_buf_size, number_of_bits);
  dc->dst_buf_size = dst_buf_size;
  dc->dst_buf = (char*)calloc(1, dst_buf_size);
  dc->max_size = dc->dst_buf_size > dc->tmp_buf_size ?
                      dc->tmp_buf_size : dc->dst_buf_size;
  pony_assert(dc->dst_file == NULL);
  pony_assert(dc->dst_buf != NULL);
  pony_assert(dc->tmp_buf != NULL);
  pony_assert(dc->bits != NULL);
  pony_assert(dc->dst_buf_size > 0);
  pony_assert(dc->dst_buf_begi == 0);
  pony_assert(dc->dst_buf_endi == 0);
  pony_assert(dc->dst_buf_cnt == 0);
  return dc;
}

void dbg_ctx_destroy(dbg_ctx_t* dc)
{
  free(dc->dst_buf);
  free(dc->tmp_buf);
  free(dc->bits);
  free(dc);
}

//static void dump(const char* leader, char *p, size_t l)
//{
//  printf("%s", leader);
//  for(size_t i = 0; i < l; i++)
//  {
//    unsigned char c = *p++;
//    if(isalpha(c))
//      printf("%c", c);
//    else
//      printf("<%02x>", c);
//  }
//  printf("\n");
//}

static void move(dbg_ctx_t* dc, char* dst, const char* src, size_t size)
{
  memcpy(dst, src, size);
  dc->dst_buf_cnt += size;
  dc->dst_buf_endi += size;
  ssize_t overwritten = dc->dst_buf_cnt - dc->dst_buf_size;
  if(overwritten > 0)
  {
    dc->dst_buf_begi += overwritten;
    dc->dst_buf_cnt -= overwritten;
  }
  if(dc->dst_buf_endi >= dc->dst_buf_size)
    dc->dst_buf_endi -= dc->dst_buf_size;
  if(dc->dst_buf_begi >= dc->dst_buf_size)
    dc->dst_buf_begi -= dc->dst_buf_size;
}

size_t dbg_printf(dbg_ctx_t* dc, const char* format, ...)
{
  va_list vlist;
  va_start(vlist, format);
  size_t total = dbg_vprintf(dc, format, vlist);
  va_end(vlist);
  return total;
}

size_t dbg_vprintf(dbg_ctx_t* dc, const char* format, va_list vlist)
{
  size_t size;
  size_t total;
  char *restrict dst;
  char *restrict src;
  if(dc->dst_buf != NULL)
  {
    dst = dc->tmp_buf;
    size = dc->max_size;
    int rv = vsnprintf(dst, size + 1, format, vlist);
    if(rv < 0)
      return 0;

    total = (size >= (size_t)rv) ? (size_t)rv : size;
    src = &dc->tmp_buf[0];
    dst = &dc->dst_buf[dc->dst_buf_endi];
    size = dc->dst_buf_size - dc->dst_buf_endi;
    if(size >= total)
    {
      move(dc, dst, src, total);
    } else {
      // Read the first part from the end of the dst_buf
      move(dc, dst, src, size);

      // Read the second part from the beginning of dst_buf
      dst = &dc->dst_buf[0];
      src = &dc->tmp_buf[size];
      size = total - size;
      move(dc, dst, src, size);
    }
  } else {
    total = vfprintf(dc->dst_file, format, vlist);
  }

  return total;
}


size_t dbg_read(dbg_ctx_t* dc, char* dst, size_t buf_size, size_t size)
{
  //MAYBE_UNUSED(buf_size);
  size_t total;
  char* src;

  pony_assert(buf_size > size);

  // Reduce length by one to leave room for null trailer
  total = 0;
  if((dst != NULL) && size > 0)
  {
    // total = min(size, dc->dst_buf_cnt)
    total = (size > dc->dst_buf_cnt) ? dc->dst_buf_cnt : size;
    if(total > 0)
    {
      size_t idx = dc->dst_buf_begi;
      if(idx >= dc->dst_buf_endi)
      {
        // Might do one or two memcpy
        size = dc->dst_buf_size - idx;
      } else {
        // One memcpy
        size = dc->dst_buf_endi - idx;
      }
      // Adjust size incase its to large
      if(size > total)
        size = total;

      // Do first copy
      size_t cnt = 0;
      src = &dc->dst_buf[idx];
      memcpy(dst, src, size);

      // Record what we copied
      dst += size;
      cnt += size;

      // Check if we're done
      if(cnt < total)
      {
        // Not done, wrap to the begining of the buffer
        // and size = endi
        pony_assert(dc->dst_buf_endi <= dc->dst_buf_begi);
        idx = 0;
        size = dc->dst_buf_endi;

        src = &dc->dst_buf[idx];
        memcpy(dst, src, size);
        dst += size;
        cnt += size;
      } else {
        size = 0;
      }
      // Validate we've finished
      pony_assert(cnt == total);

      // Adjust cnt and begi
      dc->dst_buf_cnt -= total;
      dc->dst_buf_begi += total;
      if(dc->dst_buf_begi >= dc->dst_buf_size)
        dc->dst_buf_begi -= dc->dst_buf_size;
    }

    // Add null terminator
    *dst = 0;
  }
  return total;
}

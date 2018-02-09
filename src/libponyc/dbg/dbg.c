#include "dbg.h"

#include "ponyassert.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static dbg_ctx_t* calloc_ctx_and_bits(uint32_t number_of_bits)
{
  dbg_ctx_t* dc = (dbg_ctx_t*)calloc(1, sizeof(dbg_ctx_t));
  dc->bits = (uint32_t*)calloc(1, ((number_of_bits + 31) / 32) * 32);
  dc->tmp_buf_size = DBG_TMP_BUF_SIZE;
  pony_assert(dc->bits != NULL);
  pony_assert(sizeof(dc->bits[0]) == 4);
  return dc;
}

dbg_ctx_t* dbg_ctx_create_with_dst_file(FILE* file, uint32_t number_of_bits)
{
  dbg_ctx_t* dc = calloc_ctx_and_bits(number_of_bits);
  dc->dst_file = file;
  pony_assert(dc->dst_file != NULL);
  pony_assert(dc->dst_buf == NULL);
  pony_assert(dc->dst_buf_size == 0);
  pony_assert(dc->dst_buf_begi == 0);
  pony_assert(dc->dst_buf_endi == 0);
  return dc;
}

dbg_ctx_t* dbg_ctx_create_with_dst_buf(size_t dst_buf_size,
    uint32_t number_of_bits)
{
  dbg_ctx_t* dc = calloc_ctx_and_bits(number_of_bits);
  dc->dst_buf_size = dst_buf_size;
  dc->dst_buf = (char*)calloc(1, dst_buf_size);
  dc->max_size = dc->dst_buf_size > dc->tmp_buf_size ?
                      dc->tmp_buf_size : dc->dst_buf_size;
  pony_assert(dc->dst_file == NULL);
  pony_assert(dc->dst_buf_size > 0);
  pony_assert(dc->dst_buf_begi == 0);
  pony_assert(dc->dst_buf_endi == 0);
  pony_assert(dc->dst_buf_cnt == 0);
  return dc;
}

void dbg_ctx_destroy(dbg_ctx_t* dc)
{
  free(dc->dst_buf);
  free(dc->bits);
  free(dc);
}

static void dump(const char* leader, char *p, size_t l)
{
  printf("%s", leader);
  for(size_t i = 0; i < l; i++)
  {
    unsigned char c = *p++;
    if(isalpha(c))
      printf("%c", c);
    else
      printf("<%02x>", c);
  }
  printf("\n");
}

static void move(dbg_ctx_t* dc, char* dst, const char* src, size_t size)
{
  printf("move:+ size=%zu dst_buf size=%zu begi=%zu endi=%zu cnt=%zu\n",
      size, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt);
  // Nice, only one memcpy needed
  memcpy(dst, src, size);
  dc->dst_buf_cnt += size;
  dc->dst_buf_endi += size;
  ssize_t overwritten = dc->dst_buf_cnt - dc->dst_buf_size;
  printf("move:  overwritten=%zi size=%zu dst_buf size=%zu begi=%zu endi=%zu cnt=%zu\n",
      overwritten, size, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt);
  if(overwritten > 0)
  {
    dc->dst_buf_begi += overwritten;
    dc->dst_buf_cnt -= overwritten;
    printf("move: overwritten=%zi size=%zu dst_buf size=%zu begi=%zu endi=%zu cnt=%zu\n",
        overwritten, size, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt);
  }
  printf("move:  overwritten=%zi size=%zu dst_buf size=%zu begi=%zu endi=%zu cnt=%zu\n",
      overwritten, size, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt);
  if(dc->dst_buf_endi >= dc->dst_buf_size)
    dc->dst_buf_endi -= dc->dst_buf_size;
  if(dc->dst_buf_begi >= dc->dst_buf_size)
    dc->dst_buf_begi -= dc->dst_buf_size;
  printf("move:- size=%zu dst_buf size=%zu begi=%zu endi=%zu cnt=%zu\n",
      size, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt);
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
    printf("dbg_vprintf:+ dst_buf size=%zu begi=%zu endi=%zu cnt=%zu\n",
        dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt);

    dst = dc->tmp_buf;
    size = dc->max_size;
    int rv = vsnprintf(dst, size + 1, format, vlist);
    if(rv < 0)
    {
      printf("dbg_vprintf:- tmp_buf err=%d strerror=%s\n", rv, strerror(rv));
      return 0;
    }
    total = (size >= (size_t)rv) ? (size_t)rv : size;
    printf("dbg_vprintf:  1  size=%zu rv=%d total=%zu tmp_buf=%s\n",
        size, rv, total, dc->tmp_buf);

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

    printf("dbg_vprintf: 12  dst_buf total=%zu size=%zu begi=%zu endi=%zu cnt=%zu\n",
        total, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt);
    dump("dbg_vprintf:-    ", dc->dst_buf, dc->dst_buf_size);
  } else {
    printf("dbg_vprintf:+ dst_file\n");
    total = vfprintf(dc->dst_file, format, vlist);
    printf("dbg_vprintf:- dst_file total=%zu\n", total);
  }

  return total;
}


size_t dbg_read(dbg_ctx_t* dc, char* dst, size_t buf_size, size_t size)
{
  //MAYBE_UNUSED(buf_size);
  size_t total;
  char* src;
  char* org_dst = dst;

  pony_assert(buf_size > size);

  // Reduce length by one to leave room for null trailer
  printf("dbg_read:+ rd size=%zu dst_buf size=%zu begi=%zu endi=%zu cnt=%zu\n",
      size, dc->dst_buf_size, dc->dst_buf_begi,
      dc->dst_buf_endi, dc->dst_buf_cnt);
  total = 0;
  if((dst != NULL) && size > 0)
  {
    // total = min(size, dc->dst_buf_cnt)
    total = (size > dc->dst_buf_cnt) ? dc->dst_buf_cnt : size;
    if(total > 0)
    {
      size_t idx = dc->dst_buf_begi;
      printf("dbg_read:  total=%zu idx=%zu\n", total, idx);
      if(idx >= dc->dst_buf_endi)
      {
        // Might do one or two memcpy
        size = dc->dst_buf_size - idx;
        printf("dbg_read:  one or two memcpy size=%zu\n", size);
      } else {
        // One memcpy
        size = dc->dst_buf_endi - idx;
        printf("dbg_read:  one memcpy size=%zu\n", size);
      }
      // Adjust size incase its to large
      if(size > total)
        size = total;
      printf("dbg_read:  total=%zu idx=%zu size=%zu\n", total, idx, size);

      // Do first copy
      size_t cnt = 0;
      src = &dc->dst_buf[idx];
      printf("dbg_read:  1st memcpy cnt=%zu size=%zu total=%zu\n", cnt, size, total);
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
        printf("dbg_read:  2nd memcpy cnt=%zu size=%zu total=%zu\n", cnt, size, total);
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
  printf("dbg_read:- total=%zu dst_buf size=%zu begi=%zu endi=%zu cnt=%zu dst=%s\n",
      total,
      dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi, dc->dst_buf_cnt,
      org_dst);
  return total;
}

#if 0
NOT complete and NOT used
void dbg_linearize(dbg_ctx_t* dc)
{
  char* src;
  char* dst;
  if(dc->dst_buf_endi < dc->dst_buf_begi)
  {
    size_t chunk_size = dc->dst_buf_size - dc->dst_buf_begi;
    if(chunk_size < dc->tmp_buf_size)
    {
      // Fast path as entire beg_chunk fits in tmp_buf

      // Copy entire chunk to tmp_buf
      dst = dc->tmp_buf;
      src = &dc->dst_buf[dc->dst_buf_begi];
      memcpy(dst, src, chunk_size);

      // Move the entire end_chunk which starts at the begining of dst_buf
      // through to endi plus the zero byte using memmove as this is an
      // overlapping move
      dst = &dc->dst_buf[chunk_size];
      src = dc->dst_buf;
      memmove(dst,src, dc->dst_buf_endi + 1);

      // Copy the chunk in tmp_buf back to dst_buf
      dst = dc->dst_buf;
      src = dc->tmp_buf;
      memcpy(dst, src, chunk_size);

    } else {
      // Slow path we have to move things in tmp_buf_size chunks

      // Copy end of the beg_chunk to tmp_buf to make a hole
      dst = dc->tmp_buf;
      src = &dc->dst_buf[dc->dst_buf_size - dc->tmp_buf_size];
      memcpy(dst, src, dc->tmp_buf_size);

      // Loop sliding chunk_size up to the beginning of the buffer
      // through the end chunk
      while(dc->dst_buf_begi > 0)
      {
      }
    }

    // Adjust begi and endi
    dc->dst_buf_begi = 0;
    dc->dst_buf_endi += chunk_size;
  } else {
    // Already linear
  }
}
#endif

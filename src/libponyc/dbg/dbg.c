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
  return dc;
}

void dbg_ctx_destroy(dbg_ctx_t* dc)
{
  free(dc->dst_buf);
  free(dc->bits);
  free(dc);
}

int dbg_printf(dbg_ctx_t* dc, const char* format, ...)
{
  va_list vlist;
  va_start(vlist, format);
  int cnt = dbg_vprintf(dc, format, vlist);
  va_end(vlist);
  return cnt;
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

int dbg_vprintf(dbg_ctx_t* dc, const char* format, va_list vlist)
{
  int cnt;
  size_t size;
  char *restrict dst;
  char *restrict src;
  if(dc->dst_buf != NULL)
  {
    printf("dbg_vprintf:+ dst_buf size=%zu begi=%zu endi=%zu\n",
        dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);

    // Create a copy of the list as we may have to use it twice
    va_list tmp_vlist;
    va_copy(tmp_vlist, vlist);

    dst = &dc->dst_buf[dc->dst_buf_endi];
    if(dc->dst_buf_endi >= dc->dst_buf_begi)
    {
      size = dc->max_size - dc->dst_buf_endi;
      cnt = vsnprintf(dst, size, format, vlist);
      printf("dbg_vprintf: 1 size=%zu cnt=%d\n", size, cnt);
      dump("dbg_vprintf:11 ", dc->dst_buf, dc->dst_buf_size);
      if(cnt >= 0)
      {
        size_t written_excluding_null =
          (size > (size_t)cnt) ? (size_t)cnt : size - 1;
        size_t not_written = (size_t)cnt - written_excluding_null;
        printf("dbg_vprintf: 2 not_written=%zu written_excluding_null=%zu begi=%zu endi=%zu\n",
            not_written, written_excluding_null, dc->dst_buf_begi,
            dc->dst_buf_endi);
        if((written_excluding_null < (dc->max_size - 1))
            && (not_written > 0))
        { // We have not written all we can write AND we have more to write
          printf("dbg_vprintf: 3 not_written=%zu > 0\n", not_written);
          if(written_excluding_null < dc->tmp_buf_size)
          {
            // Write to tmp_buf then move the non-written data to dst_buf
            dst = dc->tmp_buf;
            // min(tmp_buf_size, dst_buf_size)
            size = dc->tmp_buf_size > dc->dst_buf_size ?
              dc->dst_buf_size : dc->tmp_buf_size;
            cnt = vsnprintf(dst, size, format, tmp_vlist);
            printf("dbg_vprintf: 4 tmp_buf=%s size=%zu cnt=%d\n", dst, size, cnt);
            printf("dbg_vprintf:   dst_buf size=%zu begi=%zu endi=%zu\n",
                dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
            dump("dbg_vprintf:41 ", dc->dst_buf, dc->dst_buf_size);
            if(cnt >= 0)
            {
              dst = &dc->dst_buf[dc->dst_buf_endi + written_excluding_null];
              src = &dc->tmp_buf[written_excluding_null];
              size = dc->dst_buf_size - (dc->dst_buf_endi + written_excluding_null);
              printf("dbg_vprintf: 5 before 1 memcpy size=%zu\n", size);
              if(size > not_written)
                size = not_written;
              printf("dbg_vprintf: 6 before 2 memcpy size=%zu\n", size);
              memcpy(dst, src, size);
              dump("dbg_vprintf:61 ", dc->dst_buf, dc->dst_buf_size);
              dc->dst_buf_endi += written_excluding_null + size;
              if(dc->dst_buf_endi >= dc->dst_buf_size)
              {
                printf("dbg_vprintf: 7 dst_buf_endi wrapping\n");
                dc->dst_buf_endi -= dc->dst_buf_size;
                if(dc->dst_buf_endi >= dc->dst_buf_begi)
                {
                  printf("dbg_vprintf: 8 dst_buf_endi wrapped past begi\n");
                  dc->dst_buf_begi = dc->dst_buf_endi + 1;
                  if(dc->dst_buf_begi >= dc->dst_buf_size)
                  {
                    printf("dbg_vprintf:81 dst_buf_endi wrapped past begi\n");
                    dc->dst_buf_begi -= dc->dst_buf_size;
                  }
                }
              }
              dc->dst_buf[dc->dst_buf_endi] = 0;
            } else {
              // some error, just ignore as there isn't anything we can do
              printf("dbg_vprintf: 9 tmp_buf err=%d strerror=%s\n",
                  cnt, strerror(cnt));
            }
          } else {
            printf("dbg_vprintf:10 tmp_buf is too small, ignoring\n");
          }
        } else {
          // We're done on the first write!
          dc->dst_buf_endi += written_excluding_null;
        }
      } else {
        // some error, just ignore as there isn't anything we can do
        printf("dbg_vprintf:11 dst_buf err=%d strerror=%s\n", cnt, strerror(cnt));
      }
    } else {
      printf("dbg_vprintf:12 endi is behind begi\n");
      // Try to write and check for writing past begi
      size = dc->dst_buf_size - dc->dst_buf_endi;
      cnt = vsnprintf(dst, size, format, vlist);
      printf("dbg_vprintf:13 size=%zu cnt=%d\n", size, cnt);
      if(cnt >= 0)
      {
        size_t written_excluding_null =
          (size > (size_t)cnt) ? (size_t)cnt : size - 1;
        size_t not_written = (size_t)cnt - written_excluding_null;
        printf("dbg_vprintf:14 not_written=%zu written_excluding_null=%zu begi=%zu endi=%zu\n",
            not_written, written_excluding_null, dc->dst_buf_begi,
            dc->dst_buf_endi);
        if(not_written > 0)
        {
          printf("dbg_vprintf:15 not_written=%zu > 0\n", not_written);
          if(written_excluding_null < dc->tmp_buf_size)
          {
            // Write to tmp_buf then move the non-written data to dst_buf
            dst = dc->tmp_buf;
            size = dc->tmp_buf_size;
            cnt = vsnprintf(dst, size, format, tmp_vlist);
            printf("dbg_vprintf:16 tmp_buf=%s size=%zu cnt=%d\n", dst, size, cnt);
            printf("dbg_vprintf:   dst_bufi size=%zu begi=%zu endi=%zu\n",
                dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
            if(cnt >= 0)
            {
              dst = &dc->dst_buf[0];
              src = &dc->tmp_buf[written_excluding_null];
              size = dc->dst_buf_endi - 1; // ?
              printf("dbg_vprintf:17 before 1 memcpy size=%zu\n", size);
              if(size > not_written)
                size = not_written;
              printf("dbg_vprintf:18 before 2 memcpy size=%zu\n", size);
              memcpy(dst, src, size);
              dc->dst_buf_endi += size;
              dc->dst_buf[dc->dst_buf_endi] = 0;
            } else {
              // some error, just ignore as there isn't anything we can do
              printf("dbg_vprintf:19 tmp_buf err=%d strerror=%s\n",
                  cnt, strerror(cnt));
            }
          } else {
            printf("dbg_vprintf:20 tmp_buf is too small, ignoring\n");
          }
        } else {
          dc->dst_buf_endi = written_excluding_null;
          printf("dbg_vprintf:21 everything was written check endi past begi\n");
          if(dc->dst_buf_endi >= dc->dst_buf_begi)
          {
            printf("dbg_vprintf:22 dst_buf_endi wrapped past begi\n");
            dc->dst_buf_begi = dc->dst_buf_endi + 1;
            if(dc->dst_buf_begi >= dc->dst_buf_size)
            {
              printf("dbg_vprintf:23 dst_buf_endi wrapped past begi\n");
              dc->dst_buf_begi -= dc->dst_buf_size;
            }
          }
        }
      } else {
        // some error, just ignore as there isn't anything we can do
        printf("dbg_vprintf:11 dst_buf err=%d strerror=%s\n", cnt, strerror(cnt));
      }
    }
    // Release the copy
    va_end(tmp_vlist);
    printf("dbg_vprintf:  dst_buf cnt=%d size=%zu begi=%zu endi=%zu\n",
        cnt, dc->dst_buf_size, dc->dst_buf_begi, dc->dst_buf_endi);
    dump("dbg_vprintf:- ", dc->dst_buf, dc->dst_buf_size);
  } else {
    printf("dbg_vprintf:+ dst_file\n");
    cnt = vfprintf(dc->dst_file, format, vlist);
    printf("dbg_vprintf:- dst_file cnt=%d\n", cnt);
  }
  return cnt;
}

size_t dbg_read(dbg_ctx_t* dc, char* dst, size_t size)
{
  size_t cnt;
  char* src;

  // Reduce length by one to leave room for null trailer
  size -= 1;
  printf("dbg_read:+ rd_size=%zu size=%zu begi=%zu endi=%zu\n",
      size, dc->dst_buf_size, dc->dst_buf_begi,
      dc->dst_buf_endi);
  if (dc->dst_buf_endi >= dc->dst_buf_begi)
  {
    cnt = dc->dst_buf_endi - dc->dst_buf_begi;
  } else {
    cnt = dc->dst_buf_size - (dc->dst_buf_begi - dc->dst_buf_endi);
  }
  if((dst != NULL) && size > 0) {
    if(dc->dst_buf_endi >= dc->dst_buf_begi)
    {
      cnt = dc->dst_buf_endi - dc->dst_buf_begi;
      if(cnt > size)
        cnt = size;
      if(cnt > 0)
      {
        printf("dbg_read:  memcpy single chunk cnt=%zu size=%zu begi=%zu endi=%zu\n",
            cnt, dc->dst_buf_size, dc->dst_buf_begi,
            dc->dst_buf_endi);
        src = &dc->dst_buf[dc->dst_buf_begi];
        memcpy(dst, src, cnt);
      } else {
        printf("dbg_read:  nothing to copy single chunk cnt=%zu\n", cnt);
      }
      dst += cnt;
    } else {
      cnt = dc->dst_buf_size - (dc->dst_buf_begi - dc->dst_buf_endi);
      src = &dc->dst_buf[dc->dst_buf_begi];
      if(cnt > size)
        cnt = size;
      if(cnt > 0)
      {
        size_t chunk_size = dc->dst_buf_size - dc->dst_buf_begi;
        if(chunk_size > cnt)
        {
          chunk_size = cnt;
        }
        printf("dbg_read:  memcpy first chunk cnt=%zu chunk_size=%zu size=%zu begi=%zu endi=%zu\n",
            cnt, chunk_size, dc->dst_buf_size, dc->dst_buf_begi,
            dc->dst_buf_endi);
        memcpy(dst, src, chunk_size);
        dst += chunk_size;
        src = &dc->dst_buf[0];
        printf("dbg_read:  memcpy second chunk cnt=%zu chunk_size=%zu size=%zu begi=%zu endi=%zu\n",
            cnt, cnt - chunk_size, dc->dst_buf_size, dc->dst_buf_begi,
            dc->dst_buf_endi);
        memcpy(dst, src, cnt - chunk_size);
        dst += cnt - chunk_size;
      }
    }
    *dst = 0;
    dc->dst_buf_begi += cnt;
    if(dc->dst_buf_begi >= dc->dst_buf_size)
      dc->dst_buf_begi -= dc->dst_buf_size;
  }
  printf("dbg_read:- cnt=%zu size=%zu begi=%zu endi=%zu\n",
      cnt, dc->dst_buf_size, dc->dst_buf_begi,
      dc->dst_buf_endi);
  return cnt;
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

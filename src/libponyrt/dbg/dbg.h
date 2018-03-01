/**
 * Allow fine grained debugging.
 *
 * The basic concept is there is an opaque debug context which allows
 * multiple implementations and a simple primary interface:
 *
 *    DBG(dc, bit_idx)
 *    DBG_LOC(dc)
 *    dbg_printf(dc, format, ...)
 *
 * DBG is a macro which will return true if the bit_idx is set and if true
 * also invokes the DBG_LOC macro to set location information.
 *
 * DBG_LOC sets the location information, function name and line number.
 *
 * dbg_printf is a variadic functions like fprintf but instead of a FILE*
 * for the destination it has a dbg_ctx_t*. If there is location information
 * in the dbg_ctx_t it will preceed the other data. The location information
 * will be erased after printing so it only prints once.
 *
 * You also need to create/destroy the dbg_ctx_t. Destroying of all contexts
 * is done using dbg_destroy(dc). To create a context each implementation
 * provides one or more create methods. Currently there are three
 * implementations:
 *
 *    dbg_ctx_create(number_of_bits)
 *    dbg_ctx_impl_file_create(number_of_bits, file)
 *    dbg_ctx_impl_mem_create(number_of_bits, dst_size, tmp_buf_size)
 *
 * dbg_ctx_create creates an array bits without a destination and is
 * useful if you never need dbg_printf and just want to control some
 * other statements.
 *
 * dbg_ctx_impl_file_create creates the array of bits and prints to the
 * file using vfprintf.
 *
 * dbg_ctx_impl_mem_create creates the array of bits and prints to a memory
 * buffer using vsnprintf.
 *
 * Example:
 *   #define DBG_ENABLED 1
 *   #include "dbg.h"
 *   #include "dbg_ctx_impl_file.h"
 *
 *   void example()
 *   {
 *     dbg_ctx_t* dc = dbg_ctx_impl_file_create(2, stdout);
 *     dbg_sb(dc, 0, 1);
 *     if(DBG(dc, 0)) dbg_printf(dc, "this is printed with location info\n");
 *     if(DBG(dc, 1)) dbg_printf(dc, "this is not printed\n");
 *     dbg_printf(dc, "printed without location %s\n", "info");
 *     dbg_printf(DBG_LOC(dc), "location %s", "info"); dbg_printf(dc, ", then none!\n");
 *     dbg_destroy(dc);
 *   }
 *
 * The output would be:
 *   example:9    this is printed with location info
 *   printed without location info
 *   example:12   location info, then none!
 */
#ifndef DBG_H
#define DBG_H

#if !defined(DBG_ENABLED)
#define DBG_ENABLED false
#endif

#include "dbg_common.h"
#include <platform.h>
#include <stdio.h>
#include <stdint.h>

PONY_EXTERN_C_BEGIN

/**
 * @return true if DBG_ENABLED and bit is set, also sets DBG_LOC.
 */
#define DBG(dc, bit_idx) dbg(DBG_LOC(dc), bit_idx)

/**
 * Set the dc location information
 */
#define DBG_LOC(dc) dbg_loc(dc, (char*)__FUNCTION__, __LINE__)

/**
 * Print a formated string preceeded optionally by location information
 * if dc.fun_name is not NULL to the current dbg_ctx destination.
 */
size_t dbg_printf(dbg_ctx_t* dc, const char* format, ...);

/**
 * Print a formated string preceeded optionally by location information
 * if dc.fun_name is not NULL to the current dbg_ctx destination with
 * the args being a va_list.
 */
size_t dbg_vprintf(dbg_ctx_t* dc, const char* format, va_list args);

/**
 * Create a dbg_ctx with just bits
 */
dbg_ctx_t* dbg_ctx_create(size_t number_of_bits);

/**
 * Destroy a previously created dbg_ctx
 */
void dbg_ctx_destroy(dbg_ctx_t* dc);

/**
 * Set bit
 */
void dbg_set_bit(dbg_ctx_t* dc, size_t bit_idx, bool bit_value);

/**
 * Get bit
 */
bool dbg_get_bit(dbg_ctx_t* dc, size_t bit_idx);

/**
 * Calculate index into dbg_ctx_t.bits_array
 */
static inline size_t dbg_bits_array_idx(size_t bit_idx)
{
  return bit_idx / (sizeof(size_t) * 8);
}

/**
 * Calculate bit mask for dbg_ctx_t.bits_array
 */
static inline size_t dbg_bit_mask(size_t bit_idx)
{
  return (size_t)1 << (size_t)((bit_idx) & ((sizeof(size_t) * 8) - 1));
}

/**
 * Set bit at bit_idx to bit_value
 */
static inline void dbg_sb(dbg_ctx_t* dc, size_t bit_idx, bool bit_value)
{
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;
  if((dcc != NULL) && ((dcc->id & 0xFFFF) == (DBG_CTX_ID_COMMON & 0xFFFF))
      && (bit_idx < dcc->number_of_bits))
  {
    size_t bits_array_idx = dbg_bits_array_idx(bit_idx);
    size_t bit_mask = dbg_bit_mask(bit_idx);
    if(bit_value)
    {
      dcc->bits[bits_array_idx] |= bit_mask;
    } else {
      dcc->bits[bits_array_idx] &= ~bit_mask;
    }
  }
}

/**
 * Get bit at bit_idx
 */
static inline bool dbg_gb(dbg_ctx_t* dc, size_t bit_idx)
{
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;
  if((dcc != NULL) && ((dcc->id & 0xFFFF) == (DBG_CTX_ID_COMMON & 0xFFFF))
      && (bit_idx < dcc->number_of_bits))
  {
    size_t bits_array_idx = dbg_bits_array_idx(bit_idx);
    return (dcc->bits[bits_array_idx] & dbg_bit_mask(bit_idx)) != 0;
  } else {
    return false;
  }
  return false;
}

/**
 * Set the dc location information
 */
static inline dbg_ctx_t* dbg_loc(dbg_ctx_t* dc, char* fun_name, int line_num)
{
  dbg_ctx_common_t* dcc = (dbg_ctx_common_t*)dc;
  if((dcc != NULL) && ((dcc->id & 0xFFFF) == (DBG_CTX_ID_COMMON & 0xFFFF)))
  {
    dcc->fun_name = fun_name;
    dcc->line_num = line_num;
  }
  return dc;
}

/**
 * @return true if DBG_ENABLED and bit is set
 */
static inline bool dbg(dbg_ctx_t* dc, size_t bit_idx)
{
  return DBG_ENABLED && dbg_gb(dc, bit_idx);
}

PONY_EXTERN_C_END

#endif

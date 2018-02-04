#ifndef DBG_UTIL_H
#define DBG_UTIL_H

// Assume #define z 123
// Assume #define FOO z

// xstr(FOO) -> "123"
#define dbg_xstr(z) dbg_str(z)

// str(FOO)  -> "FOO"
#define dbg_str(s) #s

// Useful to mark unused parameters to routines
#if !defined(MAYBE_UNUSED)
#define MAYBE_UNUSED(x) (void)x
#endif

#if !defined(UNUSED)
#define UNUSED(x) (void)x
#endif

/**
 * Define INITIALIZER and FINALIZER macros
 * Example:
 *
 * // Initialize debug context
 * static dbg_ctx_t* dc = NULL;
 * INITIALIZER(initialize)
 * {
 *   dc = dc_create(stderr, 32);
 *   fprintf(stderr, "initialize:# %s\n", __FILE__);
 * }
 *
 * //  Finalize debug context
 * FINALIZER(finalize)
 * {
 *   dc_destroy(dc);
 *   dc = NULL;
 *   fprintf(stderr, "finalize:# %s\n", __FILE__);
 * }
 */
#ifdef _MSC_VER

#define INITIALIZER(f) \
  static void f(); \
  static int __f1(){f();return 0;} \
  __pragma(data_seg(".CRT$XIU")) \
  static int(*__f2) () = __f1; \
  __pragma(data_seg()) \
  static void f()

#define FINALIZER(f) \
  static void f(); \
  static int __f1(){f();return 0;} \
  __pragma(data_seg(".CRT$XPU")) \
  static int(*__f2) () = __f1; \
  __pragma(data_seg()) \
  static void f()

#else

#define INITIALIZER(f) \
  static void __attribute__ ((constructor)) f()

#define FINALIZER(f) \
  static void __attribute__ ((destructor)) f()

#endif

#define _DBG_BIT_IDX_NAME(base_name) (base_name ## _bit_idx)
#define _DBG_BIT_CNT_NAME(base_name) (base_name ## _bit_cnt)

#define _DBG_NEXT_IDX(base_name) (_DBG_BIT_IDX_NAME(base_name) + \
                                    _DBG_BIT_CNT_NAME(base_name))

#define DBG_BITS_FIRST(name, cnt) \
  name ## _bit_idx = 0, name ## _bit_cnt = cnt

#define DBG_BITS_NEXT(name, cnt, previous) \
  name ## _bit_idx = _DBG_NEXT_IDX(previous), name ## _bit_cnt = cnt

#define DBG_BITS_SIZE(name, previous) \
  name = _DBG_NEXT_IDX(previous)

/**
 * Convert a base_name and offset to an index into
 * the bits array.
 *
 * Example: bits can be defined by a base_name and offset and
 * placed in an enum. And the the dbg_bnoi macro can be used
 * to convert the base_name to a bits array index.
 *
 * enum {
 *   DBG_BITS_FIRST(first, 30),
 *   DBG_BITS_NEXT(dummy, 3, first),
 *   DBG_BITS_NEXT(another, 2, dummy),
 *   DBG_BITS_SIZE(bits_size, another)
 * };
 *
 *
 * The macro dbg_bnoi can now be used to convert
 * the base_name and an offset to a bit_idx. So
 * below we create debug context, dc, with enough
 * bits to over first, dummy, another. Next we
 * use dbg_bnoi to get the last bit of dummy and
 * print "Hello, World" because we've set the bit.
 *
 * dbg_ctx_t* dc = dbg_ctx_create(stderr, bits_size);
 * dbg_sb(dc, dbg_bnoi(dummy, 2), 1);
 * dbg_pf(dc, dbg_bnoi(dummy, 2), "Hello, %s\n", "World");
 * dbg_ctx_destroy(dc);
 */
#define dbg_bnoi(base_name, offset) \
  (_DBG_BIT_IDX_NAME(base_name) + offset)

#endif

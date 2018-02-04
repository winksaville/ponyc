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

#endif

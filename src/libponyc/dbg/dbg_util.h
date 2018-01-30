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

#endif

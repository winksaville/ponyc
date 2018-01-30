#ifndef DBG_H
#define DBG_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

#define xstr(x) str(s)
#define str(s) #s

#if !defined(UNUSED)
#define UNUSED(x) (void)x
#endif

#if !defined(DBG_FILE)
#define DBG_FILE stdout
#endif

#if !defined(DBG_ENABLED)
#define DBG_ENABLED false
#endif

/**
 * Debug print to the file without trailing new line
 */
#define DFP(file, format, ...) \
  do { if(DBG_ENABLED) fprintf(file, format, ## __VA_ARGS__); } while(0)

/**
 * Debug print to the file with trailing new line
 */
#define DFPL(file, format, ...) \
  do { if(DBG_ENABLED) fprintf(file, format "\n", ## __VA_ARGS__); } while(0)

/**
 * Debug print, without trailing new line
 */
#define DP(format, ...) \
  DFP(DBG_FILE, format, ## __VA_ARGS__)

/**
 * Debug print with trailing new line
 */
#define DPL(format, ...) \
  DFPL(DBG_FILE, format, ## __VA_ARGS__)

/**
 * Debug print leading functionName: without trailing new line
 */
#define DPN(format, ...) \
  DFP(DBG_FILE, "%s:  " format, __FUNCTION__, ## __VA_ARGS__)

/**
 * Debug print leading functionName: with trailing new line
 */
#define DPLN(format, ...) \
  DFPL(DBG_FILE, "%s:  " format, __FUNCTION__, ## __VA_ARGS__)

/**
 * Enter routine print leading functionName:+ without trailing new line
 */
#define DPE(format, ...) \
  DFP(DBG_FILE, "%s:+ " format, __FUNCTION__, ## __VA_ARGS__)

/**
 * eXit routine print leading functionName:+ without trailing new line
 */
#define DPX(format, ...) \
  DFP(DBG_FILE, "%s:- " format, __FUNCTION__, ## __VA_ARGS__)

/**
 * eXit routine print leading functionName:+ r=result without trailing new line
 */
#define DPXR(r, format, ...) \
  DFP(DBG_FILE, "%s:- r=%d " format, __FUNCTION__, r, ## __VA_ARGS__)

/**
 * Enter routine print leading functionName:+ with trailing new line
 */
#define DPLE(format, ...) \
  DFPL(DBG_FILE, "%s:+ " format "\n", __FUNCTION__, ## __VA_ARGS__)

/**
 * eXit routine Debug print leading functionName:+ name with trailing new line
 */
#define DPLX(format, ...) \
  DFPL(DBG_FILE, "%s:- " format "\n", __FUNCTION__, ## __VA_ARGS__)

/**
 * eXit routine print leading functionName:+ r=result without trailing new line
 */
#define DPLXR(r, format, ...) \
  DFP(DBG_FILE, "%s:- r=%d " format "\n", __FUNCTION__, r, ## __VA_ARGS__)

#endif

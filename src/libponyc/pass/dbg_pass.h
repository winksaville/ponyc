#ifndef DBG_H
#define DBG_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

PONY_EXTERN_C_BEGIN

#if !defined(UNUSED)
#define UNUSED(x) (void)x
#endif

#if !defined(AST_PRINT_WIDTH)
#define AST_PRINT_WIDTH 200
#endif

#define RV_IGNORE -99999

#define xstr(x) str(s)
#define str(s) #s

#if !defined(LOCAL_DBG_PASS)
#define LOCAL_DBG_PASS false
#endif

#if LOCAL_DBG_PASS == true

  #define DB(format, ...)  fprintf(stdout, "" format, ## __VA_ARGS__)
  #define DBG(format, ...)  fprintf(stdout, "%s:  " format, __FUNCTION__, ## __VA_ARGS__)

  #define DBGE() dbg_pass_fprint(stdout, __FUNCTION__, "+", RV_IGNORE, NULL)
  #define DBGX() dbg_pass_fprint(stdout, __FUNCTION__, "-", RV_IGNORE, NULL)
  #define DBGXR(rv) dbg_pass_fprint(stdout, __FUNCTION__, "-", rv, NULL)

  #define DBGES(s) dbg_pass_fprint(stdout, __FUNCTION__, "+", RV_IGNORE, s)
  #define DBGXS(s) dbg_pass_fprint(stdout, __FUNCTION__, "-", RV_IGNORE, s)
  #define DBGXRS(rv, s) dbg_pass_fprint(stdout, __FUNCTION__, "-", rv, s)

  #define DAPE(ast) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, "+", RV_IGNORE, str(ast), ast, NULL, 1, \
      AST_PRINT_WIDTH)
  #define DAPES(ast, s) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, "+", RV_IGNORE, str(ast), ast, s, 1, \
      AST_PRINT_WIDTH)

  #define DAPX(ast) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, "-", RV_IGNORE, str(ast), ast, NULL, 1, \
      AST_PRINT_WIDTH)
  #define DAPXS(ast, s) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, "-", RV_IGNORE, str(ast), ast, s, 1, \
      AST_PRINT_WIDTH)
  #define DAPXR(ast, rv) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, "-", rv, str(ast), ast, NULL, 1, \
      AST_PRINT_WIDTH)
  #define DAPXRS(ast, rv, s) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, "-", rv, str(ast), ast, s, 1, \
      AST_PRINT_WIDTH)
  #define DAPXVS(ast, v, s) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, " ", v, str(ast), ast, s, 1, \
      AST_PRINT_WIDTH)


  // Debug ast_print
  #define DAP(ast) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, " ", RV_IGNORE, str(ast), ast, NULL, 1, \
      AST_PRINT_WIDTH)
  #define DAPS(ast, s) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, " ", RV_IGNORE, str(ast), ast, s, 1, \
      AST_PRINT_WIDTH)

  // Debug ast_print_and_parents
  #define DAPP(ast, number) dbg_pass_ast_fprint_and_parents( \
      stdout, __FUNCTION__, " ", RV_IGNORE, str(ast), ast, NULL, number, \
      AST_PRINT_WIDTH)

  // Debug ast_siblings
  #define DAS(ast) dbg_pass_ast_fprint_siblings( \
      stdout, __FUNCTION__, str(ast), ast, AST_PRINT_WIDTH)

#else
  #define DB(format, ...) do {UNUSED(format);} while(0)
  #define DBG(format, ...) do {UNUSED(format);} while(0)

  #define DBGE() do {} while(0)
  #define DBGX() do {} while(0)
  #define DBGXR(rv) do {UNUSED(rv);} while(0)

  #define DBGES(s) do {UNUSED(s);} while(0)
  #define DBGXS(s) do {UNUSED(s);} while(0)
  #define DBGXRS(rv,s) do {UNUSED(rv); UNUSED(s);} while(0)

  #define DAPE(ast) do {UNUSED(ast);} while(0)
  #define DAPES(ast,s) do {UNUSED(ast); UNUSED(s);} while(0)
  #define DAPX(ast) do {UNUSED(ast);} while(0)
  #define DAPXS(ast,s) do {UNUSED(ast); UNUSED(s);} while(0)
  #define DAPXR(ast,rv) do {UNUSED(ast); UNUSED(rv);} while(0)
  #define DAPXRS(ast,rv,s) do {UNUSED(ast); UNUSED(rv); UNUSED(s);} while(0)
  #define DAPXVS(ast,v,s) do {UNUSED(ast); UNUSED(v); UNUSED(s);} while(0)

  #define DAP(ast) do {UNUSED(ast);} while(0)
  #define DAPS(ast,s) do {UNUSED(ast); UNUSED(s);} while(0)

  #define DAPP(ast,number) do {UNUSED(ast); UNUSED(number);} while(0)
  #define DAS(ast) do {UNUSED(ast);} while(0)
#endif

void dbg_pass_fprint(FILE* fp, const char* fun_name, const char* ex, int rv,
    const char* s);

void dbg_pass_ast_fprint_and_parents(FILE* fp, const char* fun_name,
    const char* ex, int rv, const char* ast_name, ast_t* ast, const char* s,
    int number, int width);

void dbg_pass_ast_fprint_siblings(FILE* fp, const char* fun_name,
    const char* ast_name, ast_t* ast, int width);

#endif

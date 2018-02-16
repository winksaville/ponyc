#ifndef DBG_AST_H
#define DBG_AST_H

#include "../../libponyrt/dbg/dbg.h"
#include "../../libponyrt/dbg/dbg_util.h"
#include "../../libponyrt/dbg/dbg_ctx_impl_file.h"
#include "../../libponyrt/dbg/dbg_ctx_impl_file_internal.h"
#include "../ast/ast.h"
#include "../pass/pass.h"

#include <platform.h>

#if !defined(DBG_AST_ENABLED)
#define DBG_AST_ENABLED false
#endif

#if !defined(DBG_AST_PRINT_WIDTH)
#define DBG_AST_PRINT_WIDTH 200
#endif

#define DBG_AST(ctx, ast) \
  _DBG_DO( \
    if(DBG_AST_ENABLED) \
    { \
      dbg_printf(dc, "%s ", dbg_str(ast)); \
      dbg_ctx_impl_file_t* dcf = (dbg_ctx_impl_file_t*)dc; \
      ast_fprint(dcf->dst_file, ast, DBG_AST_PRINT_WIDTH); \
    })

#define DBG_ASTF(ctx, ast, format, ...) \
  _DBG_DO( \
    if(DBG_AST_ENABLED) \
    { \
      dbg_printf(dc, "%s " format, dbg_str(ast), ## __VA_ARGS__); \
      dbg_ctx_impl_file_t* dcf = (dbg_ctx_impl_file_t*)dc; \
      ast_fprint(dcf->dst_file, ast, DBG_AST_PRINT_WIDTH); \
    })

// Debug ast_print_and_parents
#define DBG_ASTP(ctx, ast, number) \
  _DBG_DO( \
    if(DBG_AST_ENABLED) \
    { \
      dbg_ctx_impl_file_t* dcf = (dbg_ctx_impl_file_t*)dc; \
      for(int i=0; (ast != NULL) && (i < number); i++) \
      { \
        dbg_printf(DBG_LOC(dc), "%s[%d]: ", dbg_str(ast), -i); \
        ast_fprint(dcf->dst_file, ast, DBG_AST_PRINT_WIDTH); \
        ast = ast_parent(ast); \
      } \
    })

// Debug ast_siblings
#define DBG_ASTS(ctx, ast) \
  _DBG_DO( \
    if(DBG_AST_ENABLED) \
    { \
      ast_t* parent = ast_parent(ast); \
      ast_t* child = ast_child(parent); \
      dbg_ctx_impl_file_t* dcf = (dbg_ctx_impl_file_t*)dc; \
      for(int i=0; (child != NULL); i++) \
      { \
        dbg_printf(DBG_LOC(dc), "%s[%d]: ", dbg_str(ast), i); \
        ast_fprint(dcf->dst_file, child, DBG_AST_PRINT_WIDTH); \
        child = ast_sibling(child); \
      } \
    })

#endif

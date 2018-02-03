#ifndef DBG_AST_H
#define DBG_AST_H

#include "../dbg/dbg.h"
#include "../dbg/dbg_util.h"
#include "../ast/ast.h"
#include "../pass/pass.h"

#include <platform.h>

#if !defined(DBG_AST_ENABLED)
#define DBG_AST_ENABLED false
#endif

#if !defined(AST_PRINT_WIDTH)
#define AST_PRINT_WIDTH 200
#endif

#define DAST(ctx, bit_idx, ast) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      if(dc_gb(ctx, bit_idx)) \
      { \
        fprintf(ctx->file, "%s:  %s ", __FUNCTION__, dbg_str(ast)); \
        ast_fprint(ctx->file, ast, AST_PRINT_WIDTH); \
      } \
    } \
  } \
  while(0)

#define DASTF(ctx, bit_idx, ast, format, ...) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      if(dc_gb(ctx, bit_idx)) \
      { \
        fprintf(ctx->file, "%s:  " format, __FUNCTION__, ## __VA_ARGS__); \
        ast_fprint(ctx->file, ast, AST_PRINT_WIDTH); \
      } \
    } \
  } \
  while(0)

// Debug ast_print_and_parents
#define DASTP(ctx, bit_idx, ast, number) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      if(dc_gb(ctx, bit_idx)) \
      { \
        for(int i=0; (ast != NULL) && (i < number); i++) \
        { \
          fprintf(ctx->file, "%s: %s[%d]: ", __FUNCTION__, dbg_str(ast), -i); \
          ast_fprint(ctx->file, ast, AST_PRINT_WIDTH); \
          ast = ast_parent(ast); \
        } \
      } \
    } \
  } \
  while(0)

// Debug ast_siblings
#define DASTS(ctx, bit_idx, ast) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      if(dc_gb(ctx, bit_idx)) \
      { \
        ast_t* parent = ast_parent(ast); \
        ast_t* child = ast_child(parent); \
        for(int i=0; (child != NULL); i++) \
        { \
          fprintf(ctx->file, "%s %s[%d]: ", __FUNCTION__, dbg_str(ast), i); \
          ast_fprint(ctx->file, child, AST_PRINT_WIDTH); \
          child = ast_sibling(child); \
        } \
      } \
    } \
  } \
  while(0)

#endif

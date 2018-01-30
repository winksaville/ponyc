#ifndef DBG_AST_H
#define DBG_AST_H

#include <platform.h>
#include "../ast/ast.h"
#include "../pass/pass.h"

#if !defined(DBG_AST_ENABLED)
#define DBG_AST_ENABLED false
#endif

#if !defined(DBG_ENABLED)
#define DBG_ENABLED DBG_AST_ENABLED
#endif
#include "../dbg/dbg.h"

PONY_EXTERN_C_BEGIN

#if !defined(AST_PRINT_WIDTH)
#define AST_PRINT_WIDTH 200
#endif

#define DAST(ast) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      DPN("%s ", str(ast)); \
      ast_print(ast, AST_PRINT_WIDTH); \
    } \
  } \
  while(0)

#define DASTF(ast, format, ...) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      DPN("%s " format, str(ast), ## __VA_ARGS__); \
      ast_print(ast, AST_PRINT_WIDTH); \
    } \
  } \
  while(0)

#define DASTE(ast) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      DPE("%s ", str(ast)); \
      ast_print(ast, AST_PRINT_WIDTH); \
    } \
  } \
  while(0)

#define DASTX(ast) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      DPX("%s ", str(ast)); \
      ast_print(ast, AST_PRINT_WIDTH); \
    } \
  } \
  while(0)

#define DASTXR(r, ast) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      DPX("r=%d %s ", r, str(ast)); \
      ast_print(ast, AST_PRINT_WIDTH); \
    } \
  } \
  while(0)

// Debug ast_print_and_parents
#define DASTP(ast, number) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      for(int i=0; (ast != NULL) && (i < number); i++) \
      { \
        fprintf(DBG_FILE, "%s: %s[%d]: ", __FUNCTION__, str(ast), -i); \
        ast_fprint(DBG_FILE, ast, AST_PRINT_WIDTH); \
        ast = ast_parent(ast); \
      } \
    } \
  } \
  while(0)

// Debug ast_siblings
#define DASTS(ast) \
  do \
  { \
    if(DBG_AST_ENABLED) \
    { \
      ast_t* parent = ast_parent(ast); \
      ast_t* child = ast_child(parent); \
      for(int i=0; (child != NULL); i++) \
      { \
        fprintf(DBG_FILE, "%s %s[%d]: ", __FUNCTION__, str(ast), i); \
        ast_fprint(DBG_FILE, child, AST_PRINT_WIDTH); \
        child = ast_sibling(child); \
      } \
    } \
  } \
  while(0)

#endif
